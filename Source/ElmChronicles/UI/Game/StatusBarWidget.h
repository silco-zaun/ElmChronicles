// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ElmBaseWidget.h"
#include "StatusBarWidget.generated.h"

struct FOnAttributeChangeData;

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UStatusBarWidget : public UElmBaseWidget
{
	GENERATED_BODY()
	
public:
	UStatusBarWidget(const FObjectInitializer& ObjectInitializer);

protected:
	TWeakObjectPtr<class UAbilitySystemComponent> mASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> mNameText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> mHPBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> mMPBar;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	void SetName(const FString& Name);

private:
	void SetHP(float HP, float MaxHP);
	void SetMP(float MP, float MaxMP);
	void OnAttributeChanged(const FOnAttributeChangeData& Data);
	void RefreshStatus();
};
