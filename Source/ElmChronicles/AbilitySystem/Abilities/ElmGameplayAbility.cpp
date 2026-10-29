// Fill out your copyright notice in the Description page of Project Settings.


#include "ElmGameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "../Effects/GameplayEffect_AbilityCost.h"
#include "../Effects/GameplayEffect_Cooldown.h"
#include "../Effects/GameplayEffect_Damage.h"
#include "../ElmGameplayTags.h"
#include "../Attributes/BaseAttributeSet.h"
#include "../../AI/TeamTypes.h"
#include "../../ElmCollisionChannels.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

UElmGameplayAbility::UElmGameplayAbility()
{
	// AbilityTask(몽타주 대기, 이벤트 대기)를 쓰려면 인스턴스가 있어야 한다.
	// InstancedPerActor는 인스턴스 하나를 재사용한다.
	// 그래서 멤버 변수에 이전 활성화 때의 값이 남아 있다.
	// 활성화할 때마다 초기화해야 하는 값은 ActivateAbility 시작 부분에서 리셋해야 한다.
	// 콤보 인덱스를 다룰 때 중요해진다.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 죽은 상태(State.Dead)에서는 어떤 어빌리티도 발동하지 않는다.
	ActivationBlockedTags.AddTag(ElmTags::State_Dead);
}

const FGameplayTagContainer* UElmGameplayAbility::GetCooldownTags() const
{
	const FGameplayTagContainer* ParentTagContainer = Super::GetCooldownTags();

	mCooldownTagsContainer.Reset();

	if (ParentTagContainer)
		mCooldownTagsContainer.AppendTags(*ParentTagContainer);
	
	if (mCooldownTag.IsValid())
		mCooldownTagsContainer.AddTag(mCooldownTag);

	return &mCooldownTagsContainer;
}

// Ability 활성화시 호출되는 함수. Ability의 로직을 구현하는 곳
void UElmGameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Commit = 비용 지불 + 쿨다운 시작. 실패하면 "취소"로 끝낸다.
	// 자식 클래스는 Super 호출 뒤 IsActive()로 계속할지 확인한다.
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UElmGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (mCooldownTime <= 0.f)
		return;

	if (!ensureMsgf(mCooldownTag.IsValid(), TEXT("%s: mCooldownTime을 쓰려면 mCooldownTag도 지정해야 합니다."),
		*GetClass()->GetName()))
		return;

	FGameplayEffectSpecHandle CooldownSpec = MakeOutgoingGameplayEffectSpec(
		UGameplayEffect_Cooldown::StaticClass(), GetAbilityLevel());

	if (!CooldownSpec.IsValid())
		return;

	CooldownSpec.Data->SetSetByCallerMagnitude(ElmTags::SetByCaller_Cooldown, mCooldownTime);
	CooldownSpec.Data->DynamicGrantedTags.AddTag(mCooldownTag);

	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpec);
}

bool UElmGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
		return false;

	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	if (!ASC)
		return false;

	const float CurrentMP = ASC->GetNumericAttribute(UBaseAttributeSet::GetMPAttribute());
	const float CurrentHP = ASC->GetNumericAttribute(UBaseAttributeSet::GetHPAttribute());

	// HP 비용으로 죽으면 안 되므로, HP는 비용보다 "많아야" 한다.
	return CurrentMP >= mMPCost && (mHPCost <= 0.f || CurrentHP > mHPCost);
}

void UElmGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (mMPCost <= 0.f && mHPCost <= 0.f)
		return;

	FGameplayEffectSpecHandle CostSpec = MakeOutgoingGameplayEffectSpec(
		UGameplayEffect_AbilityCost::StaticClass(), GetAbilityLevel());

	if (!CostSpec.IsValid())
		return;

	// GE_AbilityCost의 Modifier 두 개가 모두 SetByCaller이므로 둘 다 항상 채워야 한다.
	// (하나라도 비우면 "magnitude had not yet been set by caller" 에러 로그가 남는다)
	CostSpec.Data->SetSetByCallerMagnitude(ElmTags::SetByCaller_Cost_MP, -mMPCost);
	CostSpec.Data->SetSetByCallerMagnitude(ElmTags::SetByCaller_Cost_HP, -mHPCost);

	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CostSpec);
}

FGameplayEffectSpecHandle UElmGameplayAbility::MakeDamageSpec() const
{
	FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(
		UGameplayEffect_Damage::StaticClass(), GetAbilityLevel());

	if (DamageSpec.IsValid() && mIsMagicDamage)
		DamageSpec.Data->AddDynamicAssetTag(ElmTags::Damage_Magical);

	return DamageSpec;
}

void UElmGameplayAbility::ApplyDamage(const FHitResult& Hit)
{
	FGameplayEffectSpecHandle DamageSpec = MakeDamageSpec();

	if (!DamageSpec.IsValid())
		return;

	// HitResult로 타겟 데이터를 만들면 Context에 HitResult도 자동으로 들어간다.
	ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,
		DamageSpec, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit));
}

void UElmGameplayAbility::ApplyMeleeDamage()
{
	ACharacter* Avatar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	if (!IsValid(Avatar) || !ASC)
		return;

	const float AttackRange = ASC->GetNumericAttribute(UBaseAttributeSet::GetAttackRangeAttribute());
	const float AttackRadius = ASC->GetNumericAttribute(UBaseAttributeSet::GetAttackRadiusAttribute());

	const FVector Forward = Avatar->GetActorForwardVector();
	const FVector Start = Avatar->GetActorLocation() + Forward * Avatar->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector End = Start + Forward * AttackRange;

	TArray<FHitResult> HitArray;
	FCollisionQueryParams Params(NAME_None, false, Avatar);

	// 캐릭터 몸(Player / Monster / NPC 캡슐)에 닿았는지만 본다. 때릴지는 아래에서 팀으로 정한다.
	// 오브젝트 타입으로 찾으면 Multi Sweep은 닿은 것을 모두 돌려준다.
	Avatar->GetWorld()->SweepMultiByObjectType(HitArray, Start, End, FQuat::Identity,
		ElmChannels::MakeCharacterObjectQuery(), FCollisionShape::MakeSphere(AttackRadius), Params);

	// 한 대상의 컴포넌트가 여러 개 맞아도 데미지는 한 번만 준다.
	TSet<AActor*> DamagedActors;

	for (const FHitResult& Hit : HitArray)
	{
		AActor* Target = Hit.GetActor();

		if (!IsValid(Target) || DamagedActors.Contains(Target))
			continue;

		// 적대 관계일 때만 맞는다. (몬스터끼리, 플레이어와 중립 NPC는 서로 맞지 않는다)
		if (!IsHostile(Avatar, Target))
			continue;

		DamagedActors.Add(Target);
		ApplyDamage(Hit);
	}

#if WITH_EDITOR
	float HalfHeight = (End - Start).Size() * 0.5f + AttackRadius;
	FQuat Rotation = FRotationMatrix::MakeFromZ(Forward).ToQuat();
	// 실제로 데미지를 준 대상이 있을 때만 빨간색. (같은 팀에 닿기만 한 것은 초록색)
	FColor DebugColor = DamagedActors.Num() > 0 ? FColor::Red : FColor::Green;

	// 에디터에서는 충돌체를 그려준다.
	DrawDebugCapsule(Avatar->GetWorld(), (Start + End) / 2.f, HalfHeight, AttackRadius,
		Rotation, DebugColor, false, 1.f);
#endif
}

float UElmGameplayAbility::GetAttackPlayRate() const
{
	float AttackSpeed = 1.f;

	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		AttackSpeed = ASC->GetNumericAttribute(UBaseAttributeSet::GetAttackSpeedAttribute());

	// 스탯 로드에 실패하면 0이다. 0으로 재생하면 몽타주가 멈춰서 어빌리티가 끝나지 않는다.
	if (AttackSpeed <= 0.f)
		AttackSpeed = 1.f;

	return mMontagePlayRate * AttackSpeed;
}
