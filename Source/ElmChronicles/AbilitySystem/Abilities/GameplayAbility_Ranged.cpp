// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_Ranged.h"
#include "../../Characters/Projectiles/ElmProjectile.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

void UGameplayAbility_Ranged::OnAttackHit()
{
	ACharacter* Avatar = Cast<ACharacter>(GetAvatarActorFromActorInfo());

	if (!IsValid(Avatar))
		return;

	if (!ensureMsgf(mProjectileClass != nullptr, TEXT("%s: mProjectileClass를 지정해야 합니다."),
		*GetClass()->GetName()))
		return;

	// ① 발사 위치. 소켓이 있으면 소켓에서, 없으면 캡슐 앞쪽에서 쏜다.
	USkeletalMeshComponent* Mesh = Avatar->GetMesh();
	const FVector SpawnLocation = Mesh->DoesSocketExist(mMuzzleSocket)
		? Mesh->GetSocketLocation(mMuzzleSocket)
		: Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * Avatar->GetCapsuleComponent()->GetScaledCapsuleRadius();

	// ② 조준 방향. AI는 BTTask_MonsterAttack이 SetFocus로 정해 둔 타겟을, 사람은 카메라가 보는 방향을 조준한다.
	// 둘 다 아니면(타겟이 없는 AI 등) 정면으로 쏜다.
	FVector Direction = Avatar->GetActorForwardVector();

	if (AAIController* AIController = Cast<AAIController>(Avatar->GetController()))
	{
		if (AActor* Target = AIController->GetFocusActor())
			Direction = (Target->GetActorLocation() - SpawnLocation).GetSafeNormal();
	}
	else if (Avatar->IsPlayerControlled())
		Direction = Avatar->GetControlRotation().Vector();

	// ③ 데미지 스펙. 지금(발사 순간)의 공격력이 담긴다.
	FGameplayEffectSpecHandle DamageSpec = MakeDamageSpec();

	if (!DamageSpec.IsValid())
		return;

	// ④ 지연 스폰. 만들기 → 스펙 싣기 → 완성 순서라서, 스폰 직후 겹쳐도 스펙이 이미 들어 있다.
	const FTransform SpawnTransform(Direction.Rotation(), SpawnLocation);

	AElmProjectile* Projectile = Avatar->GetWorld()->SpawnActorDeferred<AElmProjectile>(
		mProjectileClass, SpawnTransform, Avatar, Avatar, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!Projectile)
		return;

	Projectile->SetDamageSpec(DamageSpec);
	// 쏜 캐릭터의 팀. 투사체가 맞힐 대상을 이 팀 기준으로 고른다.
	Projectile->SetTeamId(FGenericTeamId::GetTeamIdentifier(Avatar));
	Projectile->FinishSpawning(SpawnTransform);
}
