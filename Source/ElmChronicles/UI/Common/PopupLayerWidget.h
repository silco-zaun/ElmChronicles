// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ElmBaseWidget.h"
#include "PopupLayerWidget.generated.h"

// 어느 화면(메인 화면, 게임)에서나 띄우는 팝업을 모아 두는 층. WBP_PopupLayer의 부모.
// UPopupComponent가 만들어 HUD보다 높은 ZOrder로 화면에 붙인다.
// 팝업의 자리(화면 전체, 가운데 등)는 WBP_PopupLayer 안의 슬롯이 정한다.
// 팝업이 모두 닫혀 있으면 클릭을 가로채지 않는다. (UserWidget / Canvas Panel의 기본 Visibility가 SelfHitTestInvisible)
UCLASS()
class ELMCHRONICLES_API UPopupLayerWidget : public UElmBaseWidget
{
	GENERATED_BODY()

public:
	UPopupLayerWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// WBP_PopupLayer에 같은 이름의 위젯이 있어야 한다.
	// 옵션 창. 화면 전체 크기로 놓는다. (창의 반투명 배경이 뒤쪽 클릭을 막는다)
	UPROPERTY(meta = (BindWidget)) TObjectPtr<class UOptionsWidget> mOptionsWidget;

public:
	class UOptionsWidget* GetOptions() const { return mOptionsWidget; }
};
