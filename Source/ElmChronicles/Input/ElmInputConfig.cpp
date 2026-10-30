// Fill out your copyright notice in the Description page of Project Settings.


#include "ElmInputConfig.h"
#include "Engine/LocalPlayer.h"

UElmInputConfig::UElmInputConfig()
{
}

TObjectPtr<UInputAction> UElmInputConfig::FindInputAction(const FString& Name) const
{
	return mInputActionMap.FindRef(Name);
}

FText UElmInputConfig::FindMappedKeyText(const ULocalPlayer* LocalPlayer, const FString& Name) const
{
	// 키는 IMC 에셋이 아니라 서브시스템에 묻는다. 지금 켜진 IMC들을 합친 실제 매핑이라서
	// 꺼진 IMC는 빠지고, 나중에 플레이어 키 재설정을 넣어도 그대로 반영된다.
	const UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

	if (!InputSubsystem)
		return FText::GetEmpty();

	const TArray<FKey> Keys = InputSubsystem->QueryKeysMappedToAction(FindInputAction(Name));

	// IMC는 AddMappingContext를 부른 프레임의 끝에 적용되므로 그 전에 물으면 비어 있다.
	if (Keys.IsEmpty())
		return FText::GetEmpty();

	// 키가 여러 개면 첫 번째. false = 짧은 이름
	return Keys[0].GetDisplayName(false);
}
