// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ElmBaseWidget.h"
#include "CelebrationWidget.generated.h"

// 줄에 세워 둘 문구 하나
struct FCelebrationEntry
{
	FText Title;
	FText SubText;
};

// 화면 가운데 축하 문구 ("LEVEL UP!", "퀘스트 완료"). WBP_PlayerHUD에 하나만 두고 UPlayerHUDWidget::ShowCelebration으로 띄운다.
// 퀘스트 보상으로 레벨이 오르면 두 문구가 같은 프레임에 들어오므로, 줄을 세워 하나씩 보여 준다. (Lyra의 Accolade와 같은 방식)
UCLASS()
class ELMCHRONICLES_API UCelebrationWidget : public UElmBaseWidget
{
	GENERATED_BODY()

public:
	UCelebrationWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// WBP_Celebration에 같은 이름의 위젯이 있어야 한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> mTitleText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> mSubText;

	// 나타났다가 사라지는 연출 한 번. 이 애니메이션의 길이가 곧 문구가 떠 있는 시간이다.
	// WBP_Celebration의 애니메이션 이름도 mShowAnim이어야 한다. (없으면 BindWidget처럼 WBP 컴파일 에러로 알려 준다)
	// UE5부터 BindWidgetAnim에는 Transient가 필요하다.
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> mShowAnim;

	// 문구가 뜰 때 소리. WBP_Celebration의 Class Defaults에서 고른다. 비워 두면 소리 없이 띄운다.
	UPROPERTY(EditAnywhere, Category = "Celebration")
	TObjectPtr<class USoundBase> mSound;

private:
	// 띄울 문구 줄. [0]이 지금 보이는 것이다.
	TArray<FCelebrationEntry> mPending;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	// 애니메이션이 끝나면 엔진이 부른다. 다음 문구를 꺼낸다.
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

public:
	// 줄 끝에 넣는다. 보이는 문구가 없으면 바로 띄운다.
	void Push(const FText& Title, const FText& SubText);

private:
	// mPending[0]을 띄우고 애니메이션을 처음부터 재생한다.
	void ShowFront();
};
