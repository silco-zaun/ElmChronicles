// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_SendGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;

	// ASC가 없는 액터(애니메이션 에디터 프리뷰 등)에는 보내지 않는다. 보내면 에러 로그가 남는다.
	if (!mEventTag.IsValid() || !UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner))
		return;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, mEventTag, FGameplayEventData());
}

FString UAnimNotify_SendGameplayEvent::GetNotifyName_Implementation() const
{
	// 노티파이를 클릭하고 Details 패널에서 Event Tag를 고르면 타임라인 표시가 태그 이름으로 바뀐다.
	return mEventTag.IsValid() ? mEventTag.ToString() : Super::GetNotifyName_Implementation();
}
