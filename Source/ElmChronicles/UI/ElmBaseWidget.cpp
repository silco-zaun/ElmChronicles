// Fill out your copyright notice in the Description page of Project Settings.


#include "ElmBaseWidget.h"

UElmBaseWidget::UElmBaseWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UElmBaseWidget::PlayWidgetAnimation(const FString& Name, float StartTime,
	float PlaySpeed, bool Forward, bool RestoreState, int32 LoopCount)
{
	TObjectPtr<UWidgetAnimation> Anim = mAnimMap.FindRef(Name);

	if (IsValid(Anim))
	{
		EUMGSequencePlayMode::Type PlayMode = EUMGSequencePlayMode::Forward;

		if (!Forward)
			PlayMode = EUMGSequencePlayMode::Reverse;

		PlayAnimation(Anim, StartTime, LoopCount, PlayMode,	PlaySpeed, RestoreState);
	}
}

void UElmBaseWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 위젯 애니메이션이 있을 경우 Map에 추가한다.
	UWidgetBlueprintGeneratedClass* GeneratedClass = GetWidgetTreeOwningClass();

	if (GeneratedClass)
	{
		for (auto& Anim : GeneratedClass->Animations)
		{
			FString Name = Anim->GetName();

			// 애니메이션 이름 뒤에 _INST가 붙기 때문에 제거한다.
			Name.ReplaceInline(TEXT("_INST"), TEXT(""));

			mAnimMap.Add(Name, Anim);
		}
	}
}

void UElmBaseWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UElmBaseWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UElmBaseWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UElmBaseWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}
