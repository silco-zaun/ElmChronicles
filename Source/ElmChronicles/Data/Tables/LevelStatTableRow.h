#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LevelStatTableRow.generated.h"

USTRUCT(BlueprintType)
struct FLevelStatTableRow : public FTableRowBase
{
	GENERATED_BODY()

	// 플레이어 전용, 0 = 최대 레벨
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	int32 RequiredExp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float PhysicalAttack = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float MagicalAttack = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float PhysicalDefense = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float MagicalDefense = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float MaxHP = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float MaxMP = 100.f;
};
