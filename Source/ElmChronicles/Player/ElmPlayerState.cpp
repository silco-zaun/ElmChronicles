// Fill out your copyright notice in the Description page of Project Settings.


#include "ElmPlayerState.h"
#include "../AbilitySystem/Components/PlayerAbilitySystemComponent.h"
#include "../AbilitySystem/Attributes/PlayerAttributeSet.h"

AElmPlayerState::AElmPlayerState()
{
	UE_LOG(LogTestOrder, Log, TEXT("AElmPlayerState::Constructor - %s"), *GetName());

	mASC = CreateDefaultSubobject<UPlayerAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	//mPlayerAbility->SetIsReplicated(true);
	//mPlayerAbility->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// ASC를 가진 액터의 서브오브젝트로 만들면 ASC가 초기화될 때 자동으로 등록된다.
	mAttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("AttributeSet"));

	// PlayerState는 기본 네트워크 갱신 빈도가 낮다. (멀티플레이 대비)
	//SetNetUpdateFrequency(100.f);
}

UAbilitySystemComponent* AElmPlayerState::GetAbilitySystemComponent() const
{
	return mASC;
}
