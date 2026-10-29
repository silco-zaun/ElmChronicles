#pragma once

#include "CoreMinimal.h"
#include "AICharacterTableRow.h"
#include "NPCCharacterTableRow.generated.h"

// DT_NPC 행. AI 공통 항목(FAICharacterTableRow)에 NPC 전용 항목을 더한다.
USTRUCT(BlueprintType)
struct FNPCCharacterTableRow : public FAICharacterTableRow
{
	GENERATED_BODY()

	// 필수 NPC(퀘스트 NPC). true면 HP가 0이 되어도 죽지 않고 쓰러졌다가 ReviveTime초 뒤 일어난다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	bool IsEssential = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	float ReviveTime = 10.f;

	// 대화 시작 행 이름 (DT_Dialogue). 비워두면(None) 대화할 수 없다. (Step 5에서 쓴다)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	FName DialogueRow;
};
