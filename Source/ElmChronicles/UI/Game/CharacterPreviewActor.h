// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "GameFramework/Actor.h"
#include "CharacterPreviewActor.generated.h"

class AElmCharacterBase;
class USceneCaptureComponent2D;
class USkeletalMeshComponent;
class UStaticMeshComponent;

// 캐릭터 창 가운데에 보이는 캐릭터. 플레이어의 겉모습(몸 메시, ABP, 장비 메시)을 복사해서 세워 두고,
// 앞에 둔 SceneCapture2D 두 개가 같은 자리에서 찍는다. 색은 RT_CharacterPreview, 실루엣은 RT_CharacterPreviewMask에 찍고,
// M_CharacterPreview가 실루엣으로 색을 오려 낸다. 캐릭터 창(UCharacterInfoWidget)이 만들고 지운다.
// 조명은 레벨의 태양(Directional Light)과 스카이라이트를 그대로 받는다. 둘은 맵 어디에나 닿아서
// 맵 위 높은 곳의 미리보기도 플레이어와 같은 빛을 받는다. 해가 비스듬히 비추도록 BeginPlay에서 몸을 돌린다.
UCLASS()
class ELMCHRONICLES_API ACharacterPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	ACharacterPreviewActor();

protected:
	UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> mRoot;
	UPROPERTY(VisibleAnywhere) TObjectPtr<USkeletalMeshComponent> mBody;
	// 장비 메시. AElmCharacterBase::GetGearComponents와 같은 순서(오른손, 왼손, 머리, 등)
	UPROPERTY(VisibleAnywhere) TArray<TObjectPtr<UStaticMeshComponent>> mGearArray;
	// 색 캡처. 게임 화면과 같은 후처리(노출, 톤매퍼)를 거친 색을 찍는다. 후처리를 거치면 알파가 사라진다.
	UPROPERTY(VisibleAnywhere) TObjectPtr<USceneCaptureComponent2D> mCapture;
	// 실루엣 캡처. 후처리 없이 찍어서 알파(1 - 불투명도)가 남는다. 색은 쓰지 않는다.
	UPROPERTY(VisibleAnywhere) TObjectPtr<USceneCaptureComponent2D> mMaskCapture;

protected:
	virtual void BeginPlay() override;

public:
	// 창을 열 때와 장비가 바뀔 때 캐릭터 창이 부른다.
	void CopyAppearance(const AElmCharacterBase* Source);
	// 창이 열려 있는 동안만 매 프레임 찍는다.
	void SetCaptureActive(bool Active);
};
