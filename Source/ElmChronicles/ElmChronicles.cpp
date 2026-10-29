// Copyright Epic Games, Inc. All Rights Reserved.

#include "ElmChronicles.h"
#include "Modules/ModuleManager.h"
#include "GameInfo.h"
#include "AI/TeamTypes.h"

class FElmChroniclesModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		// 엔진 기본 규칙(팀이 다르면 무조건 적대) 대신 프로젝트 규칙을 쓰도록 등록한다.
		FGenericTeamId::SetAttitudeSolver(&SolveTeamAttitude);

		UE_LOG(LogTestOrder, Log, TEXT("FElmChroniclesModule::StartupModule - AttitudeSolver 등록"));
	}

	virtual void ShutdownModule() override
	{
		// 모듈이 내려갈 때 엔진 기본 규칙으로 되돌린다.
		FGenericTeamId::ResetAttitudeSolver();
	}
};

// 첫 번째 인자가 이 모듈의 클래스다. FDefaultGameModuleImpl을 넣으면 위의 StartupModule이 불리지 않는다.
IMPLEMENT_PRIMARY_GAME_MODULE(FElmChroniclesModule, ElmChronicles, "ElmChronicles");
