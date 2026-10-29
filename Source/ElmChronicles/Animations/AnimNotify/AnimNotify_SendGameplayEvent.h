// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AnimNotify_SendGameplayEvent.generated.h"

// 노티파이 시점에 메시 주인(ASC를 가진 액터)에게 GameplayEvent를 보낸다.
// 몽타주에서 태그만 골라 쓰면 되므로, 이벤트마다 노티파이 클래스나 함수를 만들 필요가 없다.
UCLASS(meta = (DisplayName = "Send Gameplay Event"))
class ELMCHRONICLES_API UAnimNotify_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

protected:
	// 보낼 이벤트 태그. 태그 선택창에는 Event.* 만 보인다.
	UPROPERTY(EditAnywhere, Category = "GameplayEvent", meta = (Categories = "Event", DisplayName = "Event Tag"))
	FGameplayTag mEventTag;

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	// 타임라인에 클래스 이름 대신 태그 이름을 보여준다.
	virtual FString GetNotifyName_Implementation() const override;
};
