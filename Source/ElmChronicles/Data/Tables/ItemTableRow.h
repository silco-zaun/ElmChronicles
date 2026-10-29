#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemTableRow.generated.h"

class UTexture2D;
class UStaticMesh;

UENUM(BlueprintType)
enum class EItemType : uint8 { Weapon, Armor, Potion, Necklace };

UENUM(BlueprintType)
enum class EItemUILayerType : uint8 { Single, Layered };

UENUM(BlueprintType)
enum class EItemOptionType : uint8
{
	PhysicalAttack, MagicalAttack, PhysicalDefense, MagicalDefense,
	HPRecovery, MPRecovery
};

USTRUCT(BlueprintType)
struct FItemOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Option")
	EItemOptionType Type = EItemOptionType::PhysicalAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Option")
	float Value = 0.0f;
};

// 데이터테이블용 구조체는 반드시 FTableRowBase를 상속받아야 한다.
USTRUCT(BlueprintType)
struct FItemTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	EItemType Type = EItemType::Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Price")
	int32 BuyPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Price")
	int32 SellPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Option")
	TArray<FItemOption> Options;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	EItemUILayerType UILayerType = EItemUILayerType::Single;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	TSoftObjectPtr<UStaticMesh> Mesh;
};


