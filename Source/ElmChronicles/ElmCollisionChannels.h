#pragma once

#include "Engine/EngineTypes.h"
#include "CollisionQueryParams.h"

// 프로젝트 충돌 채널 모음. 캐릭터는 편(팀)에 상관없이 모두 엔진 기본 Pawn 오브젝트 타입을 쓴다.
// 아군 / 적 구분은 충돌이 아니라 팀(TeamTypes.h)이 한다.
// 커스텀 채널을 추가하면 DefaultEngine.ini의 순서와 맞춰서 여기에 적는다.
namespace ElmChannels
{
	// 상호작용 대상(NPC 등)을 찾는 트레이스 채널. Project Settings > Collision의 "Interaction".
	// 기본 응답이 Ignore라서 응답을 Overlap으로 바꾼 컴포넌트(NPC 캡슐)만 걸린다.
	inline constexpr ECollisionChannel Interaction = ECC_GameTraceChannel1;

	// 공격 판정이 찾는 "몸"의 종류. 누구를 실제로 때릴지는 팀(IsHostile)이 정한다.
	inline FCollisionObjectQueryParams MakeCharacterObjectQuery()
	{
		FCollisionObjectQueryParams Params;
		Params.AddObjectTypesToQuery(ECC_Pawn);
		return Params;
	}
}
