// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "../AI/TeamTypes.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "ElmCharacterBase.generated.h"

struct FCharacterTableRow;
struct FOnAttributeChangeData;
class UAnimMontage;
class UBaseAttributeSet;
class UStaticMeshComponent;

// 플레이어와 AI 캐릭터(몬스터, NPC)의 공통 부모.
// ASC를 쓰는 캐릭터의 공통 처리(테이블 외형 적용, 이동속도 반영, 피격/사망 알림, 장비 메시, 팀)를 모아 둔다.
UCLASS(Abstract)
class ELMCHRONICLES_API AElmCharacterBase : public ACharacter,
	public IAbilitySystemInterface,
	public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// 자식이 서브오브젝트 클래스를 바꿔 끼울 수 있도록 FObjectInitializer를 받아 ACharacter까지 넘긴다.
	// 기본값이 있어서 인자 없이 Super()를 부르는 자식(플레이어)은 그대로 된다.
	AElmCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	// 무기 메시. 보이기만 하고, 공격 판정은 어빌리티의 Sweep으로 처리한다.
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> mMainhand; // handslot_r
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> mOffhand; // handslot_l
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> mHeadGear; // head
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> mBackGear; // chest
	// 테이블에서 읽은 기본 공격 몽타주. 공격 어빌리티가 가져가서 재생한다.
	UPROPERTY()	TObjectPtr<UAnimMontage> mAttackMontage;
	// 소속 팀. 자식 생성자에서 정한다. 레벨에 배치한 캐릭터는 Details 패널에서 바꿀 수 있다.
	// AI 시야는 시작할 때 팀을 기억하므로 플레이 전에 정해 둔다.
	UPROPERTY(EditAnywhere, Category = "Team") ETeamType mTeam = ETeamType::Neutral;

public:
	// ASC 소유자는 자식마다 다르다. (플레이어 = PlayerState, 몬스터 = 자기 자신) 자식이 override한다.
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return nullptr; }
	// 팀 판정(AI 시야 / 공격 판정)이 이 값을 읽는다.
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	// State.Dead 태그가 있으면 죽은 것으로 본다.
	bool IsDead() const;
	UAnimMontage* GetAttackMontage() const { return mAttackMontage; }

protected:
	// 테이블 행의 외형(캡슐, 메시, ABP, 장비)과 기본 공격 몽타주를 적용한다.
	// 눈에 보이는 부분이므로 서버와 클라이언트 모두에서 자식이 호출한다.
	void InitAsset(const FCharacterTableRow& Info);
	// 캐릭터 원점 기준으로 모델링된 장비를 BoneName에 붙인다. 바디 메시가 정해진 뒤에 호출한다.
	void AttachBindPoseGear(UStaticMeshComponent* Gear, FName BoneName);
	// ASC와 AttributeSet이 준비된 뒤 자식이 호출한다. 이동속도 / 피격 / 사망 델리게이트를 연결한다.
	void BindAttributeDelegates(UAbilitySystemComponent* ASC, UBaseAttributeSet* AttributeSet);
	// 맞고 살아남았을 때. 자식이 필요하면 override한다.
	virtual void OnDamagedBy(AActor* Causer, float Damage) {}
	// 죽은 순간. State.Dead 태그와 어빌리티 취소가 끝난 뒤에 불린다.
	virtual void OnKilledBy(AActor* Killer) {}

private:
	void HandleDamaged(AActor* Causer, float Damage);
	void HandleDeath(AActor* Killer);
	void OnMoveSpeedAttributeChanged(const FOnAttributeChangeData& Data);
};
