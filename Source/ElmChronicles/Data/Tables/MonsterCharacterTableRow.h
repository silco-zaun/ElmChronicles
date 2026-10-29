#pragma once

#include "CoreMinimal.h"
#include "AICharacterTableRow.h"
#include "MonsterCharacterTableRow.generated.h"

UENUM(BlueprintType)
enum class EMonsterType : uint8
{
	Normal,
	Epic,
	Boss
};

// 등급별 배율. 등급에 따라 달라지는 수치는 여기에서만 관리한다.

// 크기 배율. 캡슐과 메시에 같이 곱한다.
inline float GetMonsterTypeSizeScale(EMonsterType Type)
{
	switch (Type)
	{
	case EMonsterType::Epic:	return 1.5f;
	case EMonsterType::Boss:	return 2.0f;
	default:					return 1.0f;
	}
}

// 체력 배율. MaxHP에 곱한다.
inline float GetMonsterTypeHPScale(EMonsterType Type)
{
	switch (Type)
	{
	case EMonsterType::Epic:	return 1.5f;
	case EMonsterType::Boss:	return 2.0f;
	default:					return 1.0f;
	}
}

USTRUCT(BlueprintType)
struct FDropItemInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DropItem")
	FName ItemKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DropItem")
	float Percent = 0.0f;
};

// DT_Monster 행. AI 공통 항목(FAICharacterTableRow)에 몬스터 전용 항목(등급, 보상)을 더한다.
USTRUCT(BlueprintType)
struct FMonsterCharacterTableRow : public FAICharacterTableRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	EMonsterType Type = EMonsterType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
	int32 DropExp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
	int32 DropGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
	TArray<FDropItemInfo> DropItems;
};
