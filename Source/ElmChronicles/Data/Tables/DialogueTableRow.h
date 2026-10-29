#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DialogueTableRow.generated.h"

// DT_Dialogue 행. 대사 한 줄이 한 행이고, NextRow로 다음 줄을 잇는다.
// 대화는 NPC의 DialogueRow(DT_NPC)에서 시작해서 NextRow가 None인 행에서 끝난다.
USTRUCT(BlueprintType)
struct FDialogueTableRow : public FTableRowBase
{
	GENERATED_BODY()

	// 말하는 사람 이름. 대화창에 그대로 보인다. (예: "선술집 주인", "카일")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FString Speaker;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FString Text;

	// 다음 대사 행 이름. None이면 이 줄에서 대화가 끝난다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName NextRow;
};
