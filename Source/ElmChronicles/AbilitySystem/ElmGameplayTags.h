#pragma once

#include "NativeGameplayTags.h"

// 프로젝트의 GameplayTag 모음. 문자열 대신 이 변수를 쓰면 오타가 컴파일 에러로 잡힌다.
namespace ElmTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Magical);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Attack_Hit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Attack_ComboOpen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Attack_ComboClose);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Monster_Type_Boss);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Cooldown);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Cost_MP);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Cost_HP);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Reward_Exp);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Reward_Gold);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Recovery_HP);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Recovery_MP);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);
}
