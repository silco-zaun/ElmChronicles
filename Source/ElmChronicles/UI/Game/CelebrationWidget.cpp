// Fill out your copyright notice in the Description page of Project Settings.


#include "CelebrationWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UCelebrationWidget::UCelebrationWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UCelebrationWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 띄울 문구가 없으니 숨긴 채로 시작한다.
	SetVisibility(ESlateVisibility::Collapsed);
}

void UCelebrationWidget::NativeDestruct()
{
	// 레벨 이동으로 HUD가 사라질 때 남은 줄을 버린다.
	// 그래야 멈춘 애니메이션의 OnAnimationFinished가 사라지는 위젯에 다음 문구를 띄우지 않는다.
	mPending.Empty();

	Super::NativeDestruct();
}

void UCelebrationWidget::Push(const FText& Title, const FText& SubText)
{
	mPending.Add(FCelebrationEntry{ Title, SubText });

	// 이미 하나를 보여 주는 중이면 기다린다. 그 애니메이션이 끝날 때 OnAnimationFinished가 꺼내 준다.
	if (mPending.Num() == 1)
		ShowFront();
}

void UCelebrationWidget::ShowFront()
{
	const FCelebrationEntry& Entry = mPending[0];

	mTitleText->SetText(Entry.Title);
	mSubText->SetText(Entry.SubText);

	// 보이기만 하고 클릭은 뒤로 통과시킨다. 떠 있는 동안에도 공격 / 인벤토리 클릭이 막히지 않는다.
	// 숨겨진(Collapsed) 위젯은 Tick이 돌지 않아 애니메이션도 멈추므로, 보이게 한 다음에 재생한다.
	SetVisibility(ESlateVisibility::HitTestInvisible);
	PlayAnimation(mShowAnim);

	if (mSound)
		UGameplayStatics::PlaySound2D(this, mSound);
}

void UCelebrationWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);

	// 다른 애니메이션이거나, 줄이 비었는데 불린 경우(위젯이 사라질 때)는 무시한다.
	if (Animation != mShowAnim.Get() || mPending.IsEmpty())
		return;

	mPending.RemoveAt(0);

	if (mPending.IsEmpty())
		SetVisibility(ESlateVisibility::Collapsed);
	else
		ShowFront();
}
