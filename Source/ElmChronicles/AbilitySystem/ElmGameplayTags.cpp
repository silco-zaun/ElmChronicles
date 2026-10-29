#include "ElmGameplayTags.h"

namespace ElmTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Magical, "Damage.Magical", "마법 데미지 공식 사용");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Attack_Hit, "Event.Attack.Hit", "공격 몽타주의 판정 시점. 근접은 Sweep 판정, 원거리는 투사체 발사");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Attack_ComboOpen, "Event.Attack.ComboOpen", "콤보 입력 구간 시작. 이 구간에서 다시 공격하면 다음 섹션으로 이어진다");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Attack_ComboClose, "Event.Attack.ComboClose", "콤보 입력 구간 끝");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Monster_Type_Boss, "Monster.Type.Boss", "보스 몬스터");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Cooldown, "SetByCaller.Cooldown", "쿨다운 시간 SetByCaller");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Cost_MP, "SetByCaller.Cost.MP", "MP 비용 SetByCaller");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Cost_HP, "SetByCaller.Cost.HP", "HP 비용 SetByCaller");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Reward_Exp, "SetByCaller.Reward.Exp", "처치 보상 경험치 SetByCaller");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Reward_Gold, "SetByCaller.Reward.Gold", "처치 보상 골드 SetByCaller");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Recovery_HP, "SetByCaller.Recovery.HP", "HP 회복량 SetByCaller");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Recovery_MP, "SetByCaller.Recovery.MP", "MP 회복량 SetByCaller");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Dead, "State.Dead", "사망 상태. 이 태그가 있으면 어빌리티가 발동하지 않는다");
}
