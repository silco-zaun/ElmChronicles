// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "Interactable.h"
#include "GameFramework/Actor.h"
#include "LevelPortal.generated.h"

class UBoxComponent;

// 다른 레벨로 가는 문 / 동굴 입구. 플레이어가 가까이 가서 E를 누르면 mTargetLevel을 연다.
// 도착 레벨에서는 Player Start Tag가 mTargetStartTag인 PlayerStart에 선다.
UCLASS()
class ELMCHRONICLES_API ALevelPortal : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	ALevelPortal();

protected:
	// 상호작용 검색(Interaction 채널)에만 걸리는 영역. 몸을 막지는 않는다.
	UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> mBox;

	// 갈 레벨. Details 패널에서 맵 에셋을 고른다.
	// Soft 참조라서 이 포탈이 있다고 도착 맵까지 메모리에 올라오지는 않는다.
	UPROPERTY(EditAnywhere, Category = "Portal") TSoftObjectPtr<UWorld> mTargetLevel;

	// 도착 레벨에서 설 PlayerStart의 Player Start Tag.
	// 비우거나 못 찾으면 태그 없는 PlayerStart에 선다(AElmGameModeBase::ChoosePlayerStart).
	UPROPERTY(EditAnywhere, Category = "Portal") FName mTargetStartTag;

	// 안내 문구. 예: "상점 들어가기" → 화면에는 "[F] 상점 들어가기"
	UPROPERTY(EditAnywhere, Category = "Portal") FText mInteractText;

public:
	// IInteractable
	virtual FText GetInteractText() const override;
	virtual bool CanInteract(const APlayerCharacter* Player) const override;
	virtual void Interact(APlayerCharacter* Player) override;
};
