// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayAbility_Attack.h"
#include "GameplayAbility_Ranged.generated.h"

class AElmProjectile;

// Event.Attack.Hit 시점에 투사체를 쏘는 원거리 공격. 플레이어 / 몬스터 / NPC가 모두 쓴다.
// 데미지 스펙은 쏘는 순간 만들어서 투사체에 실어 보낸다. (맞는 순간에는 어빌리티가 이미 끝났을 수 있다)
UCLASS()
class ELMCHRONICLES_API UGameplayAbility_Ranged : public UGameplayAbility_Attack
{
	GENERATED_BODY()

protected:
	// 쏠 투사체. BP 자식에서 화살 / 마법탄 BP로 지정한다.
	UPROPERTY(EditDefaultsOnly, Category = "Ranged")
	TSubclassOf<AElmProjectile> mProjectileClass;

	// 투사체가 나가는 소켓(본) 이름. 메시에 없으면 캡슐 앞쪽에서 쏜다.
	UPROPERTY(EditDefaultsOnly, Category = "Ranged")
	FName mMuzzleSocket = TEXT("handslot_r");

protected:
	virtual void OnAttackHit() override;
};
