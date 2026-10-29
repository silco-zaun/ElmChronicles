#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ElmSaveGame.generated.h"

// 인벤토리 한 칸의 저장 정보. 포인터 없이 "값"만 담는다.
USTRUCT(BlueprintType)
struct FInventorySaveEntry
{
	GENERATED_BODY()

	UPROPERTY() int32 SlotIndex = INDEX_NONE;	// 몇 번 칸 (드래그로 정렬한 배치 유지)
	UPROPERTY() FName ItemKey;					// DT_Item 행 이름
	UPROPERTY() int32 Count = 1;
	UPROPERTY() int32 Upgrade = 0;
};

UCLASS()
class ELMCHRONICLES_API UElmSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// 세이브 구조가 바뀌었을 때 옛 세이브를 구분하기 위한 번호
	UPROPERTY() int32 SaveVersion = 1;

	UPROPERTY() TArray<FInventorySaveEntry> Inventory;

	// 능력치. Base 값만 적는다. MaxHP / 공격력 같은 값은 Level로 DT_LevelStat에서 다시 계산되므로 적지 않는다.
	UPROPERTY() int32 Level = 1;
	UPROPERTY() float HP = 0.f;
	UPROPERTY() float MP = 0.f;
	UPROPERTY() float Exp = 0.f;
	UPROPERTY() float Gold = 0.f;

	// 나중에 추가: 장착 장비, 위치, 퀘스트 진행도 ...
};
