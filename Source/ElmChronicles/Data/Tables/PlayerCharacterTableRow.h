#pragma once

#include "CoreMinimal.h"
#include "CharacterTableRow.h"
#include "PlayerCharacterTableRow.generated.h"

UENUM(BlueprintType)
enum class EPlayerJob : uint8
{
	Knight,
	Archer,
	Wizard
};

// DT_Player 행. 캐릭터 공통 항목(FCharacterTableRow)에 플레이어 전용 항목(직업, 외형)을 더한다.
USTRUCT(BlueprintType)
struct FPlayerCharacterTableRow : public FCharacterTableRow
{
	GENERATED_BODY()

	// 부모 기본값(300)은 AI 기준이라 플레이어 기본 이동 속도로 덮어쓴다.
	// 새 행을 만들 때의 기본값일 뿐이고, 테이블에 적힌 값이 우선한다.
	FPlayerCharacterTableRow()
	{
		MoveSpeed = 500.f;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	EPlayerJob Job = EPlayerJob::Knight;
};
