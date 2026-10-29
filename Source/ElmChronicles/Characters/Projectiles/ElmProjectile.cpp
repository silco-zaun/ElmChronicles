// Fill out your copyright notice in the Description page of Project Settings.


#include "ElmProjectile.h"
#include "../../AI/TeamTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AElmProjectile::AElmProjectile()
{
	// 이동은 ProjectileMovement가 하므로 액터 Tick은 필요 없다.
	PrimaryActorTick.bCanEverTick = false;

	mCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	mCollision->InitSphereRadius(10.f);
	SetRootComponent(mCollision);

	// 벽과 바닥(WorldStatic / WorldDynamic)에는 막히고, 캐릭터 캡슐(Pawn)과는 겹치기만 한다.
	// 겹친 캐릭터를 맞힐지는 OnCollisionOverlap에서 팀으로 정한다.
	mCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	mCollision->SetCollisionObjectType(ECC_WorldDynamic);
	mCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	mCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	mCollision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	mCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	mCollision->SetGenerateOverlapEvents(true);

	mMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	mMesh->SetupAttachment(mCollision);
	mMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	mMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	mMovement->SetUpdatedComponent(mCollision);
	mMovement->InitialSpeed = 1500.f;
	mMovement->MaxSpeed = 1500.f;
	// 중력 0 = 직선으로 날아간다. 포물선을 원하면 BP에서 값을 올린다.
	mMovement->ProjectileGravityScale = 0.f;
	// 날아가는 방향으로 메시가 회전한다.
	mMovement->bRotationFollowsVelocity = true;

	// 아무것도 못 맞히면 3초 뒤 스스로 사라진다.
	InitialLifeSpan = 3.f;

	// 생성자에서 연결한다. 스폰 직후 겹침 검사는 BeginPlay보다 먼저 일어나서, BeginPlay에서 연결하면 놓칠 수 있다.
	mCollision->OnComponentBeginOverlap.AddDynamic(this, &AElmProjectile::OnCollisionOverlap);
	mMovement->OnProjectileStop.AddDynamic(this, &AElmProjectile::OnBlocked);
}

void AElmProjectile::OnCollisionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 쏜 팀의 적이 아니면 통과한다. 쏜 캐릭터 자신(스폰 순간부터 겹쳐 있다), 같은 팀 몬스터,
	// 중립 NPC, 팀이 없는 액터(트리거 볼륨 등)가 모두 여기서 걸러진다.
	if (!IsHostile(mTeamId, OtherActor))
		return;

	// 플레이어는 PlayerState의 ASC가 나온다.
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor);

	if (!TargetASC)
		return;

	if (mDamageSpec.IsValid())
	{
		// 날아가다 맞았으면 맞은 위치를 Context에 넣는다. (근접 공격의 ApplyDamage와 같은 정보)
		if (bFromSweep)
		{
			FGameplayEffectContextHandle Context = mDamageSpec.Data->GetContext();
			Context.AddHitResult(SweepResult, true);
		}

		TargetASC->ApplyGameplayEffectSpecToSelf(*mDamageSpec.Data.Get());
	}

	// 한 번 맞히면 사라진다.
	Destroy();
}

void AElmProjectile::OnBlocked(const FHitResult& ImpactResult)
{
	// 벽이나 바닥에 막혀 멈췄다.
	Destroy();
}
