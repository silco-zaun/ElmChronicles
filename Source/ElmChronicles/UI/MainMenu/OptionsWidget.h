// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ElmBaseWidget.h"
#include "Components/ComboBoxString.h"
#include "OptionsWidget.generated.h"

// 옵션 창. 화면 모드 / 해상도 / 그래픽 품질을 고르고 "적용"으로 바꾼다.
// 값은 엔진 UGameUserSettings가 들고 GameUserSettings.ini에 저장한다. (게임 진행 세이브 ElmSave.sav와 섞지 않는다)
// 메인 화면에 의존하지 않는다. 띄우는 쪽은 Open만 부르고, 닫기는 이 창의 "뒤로"가 한다. (나중에 B 메뉴에서도 쓸 수 있다)
UCLASS()
class ELMCHRONICLES_API UOptionsWidget : public UElmBaseWidget
{
	GENERATED_BODY()

public:
	UOptionsWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// WBP_Options에 같은 이름의 위젯이 있어야 한다.
	// 화면 모드. 항목 순서 = EWindowMode 값 (0 전체 화면, 1 테두리 없는 창, 2 창 모드)
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UComboBoxString> mWindowModeCombo;
	// 해상도. 항목 순서 = mResolutionList 순서
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UComboBoxString> mResolutionCombo;
	// 그래픽 품질. 항목 순서 = Scalability 단계 (0 낮음 ~ 3 최고)
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UComboBoxString> mQualityCombo;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> mApplyButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> mBackButton;

	// 해상도 콤보박스 항목과 같은 순서의 실제 값. 콤보박스는 글자만 들고 있어서 숫자는 여기서 꺼낸다.
	TArray<FIntPoint> mResolutionList;

public:
	// 지금 설정을 읽어 콤보박스에 골라 두고 창을 보인다.
	void Open();
	// 창을 닫는다. 고르기만 하고 적용하지 않은 값은 버린다. (다음 Open이 지금 설정을 다시 읽는다)
	void Close();

protected:
	virtual void NativeOnInitialized() override;

private:
	// 지금 설정(UGameUserSettings)을 콤보박스에 골라 둔다.
	void ReadSettings();
	// 테두리 없는 창이면 해상도 콤보박스를 끈다.
	void UpdateResolutionEnabled();

	// 콤보박스의 OnSelectionChanged와 버튼의 OnClicked는 Dynamic 델리게이트라서 UFUNCTION 함수만 연결할 수 있다.
	UFUNCTION()	void OnWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION()	void OnApplyClicked();
	UFUNCTION()	void OnBackClicked();
};
