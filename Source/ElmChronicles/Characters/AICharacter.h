// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "../Data/Tables/AICharacterTableRow.h"
#include "../Subsystem/AssetSubsystem.h"
#include "ElmCharacterBase.h"
#include "Engine/GameInstance.h"
#include "GameplayAbilitySpecHandle.h"
#include "AICharacter.generated.h"

// AI가 조종하는 전투 캐릭터(몬스터, NPC)의 공통 부모.
// DT_Monster / DT_NPC 행으로 외형 / 능력치 / 공격을 정하고, BehaviorTree로 움직인다. 팀은 자식 생성자에서 정한다.
UCLASS(Abstract)
class ELMCHRONICLES_API AAICharacter : public AElmCharacterBase
{
	GENERATED_BODY()

public:
	// 자식(몬스터, NPC)이 AttributeSet 클래스를 바꿔 끼울 수 있도록 FObjectInitializer를 받는다.
	AAICharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	// PlayerState가 없으므로 자기 자신이 ASC를 소유한다.
	UPROPERTY()	TObjectPtr<class UAIAbilitySystemComponent> mASC;
	// 기본은 UAIAttributeSet이다. 몬스터 / NPC는 생성자에서 UMonsterAttributeSet / UNPCAttributeSet으로 바꿔 끼운다.
	UPROPERTY()	TObjectPtr<class UAIAttributeSet> mAttributeSet;
	// 이 캐릭터가 사용할 BehaviorTree. 컨트롤러가 빙의할 때 가져가서 실행한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<class UBehaviorTree> mBehaviorTree;
	// 테이블 행 이름 (몬스터 = DT_Monster, NPC = DT_NPC). 스포너가 BeginPlay 전에 넣는다.
	// 레벨에 직접 배치한 캐릭터(NPC)는 Details 패널에서 정한다.
	UPROPERTY(EditAnywhere, Category = "AI") FName mRowName;
	// 이 캐릭터를 만든 스포너. 레벨에 직접 놓았으면 nullptr. 사라질 때(Destroy) 알려서 다시 만들게 한다.
	UPROPERTY()	TObjectPtr<class AAISpawner> mSpawner;

	// GiveAbility가 돌려준 번호표. 이걸로 공격을 실행/취소/확인한다.
	FGameplayAbilitySpecHandle mAttackHandle;
	TArray<FVector> mPatrolPoints;
	int32 mPatrolIndex = 0;

public:
	UBehaviorTree* GetBehaviorTree() const { return mBehaviorTree; }
	bool CanPatrol() const { return mPatrolPoints.Num() > 1; }
	FVector GetPatrolPoint() const { return mPatrolPoints[mPatrolIndex]; }
	void NextPatrol() { mPatrolIndex = (mPatrolIndex + 1) % mPatrolPoints.Num(); }
	void SetRowName(const FName& RowName) { mRowName = RowName; }
	void SetPatrolPoints(const TArray<FVector>& PatrolPoints) { mPatrolPoints = PatrolPoints; }
	void SetSpawner(class AAISpawner* Spawner) { mSpawner = Spawner; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	// 사망 애니메이션이 끝나면(DeathEnd 노티파이) 불린다.
	virtual void Death();

protected:
	// 테이블이 로드되면 한 번 불린다. 자식이 override해서 전용 데이터를 더 읽는다.
	virtual void InitAttributes();
	// 자기 테이블에서 mRowName 행을 찾는다. 자식이 override하면서 반환 타입을
	// 자기 행 구조체(FMonsterCharacterTableRow / FNPCCharacterTableRow)로 좁힐 수 있다. (공변 반환)
	virtual const FAICharacterTableRow* FindTableRow() const { return nullptr; }
	virtual void OnDamagedBy(AActor* Causer, float Damage) override;
	virtual void OnKilledBy(AActor* Killer) override;

	// mRowName으로 RowType 테이블에서 행을 찾는다. 테이블이 로드되기 전이거나 행이 없으면 nullptr.
	template <typename RowType>
	const RowType* FindDataRow() const
	{
		UAssetSubsystem* AssetSubsystem = GetGameInstance()->GetSubsystem<UAssetSubsystem>();

		if (!AssetSubsystem || !AssetSubsystem->IsDataLoaded())
			return nullptr;

		return AssetSubsystem->FindRow<RowType>(mRowName);
	}

public:
	bool PlayAttack();
	void StopAttack();
	bool IsAttacking() const;
};
