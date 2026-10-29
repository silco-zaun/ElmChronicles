// Fill out your copyright notice in the Description page of Project Settings.


#include "ElmCharacterBase.h"
#include "../AbilitySystem/Attributes/BaseAttributeSet.h"
#include "../AbilitySystem/ElmGameplayTags.h"
#include "../Data/Tables/CharacterTableRow.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "AnimationRuntime.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"

AElmCharacterBase::AElmCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 메시는 보이기만 한다. 충돌은 캡슐이 맡는다.
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 무기 메시는 보이기만 하고, 공격 판정은 Sweep으로 처리한다.
	mMainhand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainhandComponent"));
	mMainhand->SetupAttachment(GetMesh(), TEXT("handslot_r"));
	mMainhand->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	mOffhand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OffhandComponent"));
	mOffhand->SetupAttachment(GetMesh(), TEXT("handslot_l"));
	mOffhand->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	mHeadGear = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadComponent"));
	mHeadGear->SetupAttachment(GetMesh());
	mHeadGear->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	mBackGear = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestComponent"));
	mBackGear->SetupAttachment(GetMesh());
	mBackGear->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AElmCharacterBase::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	mTeam = static_cast<ETeamType>(TeamID.GetId());
}

FGenericTeamId AElmCharacterBase::GetGenericTeamId() const
{
	return MakeTeamId(mTeam);
}

bool AElmCharacterBase::IsDead() const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	return ASC && ASC->HasMatchingGameplayTag(ElmTags::State_Dead);
}

void AElmCharacterBase::AttachBindPoseGear(UStaticMeshComponent* Gear, FName BoneName)
{
	USkeletalMesh* Body = GetMesh()->GetSkeletalMeshAsset();

	if (!Gear || !Body)
		return;

	const FReferenceSkeleton& RefSkeleton = Body->GetRefSkeleton();
	const int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);

	if (BoneIndex == INDEX_NONE)
		return;

	// 바인드 포즈(T포즈)에서 이 본이 캐릭터 원점 기준으로 어디에 있는지.
	const FTransform BoneRefPose = FAnimationRuntime::GetComponentSpaceTransformRefPose(RefSkeleton, BoneIndex);

	// 본에 붙이고 본 위치만큼 되돌려 놓는다. T포즈에서는 원래 자리, 애니메이션 중에는 본을 따라간다.
	Gear->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, BoneName);
	Gear->SetRelativeTransform(BoneRefPose.Inverse());
}

void AElmCharacterBase::InitAsset(const FCharacterTableRow& Info)
{
	// 발이 캡슐 바닥에 오도록 메시를 HalfHeight만큼 내린다.
	GetCapsuleComponent()->SetCapsuleSize(Info.CapsuleRadius, Info.CapsuleHalfHeight);
	GetMesh()->SetRelativeLocation(FVector(0.0, 0.0, -Info.CapsuleHalfHeight));

	// 메시를 먼저, ABP를 나중에 지정한다. (ABP는 메시의 스켈레톤 위에서 동작한다)
	if (USkeletalMesh* BodyMesh = Info.BodyMesh.LoadSynchronous())
		GetMesh()->SetSkeletalMeshAsset(BodyMesh);

	if (UClass* AnimClass = Info.AnimClass.LoadSynchronous())
		GetMesh()->SetAnimInstanceClass(AnimClass);

	// 무기가 지정된 캐릭터만 붙인다.
	if (UStaticMesh* MainhandMesh = Info.MainhandMesh.LoadSynchronous())
	{
		mMainhand->SetStaticMesh(MainhandMesh);

		// Details 패널의 Location / Rotation과 같은 값이다. (소켓 기준 상대 트랜스폼)
		mMainhand->SetRelativeLocationAndRotation(Info.MainhandLocation, Info.MainhandRotation);
	}

	if (UStaticMesh* OffhandMesh = Info.OffhandMesh.LoadSynchronous())
		mOffhand->SetStaticMesh(OffhandMesh);

	if (UStaticMesh* HeadMesh = Info.HeadMesh.LoadSynchronous())
	{
		mHeadGear->SetStaticMesh(HeadMesh);
		AttachBindPoseGear(mHeadGear, TEXT("head"));
	}

	if (UStaticMesh* BackMesh = Info.BackMesh.LoadSynchronous())
	{
		mBackGear->SetStaticMesh(BackMesh);
		AttachBindPoseGear(mBackGear, TEXT("chest"));
	}

	mAttackMontage = Info.AttackMontage.LoadSynchronous();
}

void AElmCharacterBase::BindAttributeDelegates(UAbilitySystemComponent* ASC, UBaseAttributeSet* AttributeSet)
{
	if (!ASC || !AttributeSet)
		return;

	// 재빙의 등으로 다시 불려도 중복 연결되지 않게 먼저 지운다.
	ASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMoveSpeedAttribute()).RemoveAll(this);
	AttributeSet->mOnDamaged.RemoveAll(this);
	AttributeSet->mOnDeath.RemoveAll(this);

	// 이동속도 Attribute가 바뀌면(초기화, 슬로우 디버프 등) 실제 이동속도에 반영한다.
	ASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMoveSpeedAttribute())
		.AddUObject(this, &AElmCharacterBase::OnMoveSpeedAttributeChanged);

	// 피격 / 사망은 AttributeSet이 알려준다. (HP 비용이나 포션으로 HP가 바뀌어도 반응하지 않는다)
	AttributeSet->mOnDamaged.AddUObject(this, &AElmCharacterBase::HandleDamaged);
	AttributeSet->mOnDeath.AddUObject(this, &AElmCharacterBase::HandleDeath);
}

void AElmCharacterBase::HandleDamaged(AActor* Causer, float Damage)
{
	// AttributeSet이 "맞고 살아남았을 때"만 부른다. IsDead()는 중복 호출 대비 안전장치.
	if (IsDead())
		return;

	OnDamagedBy(Causer, Damage);
}

void AElmCharacterBase::HandleDeath(AActor* Killer)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	// AttributeSet이 HP가 0이 된 순간 한 번만 부른다. IsDead()는 중복 호출 대비 안전장치.
	if (!ASC || IsDead())
		return;

	// 죽음 상태 태그. 이후 모든 어빌리티 발동이 막히고(ActivationBlockedTags), 몬스터 AI는 이 대상을 타겟에서 뺀다.
	ASC->AddLooseGameplayTag(ElmTags::State_Dead);

	// 진행 중인 공격 등 모든 어빌리티를 취소한다. (PlayMontageAndWait가 몽타주도 함께 멈춘다)
	ASC->CancelAllAbilities();

	OnKilledBy(Killer);
}

void AElmCharacterBase::OnMoveSpeedAttributeChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}
