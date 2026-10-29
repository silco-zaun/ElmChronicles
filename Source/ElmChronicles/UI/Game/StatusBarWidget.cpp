// Fill out your copyright notice in the Description page of Project Settings.


#include "StatusBarWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "../../AbilitySystem/Attributes/BaseAttributeSet.h"

namespace
{
	// 상태바가 지켜보는 Attribute 목록 (이 cpp 안에서만 사용)
	TArray<FGameplayAttribute> GetStatusAttributes()
	{
		return {
			UBaseAttributeSet::GetHPAttribute(), UBaseAttributeSet::GetMaxHPAttribute(),
			UBaseAttributeSet::GetMPAttribute(), UBaseAttributeSet::GetMaxMPAttribute() };
	}
}

UStatusBarWidget::UStatusBarWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UStatusBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UStatusBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// ASC는 PlayerState가 소유한다. IAbilitySystemInterface를 통해 꺼낸다.
	UAbilitySystemComponent* ASC =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwningPlayerState());

	if (!ASC)
		return;

	mASC = ASC;

	for (const FGameplayAttribute& Attribute : GetStatusAttributes())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(Attribute)
			.AddUObject(this, &UStatusBarWidget::OnAttributeChanged);
	}

	// 구독 이전의 현재 값으로 한 번 그린다.
	RefreshStatus();
}

void UStatusBarWidget::NativeDestruct()
{
	if (UAbilitySystemComponent* AbilitySystem = mASC.Get())
	{
		for (const FGameplayAttribute& Attribute : GetStatusAttributes())
			AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attribute).RemoveAll(this);
	}

	mASC.Reset();

	Super::NativeDestruct();
}

void UStatusBarWidget::SetName(const FString& Name)
{
	mNameText->SetText(FText::FromString(Name));
}

void UStatusBarWidget::SetHP(float HP, float MaxHP)
{
	mHPBar->SetPercent(MaxHP > 0.f ? HP / MaxHP : 0.f);
}

void UStatusBarWidget::SetMP(float MP, float MaxMP)
{
	mMPBar->SetPercent(MaxMP > 0.f ? MP / MaxMP : 0.f);
}

void UStatusBarWidget::OnAttributeChanged(const FOnAttributeChangeData& Data)
{
	RefreshStatus();
}

void UStatusBarWidget::RefreshStatus()
{
	UAbilitySystemComponent* AbilitySystem = mASC.Get();

	if (!AbilitySystem)
		return;

	SetHP(AbilitySystem->GetNumericAttribute(UBaseAttributeSet::GetHPAttribute()),
		AbilitySystem->GetNumericAttribute(UBaseAttributeSet::GetMaxHPAttribute()));
	SetMP(AbilitySystem->GetNumericAttribute(UBaseAttributeSet::GetMPAttribute()),
		AbilitySystem->GetNumericAttribute(UBaseAttributeSet::GetMaxMPAttribute()));
}
