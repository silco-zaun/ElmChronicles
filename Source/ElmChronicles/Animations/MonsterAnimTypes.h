#pragma once

#include "CoreMinimal.h"
#include "MonsterAnimTypes.generated.h"

UENUM(BlueprintType)
enum class EMonsterAnimType : uint8
{
	Idle,
	Walk,
	Run,
	Attack,
	Death,
	End
};
