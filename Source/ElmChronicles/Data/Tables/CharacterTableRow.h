#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CharacterTableRow.generated.h"

// 모든 캐릭터(플레이어, 몬스터, NPC) 테이블 행의 공통 부분. 이 구조체로 직접 테이블을 만들지는 않는다.
// DT_Player는 FPlayerCharacterTableRow가, DT_Monster / DT_NPC는 FAICharacterTableRow를 거쳐 이것을 상속한다.
// 데이터테이블용 구조체는 반드시 FTableRowBase를 상속받아야 한다.
USTRUCT(BlueprintType)
struct FCharacterTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	int32 Level = 1;

	// 고유 능력치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float MoveSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float AttackSpeed = 1.0f;

	// 사거리 : 공격자 캡슐 표면 ~ 대상 캡슐 표면, 공격이 닿는 최대 거리 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float AttackRange = 120.f;

	// 맞는 범위 : 공격 판정 구의 반지름 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float AttackRadius = 60.f;

	// 에셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	TSoftObjectPtr<class USkeletalMesh> BodyMesh;

	// 몬스터 / NPC는 ABP_AI, 플레이어는 ABP_Player를 부모로 하는 ABP.
	// 둘 다 담을 수 있도록 공통 부모인 UAnimInstance로 받는다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	TSoftClassPtr<class UAnimInstance> AnimClass;

	// 무기가 없으면 비워둔다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	TSoftObjectPtr<class UStaticMesh> MainhandMesh;

	// 오른손 무기 위치/회전 보정 (handslot_r 기준). 메시 원점이 손잡이가 아니거나
	// 손 방향과 다른 축으로 모델링된 경우(석궁 등)에만 넣는다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	FVector MainhandLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	FRotator MainhandRotation = FRotator::ZeroRotator;

	// 왼손 장비 (handslot_l). 방패 등. 없으면 비워둔다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	TSoftObjectPtr<class UStaticMesh> OffhandMesh;

	// 머리 장비 (head). 모자/투구/후드. 캐릭터 원점 기준으로 모델링된 메시.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	TSoftObjectPtr<class UStaticMesh> HeadMesh;

	// 등 장비 (chest). 망토. 캐릭터 원점 기준으로 모델링된 메시.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	TSoftObjectPtr<class UStaticMesh> BackMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
	float CapsuleHalfHeight = 110.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
	float CapsuleRadius = 50.f;

	// 기본 공격 어빌리티. GameplayAbility_Melee / GameplayAbility_Ranged 계열(BP 자식 포함)을 넣는다.
	// 비워두면 플레이어 / 몬스터 / NPC 모두 GameplayAbility_Melee를 쓴다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	TSoftClassPtr<class UElmGameplayAbility> AttackAbility;

	// 기본 공격 몽타주. 어빌리티가 기다리는 Send Gameplay Event 노티파이가 들어 있어야 한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	TSoftObjectPtr<class UAnimMontage> AttackMontage;
};
