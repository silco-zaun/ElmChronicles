// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "../../Subsystem/AssetSubsystem.h"
#include "Components/SplineComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"
#include "AISpawner.generated.h"

class AAICharacter;
class UArrowComponent;

// 이름을 바꾸면 레벨에 저장된 Once / Loop 값을 잃는다. 바꿀 때는 DefaultEngine.ini의 [CoreRedirects]와 함께 바꾼다.
UENUM(BlueprintType)
enum class EMonsterSpawnType : uint8
{
	Once,	// 한 번만 만든다. 만든 캐릭터가 사라지면 스포너도 사라진다.
	Loop	// 만든 캐릭터가 사라지면 mSpawnDelay초 뒤 다시 만든다.
};

// 무엇을 만들지. 고른 종류에 따라 행 이름 콤보박스가 DT_Monster / DT_NPC 행으로 바뀐다.
UENUM(BlueprintType)
enum class EAISpawnKind : uint8
{
	Monster,
	NPC
};

// AI 캐릭터(몬스터, NPC) 스포너. 레벨에 놓고 Details 패널에서 종류를 고른 뒤 콤보박스에서 행을 고른다.
// 종류마다 다른 것(만들 클래스, 테이블, 캡슐 높이)은 mSpawnKind로 가르고, 언제 만들지는 종류와 상관없이 같다.
UCLASS()
class ELMCHRONICLES_API AAISpawner : public AActor
{
	GENERATED_BODY()

public:
	AAISpawner();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spawner")
	TObjectPtr<USceneComponent> mRoot;

	// 순찰 경로. 스플라인 점의 월드 위치를 만든 캐릭터에게 넘긴다.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spawner")
	TObjectPtr<USplineComponent> mPatrolPath;

#if WITH_EDITORONLY_DATA
	// 에디터 뷰포트에서 스포너 위치와 캐릭터가 바라볼 방향을 보여준다. (게임에는 포함되지 않는다)
	UPROPERTY()
	TObjectPtr<UArrowComponent> mArrow;
#endif

	UPROPERTY()
	TObjectPtr<AAICharacter> mSpawnedCharacter;

	// 바꾸면 PostEditChangeProperty가 mRowName과 mSpawnDelay를 그 종류에 맞게 다시 정한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	EAISpawnKind mSpawnKind = EAISpawnKind::Monster;

	// mSpawnKind 테이블(DT_Monster / DT_NPC)의 행 이름. 콤보박스 목록은 GetRowNameOptions가 채운다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (GetOptions = "GetRowNameOptions"))
	FName mRowName = TEXT("BoneGrunt");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	EMonsterSpawnType mSpawnType = EMonsterSpawnType::Loop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float mSpawnDelay = 30.f;

	FTimerHandle mSpawnTimerHandle;

	UPROPERTY(VisibleAnywhere)
	TArray<FVector> mPatrolPoints;

protected:
	virtual void BeginPlay() override;

public:
	virtual void OnConstruction(const FTransform& Transform) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// 만든 캐릭터가 사라지면(AAICharacter::EndPlay) 불린다.
	void ResetSpawn();

private:
	// 콤보박스(GetOptions)용. 선택한 스포너에서 불리므로 mSpawnKind를 보고 테이블을 고른다.
	UFUNCTION()	TArray<FString> GetRowNameOptions() const;

	// mSpawnKind에 맞는 클래스(AMonsterCharacter / ANPCCharacter).
	TSubclassOf<AAICharacter> GetSpawnClass() const;
	// 바닥(스포너 위치)에서 캡슐 중심까지의 높이. 행을 찾지 못하면 음수를 돌려줘서 스폰하지 않는다.
	float GetSpawnHalfHeight() const;

	// mRowName 행을 RowType 테이블에서 찾는다. 테이블이 로드되기 전이거나 행이 없으면 nullptr.
	template <typename RowType>
	const RowType* FindDataRow() const
	{
		UAssetSubsystem* AssetSubsystem = GetGameInstance()->GetSubsystem<UAssetSubsystem>();

		if (!AssetSubsystem || !AssetSubsystem->IsDataLoaded())
			return nullptr;

		return AssetSubsystem->FindRow<RowType>(mRowName);
	}

	void SpawnTimerCallback();
	void SpawnCharacter();
};
