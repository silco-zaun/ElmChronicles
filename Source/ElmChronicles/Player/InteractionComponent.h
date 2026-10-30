// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

// 플레이어 주변의 상호작용 대상(IInteractable)을 찾아 안내 문구("[F] 대화하기")를 띄우고, F 키로 상호작용한다.
// AElmPlayerController에 붙는다. 대상이 NPC인지 포탈인지는 모르고 IInteractable만 안다.
UCLASS()
class ELMCHRONICLES_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

protected:
	// 지금 F를 누르면 상호작용할 대상. 안내 문구도 이 대상 기준으로 띄운다.
	// 대상이 사라지면 자동으로 nullptr이 되도록 약한 포인터로 들고 있는다.
	TWeakObjectPtr<AActor> mInteractTarget;
	FTimerHandle mInteractTimer;
	// true면 대상을 찾지 않고 F도 무시한다. 대화처럼 다른 기능이 상호작용을 막을 때 쓴다.
	bool mIsBlocked = false;

	// 플레이어 중심에서 이 반지름(cm) 안에 캡슐이 걸친 대상을 찾는다.
	static constexpr float InteractRadius = 200.f;
	// 대상 검색 주기(초). 매 프레임 할 필요는 없다.
	static constexpr float InteractScanInterval = 0.1f;

protected:
	virtual void BeginPlay() override;

public:
	// F 키(IA_Interact). 바인딩은 AElmPlayerController::SetupInputComponent가 한다.
	void InteractKey();
	// true면 대상을 비우고 안내 문구를 숨기고 검색을 멈춘다. false면 다음 검색(0.1초 안)부터 다시 찾는다.
	// 누가 막는지(대화, 나중에 상점 창 등)는 몰라도 된다.
	void SetBlocked(bool Blocked);

private:
	// 주변의 상호작용 대상을 찾아 mInteractTarget과 안내 문구를 갱신한다. (타이머로 0.1초마다)
	void UpdateInteractTarget();
};
