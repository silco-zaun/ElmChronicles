// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "Components/ActorComponent.h"
#include "PopupComponent.generated.h"

// 어느 화면에서나 띄우는 팝업(옵션 창, 5단계부터 확인 상자)을 맡는다.
// 메인 화면과 게임의 컨트롤러에 같이 붙는다. 시작할 때 팝업 층(WBP_PopupLayer)을 HUD보다 위에 띄운다.
// 팝업을 열고 닫는 길은 이 컴포넌트 하나다. 팝업 위젯도 닫을 때 이 컴포넌트를 부른다.
// 입력 모드 / 커서는 화면마다 달라서 컨트롤러가 정한다.
UCLASS()
class ELMCHRONICLES_API UPopupComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPopupComponent();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UPopupLayerWidget> mPopupLayerWidgetClass;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UPopupLayerWidget> mPopupLayerWidget;

private:
	// 클수록 위에 그린다. HUD(WBP_PlayerHUD)와 메인 메뉴(WBP_MainMenu)는 기본값 0으로 붙는다.
	static constexpr int32 PopupLayerZOrder = 10;

protected:
	virtual void BeginPlay() override;

public:
	// 컨트롤러에 붙은 팝업 컴포넌트. 컨트롤러가 없거나 컴포넌트가 없으면 nullptr.
	// 팝업 위젯은 GetOwningPlayer()를 넘긴다. 어느 컨트롤러 밑에서도 같은 길로 찾기 위해서다.
	static UPopupComponent* Get(const class APlayerController* PlayerController);

	// 옵션 창을 연다. 창이 지금 설정을 읽어 보여 준다.
	void OpenOptions();
	// 옵션 창을 닫는다. 옵션 창의 "뒤로"도 이것으로 닫는다.
	void CloseOptions();
};
