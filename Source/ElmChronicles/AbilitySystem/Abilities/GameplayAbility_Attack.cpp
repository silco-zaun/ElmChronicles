// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_Attack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimMontage.h"
#include "../ElmGameplayTags.h"
#include "../../Characters/ElmCharacterBase.h"

UGameplayAbility_Attack::UGameplayAbility_Attack()
{
	// 활성 중에 다시 발동하면 현재 활성을 끝내고 새로 시작한다. (= 다음 콤보 타)
	// 콤보 구간이 없는 몽타주는 CanActivateAbility에서 막히므로 단타 공격에는 영향이 없다.
	bRetriggerInstancedAbility = true;
}

bool UGameplayAbility_Attack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	// 공격 중에는 콤보 입력 구간에서만 다음 타를 받는다. 구간 밖 입력은 무시한다.
	// (엔진은 이 검사를 통과해야 Retrigger로 현재 활성을 끝낸다)
	if (IsActive() && !mIsComboWindow)
		return false;

	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGameplayAbility_Attack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 부모에서 Commit(비용 + 쿨다운)한다. 콤보는 한 타마다 한 번씩 처리된다.
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Commit에 실패했다면 이미 종료된 상태다.
	if (!IsActive())
		return;

	// 공격 몽타주는 DT의 AttackMontage를 캐릭터 공통 부모(ElmCharacterBase)가 들고 있다.
	AElmCharacterBase* Character = Cast<AElmCharacterBase>(GetAvatarActorFromActorInfo());
	UAnimMontage* Montage = IsValid(Character) ? Character->GetAttackMontage() : nullptr;
	const int32 SectionCount = Montage ? Montage->GetNumSections() : 0;

	if (SectionCount == 0)
	{
		mComboIndex = 0;
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// 이번 타의 섹션을 정하고, 다음 타 번호로 넘겨 둔다. (Attack1 → Attack2 → Attack3 → Attack1)
	// 섹션이 하나뿐이면 항상 0번이라 단타가 된다.
	const FName SectionName = Montage->GetSectionName(mComboIndex % SectionCount);
	mComboIndex = (mComboIndex + 1) % SectionCount;

	UE_LOG(LogTestDebug, Warning, TEXT("Montage : %s, SectionCount : %d, Section : %s, ComboIndex : %d"),
		*Montage->GetName(), SectionCount , *SectionName.ToString(), mComboIndex);

	// 새 타가 시작되면 콤보 구간은 닫힌다. 이번 섹션의 ComboOpen 노티파이가 와야 다시 열린다.
	mIsComboWindow = false;

	// ① 이벤트 대기를 먼저 걸어둔다. 몽타주보다 늦게 걸면 신호를 놓칠 수 있다.
	UAbilityTask_WaitGameplayEvent* WaitHit =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ElmTags::Event_Attack_Hit);
	WaitHit->EventReceived.AddDynamic(this, &UGameplayAbility_Attack::OnHitEvent);
	WaitHit->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitComboOpen =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ElmTags::Event_Attack_ComboOpen);
	WaitComboOpen->EventReceived.AddDynamic(this, &UGameplayAbility_Attack::OnComboOpen);
	WaitComboOpen->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitComboClose =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ElmTags::Event_Attack_ComboClose);
	WaitComboClose->EventReceived.AddDynamic(this, &UGameplayAbility_Attack::OnComboClose);
	WaitComboClose->ReadyForActivation();

	// ② 이번 섹션부터 몽타주를 재생한다. 어떤 식으로든 끝나면 어빌리티를 종료한다.
	UAbilityTask_PlayMontageAndWait* PlayMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, Montage, GetAttackPlayRate(), SectionName);

	PlayMontage->OnCompleted.AddDynamic(this, &UGameplayAbility_Attack::OnMontageEnded);
	PlayMontage->OnBlendOut.AddDynamic(this, &UGameplayAbility_Attack::OnMontageEnded);
	PlayMontage->OnInterrupted.AddDynamic(this, &UGameplayAbility_Attack::OnMontageEnded);
	PlayMontage->OnCancelled.AddDynamic(this, &UGameplayAbility_Attack::OnMontageEnded);
	PlayMontage->ReadyForActivation();
}

void UGameplayAbility_Attack::OnHitEvent(FGameplayEventData Payload)
{
	OnAttackHit();
}

void UGameplayAbility_Attack::OnComboOpen(FGameplayEventData Payload)
{
	UE_LOG(LogTestDebug, Warning, TEXT("UGameplayAbility_Attack::OnComboOpen"));

	mIsComboWindow = true;
}

void UGameplayAbility_Attack::OnComboClose(FGameplayEventData Payload)
{
	UE_LOG(LogTestDebug, Warning, TEXT("UGameplayAbility_Attack::OnComboClose"));

	mIsComboWindow = false;
}

void UGameplayAbility_Attack::OnMontageEnded()
{
	UE_LOG(LogTestDebug, Warning, TEXT("UGameplayAbility_Attack::OnMontageEnded"));

	// 콤보로 이어지지 못하고 몽타주가 끝났다. (자연 종료, 피격, 취소) 다음 공격은 1타부터.
	// 콤보로 재발동할 때는 몽타주 태스크가 델리게이트를 먼저 끊기 때문에 이 함수가 불리지 않는다.
	mComboIndex = 0;
	mIsComboWindow = false;

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
