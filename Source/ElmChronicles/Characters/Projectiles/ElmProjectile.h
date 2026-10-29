// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "GenericTeamAgentInterface.h"
#include "ElmProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

// 원거리 공격의 투사체(화살, 마법탄). 플레이어 / 몬스터 / NPC가 모두 쓴다.
// 데미지 스펙과 쏜 팀을 싣고 날아가다가 그 팀의 적과 겹치면 스펙을 적용하고 사라진다. 벽에 막혀도 사라진다.
UCLASS()
class ELMCHRONICLES_API AElmProjectile : public AActor
{
	GENERATED_BODY()

public:
	AElmProjectile();

protected:
	// 충돌 판정용 구. 루트이고, ProjectileMovement가 이 구를 움직인다.
	UPROPERTY(VisibleAnywhere) TObjectPtr<USphereComponent> mCollision;
	// 보이기만 하는 메시. BP에서 화살 / 마법구 메시를 지정한다.
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> mMesh;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UProjectileMovementComponent> mMovement;
	// 발사한 어빌리티가 만들어 준 데미지 스펙. 맞은 대상에게 그대로 적용한다.
	UPROPERTY() FGameplayEffectSpecHandle mDamageSpec;
	// 쏜 캐릭터의 팀. 쏘는 순간 기억해 둔다. (맞는 순간에는 쏜 캐릭터가 죽어서 사라졌을 수 있다)
	FGenericTeamId mTeamId = FGenericTeamId::NoTeam;

public:
	// 두 함수 모두 SpawnActorDeferred와 FinishSpawning 사이에 호출한다.
	void SetDamageSpec(const FGameplayEffectSpecHandle& DamageSpec) { mDamageSpec = DamageSpec; }
	void SetTeamId(const FGenericTeamId& TeamId) { mTeamId = TeamId; }

private:
	UFUNCTION()
	void OnCollisionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBlocked(const FHitResult& ImpactResult);
};
