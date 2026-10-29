// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterCharacter.h"
#include "../AI/ElmAIController.h"
#include "../AbilitySystem/Attributes/BaseAttributeSet.h"
#include "../AbilitySystem/Attributes/PlayerAttributeSet.h"
#include "../AbilitySystem/Attributes/MonsterAttributeSet.h"
#include "../AbilitySystem/Components/AIAbilitySystemComponent.h"
#include "../AbilitySystem/Effects/GameplayEffect_Reward.h"
#include "../AbilitySystem/ElmGameplayTags.h"
#include "AbilitySystemGlobals.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

// 부모 생성자가 "AttributeSet"을 만들기 전에 그 클래스를 UMonsterAttributeSet으로 바꿔 둔다.
// 이름은 AAICharacter 생성자의 CreateDefaultSubobject와 정확히 같아야 한다.
AMonsterCharacter::AMonsterCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMonsterAttributeSet>(TEXT("AttributeSet")))
{
	mTeam = ETeamType::Monster;
}

void AMonsterCharacter::InitAttributes()
{
	// 외형 / 능력치 / 공격 / AI 설정은 부모가 테이블 값 그대로(1배) 한다.
	Super::InitAttributes();

	const FMonsterCharacterTableRow* Info = FindTableRow();

	if (!Info)
		return;

	// 등급은 UI(보스 HP바 등)에도 쓰이므로 서버/클라 모두 설정한다.
	mMonsterType = Info->Type;

	if (mMonsterType == EMonsterType::Boss)
		mASC->AddLooseGameplayTag(ElmTags::Monster_Type_Boss);

	// 부모가 넣은 값에 등급 배율을 곱한다. (Normal은 1배라서 바뀌는 것이 없다)
	ApplyTypeScale(*Info);

	// 보상은 서버에서만 설정한다.
	if (!HasAuthority())
		return;

	mASC->SetNumericAttributeBase(UMonsterAttributeSet::GetDropExpAttribute(), static_cast<float>(Info->DropExp));
	mASC->SetNumericAttributeBase(UMonsterAttributeSet::GetDropGoldAttribute(), static_cast<float>(Info->DropGold));

	// 수치가 아닌 데이터는 멤버로 보관한다.
	mDropItems = Info->DropItems;
}

void AMonsterCharacter::ApplyTypeScale(const FMonsterCharacterTableRow& Info)
{
	const float SizeScale = GetMonsterTypeSizeScale(mMonsterType);
	const float HPScale = GetMonsterTypeHPScale(mMonsterType);

	// 크기. 캡슐과 메시를 같이 키우고, 발이 캡슐 바닥에 오도록 메시를 HalfHeight만큼 내린다.
	const float ScaledHalfHeight = Info.CapsuleHalfHeight * SizeScale;

	GetCapsuleComponent()->SetCapsuleSize(Info.CapsuleRadius * SizeScale, ScaledHalfHeight);
	GetMesh()->SetRelativeLocation(FVector(0.0, 0.0, -ScaledHalfHeight));
	GetMesh()->SetRelativeScale3D(FVector(SizeScale));

	// 능력치는 서버에서만 설정한다.
	if (!HasAuthority())
		return;

	// 체력. 부모가 레벨 스탯으로 넣은 MaxHP에 배율을 곱하고 HP를 가득 채운다.
	// (HP는 MaxHP를 넘지 않게 Clamp되므로 MaxHP를 먼저 넣는다)
	const float MaxHP = mASC->GetNumericAttribute(UBaseAttributeSet::GetMaxHPAttribute()) * HPScale;

	mASC->SetNumericAttributeBase(UBaseAttributeSet::GetMaxHPAttribute(), MaxHP);
	mASC->SetNumericAttributeBase(UBaseAttributeSet::GetHPAttribute(), MaxHP);

	// 몸이 커진 만큼 공격이 닿는 거리와 범위도 늘린다. (보이는 것과 판정을 맞춘다)
	const float ScaledRange = Info.AttackRange * SizeScale;

	mASC->SetNumericAttributeBase(UBaseAttributeSet::GetAttackRangeAttribute(), ScaledRange);
	mASC->SetNumericAttributeBase(UBaseAttributeSet::GetAttackRadiusAttribute(), Info.AttackRadius * SizeScale);

	if (AElmAIController* AIController = GetController<AElmAIController>())
		AIController->SetAttackRange(ScaledRange);
}

const FMonsterCharacterTableRow* AMonsterCharacter::FindTableRow() const
{
	return FindDataRow<FMonsterCharacterTableRow>();
}

void AMonsterCharacter::OnKilledBy(AActor* Killer)
{
	// AI 정지와 충돌 끄기는 부모가 한다.
	Super::OnKilledBy(Killer);

	GiveReward(Killer);
}

void AMonsterCharacter::GiveReward(AActor* Killer)
{
	// Killer가 플레이어 캐릭터면 PlayerState의 ASC가 나온다.
	UAbilitySystemComponent* KillerASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Killer);

	// 경험치/골드 Attribute를 가진 대상(플레이어)에게만 준다. (NPC가 잡으면 보상 없음)
	if (!KillerASC || !KillerASC->HasAttributeSetForAttribute(UPlayerAttributeSet::GetExpAttribute()))
		return;

	// 보상을 "주는 쪽"은 몬스터이므로 몬스터 ASC로 스펙을 만든다.
	FGameplayEffectSpecHandle RewardSpec = mASC->MakeOutgoingSpec(
		UGameplayEffect_Reward::StaticClass(), 1.f, mASC->MakeEffectContext());

	if (!RewardSpec.IsValid())
		return;

	// GE_Reward의 Modifier 두 개가 모두 SetByCaller이므로 둘 다 항상 채워야 한다.
	RewardSpec.Data->SetSetByCallerMagnitude(ElmTags::SetByCaller_Reward_Exp,
		mASC->GetNumericAttribute(UMonsterAttributeSet::GetDropExpAttribute()));
	RewardSpec.Data->SetSetByCallerMagnitude(ElmTags::SetByCaller_Reward_Gold,
		mASC->GetNumericAttribute(UMonsterAttributeSet::GetDropGoldAttribute()));

	mASC->ApplyGameplayEffectSpecToTarget(*RewardSpec.Data.Get(), KillerASC);
}
