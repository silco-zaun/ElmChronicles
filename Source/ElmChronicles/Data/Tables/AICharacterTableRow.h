#pragma once

#include "CoreMinimal.h"
#include "CharacterTableRow.h"
#include "AICharacterTableRow.generated.h"

// AI 캐릭터(몬스터, NPC) 테이블 행의 공통 부분. 이 구조체로 직접 테이블을 만들지는 않는다.
// 캐릭터 공통 항목(FCharacterTableRow)에 AI 전용 항목(감지)을 더한다.
// DT_Monster는 FMonsterCharacterTableRow, DT_NPC는 FNPCCharacterTableRow가 이것을 상속해서 전용 항목을 더한다.
USTRUCT(BlueprintType)
struct FAICharacterTableRow : public FCharacterTableRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float DetectRange = 800.0f;
};
