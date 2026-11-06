#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MovementTableRow.generated.h"

// DT_Movement 행. 이동 감각(가속, 회전, 점프, 공중 제어)을 CharacterMovementComponent에 그대로 넣는 값이다.
// 어떤 캐릭터를 골랐는지(DT_Player)와 상관없는 조작감 설정이라 따로 둔다. 플레이어는 "Player" 행을 쓴다.
// 최고 속도는 GE(슬로우 등)로 바뀌는 능력치라 여기가 아니라 MoveSpeed Attribute가 정한다.
// 기본값은 StackOBot(BP_Bot) 값을 지금 플레이어 크기(키 약 250)에 맞춰 옮긴 것이다. 괄호 안은 엔진 기본값.
USTRUCT(BlueprintType)
struct FMovementTableRow : public FTableRowBase
{
	GENERATED_BODY()

	// 가속도(cm/s²). 4000이면 0.125초 만에 최고 속도(500)에 닿는다. (2048 → 0.24초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground")
	float MaxAcceleration = 4000.f;

	// 몸이 이동 방향으로 도는 속도(도/초, Yaw). 720이면 뒤돌기에 0.25초. (360 → 0.5초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground")
	float RotationRate = 720.f;

	// 중력 배율. 2면 같은 높이를 더 빨리 오르내려 둥실거리지 않는다. (1)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	float GravityScale = 2.f;

	// 점프 순간의 위쪽 속도(cm/s). 키를 톡 쳤을 때의 낮은 점프 높이를 정한다. (420)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	float JumpZVelocity = 400.f;

	// 점프 키를 누르고 있는 동안 상승 속도를 유지하는 최대 시간(초). (0 = 높이가 늘 같다)
	// 톡 치는 입력도 0.05 ~ 0.12초는 눌려 있으므로 그보다 충분히 길어야 짧게 / 길게가 갈린다.
	// 400 / 0.3 / 중력 2배: 0.08초 약 0.65 m, 0.2초 약 1.2 m, 0.3초 이상 약 1.5 m.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	float JumpMaxHoldTime = 0.3f;

	// 공중에서 방향을 바꾸는 힘(0 ~ 1). (0.05 = 뛰는 순간 방향이 거의 고정된다)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air")
	float AirControl = 1.f;

	// 수평 속도가 거의 없을 때(제자리 점프) AirControl에 곱하는 배율. 공중에서 출발을 돕는다. (2)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air")
	float AirControlBoostMultiplier = 5.f;

	// 공중에서 입력이 없을 때 수평 속도가 줄어드는 정도(cm/s²). 1000이면 0.5초 만에 멈춘다. (0 = 계속 미끄러져 간다)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air")
	float BrakingDecelerationFalling = 1000.f;
};
