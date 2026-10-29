// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "../Data/Tables/MonsterCharacterTableRow.h"
#include "AICharacter.h"
#include "MonsterCharacter.generated.h"


// 몬스터. 스포너가 만들고, 플레이어에게 처치되면 보상을 준다.
// AI / 공격 / 테이블 초기화는 부모(AICharacter)가 한다.
UCLASS()
class ELMCHRONICLES_API AMonsterCharacter : public AAICharacter
{
	GENERATED_BODY()

public:
	// AAICharacter가 만드는 AttributeSet을 UMonsterAttributeSet으로 바꿔 끼운다.
	AMonsterCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	TArray<FDropItemInfo> mDropItems;
	EMonsterType mMonsterType = EMonsterType::Normal;

public:
	EMonsterType GetMonsterType() const { return mMonsterType; }

protected:
	virtual void InitAttributes() override;
	// DT_Monster에서 찾는다. 반환 타입을 FMonsterCharacterTableRow로 좁혔으므로 등급 / 보상 항목을 바로 읽을 수 있다.
	virtual const FMonsterCharacterTableRow* FindTableRow() const override;
	virtual void OnKilledBy(AActor* Killer) override;

private:
	// 부모가 1배로 넣은 크기 / 체력 / 공격 범위에 등급 배율을 곱한다.
	void ApplyTypeScale(const FMonsterCharacterTableRow& Info);
	void GiveReward(AActor* Killer);
};
