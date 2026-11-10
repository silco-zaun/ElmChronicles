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

// 퀘스트 진행 상태
UENUM(BlueprintType)
enum class EQuestState : uint8
{
	None,		// 받지 않음 (받은 퀘스트 목록에 없다)
	InProgress,
	Completed
};

// 받은 퀘스트 하나의 진행도. 값만 담고 있어서 퀘스트 컴포넌트와 메모가 같은 구조체를 그대로 복사해 쓴다.
USTRUCT(BlueprintType)
struct FQuestProgress
{
	GENERATED_BODY()

	UPROPERTY() FName QuestRow;									// DT_Quest 행 이름
	UPROPERTY() EQuestState State = EQuestState::InProgress;
	UPROPERTY() int32 KillCount = 0;							// 지금까지 잡은 목표 몬스터 수
};

UCLASS()
class ELMCHRONICLES_API UElmSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// 지금 세이브 구조의 번호. USaveSubsystem::WritePlayer가 적을 때 SaveVersion에 넣는다.
	// 2 = 맵 / 위치 추가 (메인 화면 이어하기)
	static constexpr int32 CurrentVersion = 2;

	// 세이브 구조가 바뀌었을 때 옛 세이브를 구분하기 위한 번호.
	// 기본값은 1(가장 옛 버전)로 둔다. 기본값과 같은 값은 파일에 쓰이지 않아서, 기본값을 바꾸면 번호가 없는 옛 파일도 새 번호로 읽힌다.
	UPROPERTY() int32 SaveVersion = 1;
	UPROPERTY() TArray<FInventorySaveEntry> Inventory;
	// 능력치. Base 값만 적는다. MaxHP / 공격력 같은 값은 Level로 DT_LevelStat에서 다시 계산되므로 적지 않는다.
	UPROPERTY() int32 Level = 1;
	UPROPERTY() float HP = 0.f;
	UPROPERTY() float MP = 0.f;
	UPROPERTY() float Exp = 0.f;
	UPROPERTY() float Gold = 0.f;
	// 받은 퀘스트 (진행 중 + 완료)
	UPROPERTY() TArray<FQuestProgress> Quests;
	// 장착 장비. SlotIndex = 장비칸 번호(EEquipSlot). 가방 칸과 같은 구조체를 쓴다.
	UPROPERTY() TArray<FInventorySaveEntry> Equipment;
	// 이어하기 자리 (버전 2).
	// 맵은 OpenLevel에 그대로 넘길 수 있는 패키지 이름("/Game/Maps/ElmKingdom/ElmKingdom")이다.
	// 비어 있으면 맵을 적지 않던 옛 세이브(버전 1)다.
	UPROPERTY() FString LevelName;
	UPROPERTY() FVector Location = FVector::ZeroVector;
	UPROPERTY() FRotator Rotation = FRotator::ZeroRotator;	// Yaw만 쓴다

};
