#pragma once

#include "CoreMinimal.h"
#include "UITypes.generated.h"

// 슬롯 위젯이 어느 UI(인벤토리/스킬창/퀵슬롯)에 속하는지 구분한다.
UENUM(BlueprintType)
enum class ESlotType : uint8
{
	Inventory,
	Skill,
	Quick
};
