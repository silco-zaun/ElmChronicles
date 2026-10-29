#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "TeamTypes.generated.h"

UENUM(BlueprintType)
enum class ETeamType : uint8
{
	None = 0 UMETA(Hidden),	// UHT 요구사항(0 항목)용. 실제 팀으로는 사용하지 않음
	Player = 10,
	Monster = 20,
	NPC = 30,
	Neutral = 255	// FGenericTeamId::NoTeamId와 같은 값 → 팀 미설정 액터는 자동으로 중립
};

// enum class는 uint8로 자동 변환되지 않으므로 변환 헬퍼를 둡니다.
FORCEINLINE FGenericTeamId MakeTeamId(ETeamType Team)
{
	return FGenericTeamId(static_cast<uint8>(Team));
}

// 프로젝트 전체의 팀 관계 규칙. 모든 팀 판정(AI 시야, 공격 판정)이 이 함수 하나를 거친다.
// 게임 모듈 시작 시 FGenericTeamId::SetAttitudeSolver로 엔진에 등록한다.
inline ETeamAttitude::Type SolveTeamAttitude(FGenericTeamId TeamA, FGenericTeamId TeamB)
{
	struct FTeamRelation
	{
		ETeamType A;
		ETeamType B;
		ETeamAttitude::Type Attitude;
	};

	// 팀 관계표. 서로 다른 두 팀의 관계를 한 줄씩 적는다. (A, B 순서는 상관없다)
	// 표에 없는 조합은 중립이다.
	static constexpr FTeamRelation Relations[] =
	{
		{ ETeamType::Player,	ETeamType::Monster,	ETeamAttitude::Hostile },
		{ ETeamType::NPC,		ETeamType::Monster,	ETeamAttitude::Hostile },	// 마을에 몬스터가 오면 NPC가 싸운다
		{ ETeamType::Player,	ETeamType::NPC,		ETeamAttitude::Neutral },	// NPC가 플레이어를 공격하게 하려면 Hostile
	};

	const ETeamType A = static_cast<ETeamType>(TeamA.GetId());
	const ETeamType B = static_cast<ETeamType>(TeamB.GetId());

	// 둘 중 하나라도 중립(팀 미설정)이면 중립
	if (A == ETeamType::Neutral || B == ETeamType::Neutral)
		return ETeamAttitude::Neutral;

	// 같은 팀끼리는 우호
	if (A == B)
		return ETeamAttitude::Friendly;

	for (const FTeamRelation& Relation : Relations)
	{
		if ((Relation.A == A && Relation.B == B) || (Relation.A == B && Relation.B == A))
			return Relation.Attitude;
	}

	return ETeamAttitude::Neutral;
}

// Source가 Target을 공격해도 되는지. 적대(Hostile)일 때만 true다. (공격 판정에서 쓴다)
inline bool IsHostile(const AActor* Source, const AActor* Target)
{
	return FGenericTeamId::GetAttitude(Source, Target) == ETeamAttitude::Hostile;
}

// 쏜 쪽의 팀만 아는 경우(투사체). Target이 그 팀에게 적대인지.
inline bool IsHostile(FGenericTeamId SourceTeam, const AActor* Target)
{
	return FGenericTeamId::GetAttitude(SourceTeam, FGenericTeamId::GetTeamIdentifier(Target)) == ETeamAttitude::Hostile;
}
