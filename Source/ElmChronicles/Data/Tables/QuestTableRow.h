#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "QuestTableRow.generated.h"

// DT_Quest 행. 퀘스트 하나가 한 행이고, 행 이름이 퀘스트 이름이다. (예: SkeletonRaid)
// 목표는 "TargetMonster를 TargetCount마리 처치"다. 다 잡는 순간 완료되고 그 자리에서 보상을 받는다.
USTRUCT(BlueprintType)
struct FQuestTableRow : public FTableRowBase
{
	GENERATED_BODY()

	// 퀘스트 창과 추적창에 보이는 제목
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FString Title;

	// 퀘스트 창에 보이는 설명
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FString Description;

	// 잡아야 하는 몬스터의 DT_Monster 행 이름 (예: BoneWarrior). 몬스터의 mRowName과 비교한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FName TargetMonster;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	int32 TargetCount = 1;

	// DT_Monster의 DropExp / DropGold와 같은 정수다. GE에 넣을 때 float로 바꾼다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
	int32 RewardExp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
	int32 RewardGold = 0;

	// 퀘스트를 준 NPC에게 다시 말을 걸 때 시작할 DT_Dialogue 행. 진행 중일 때 / 완료한 뒤. (ANPCCharacter::ChooseDialogueRow가 고른다))
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName ProgressDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName CompleteDialogue;
};
