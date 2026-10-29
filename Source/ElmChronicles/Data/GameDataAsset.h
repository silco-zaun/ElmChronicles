// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "Engine/DataAsset.h"
#include "GameDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UGameDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// TSoftObjectPtr : 에셋이나 오브젝트를 경로만 가지고 있다가 필요할때 로드하는 포인터.
	// 행 구조체(RowStruct) 기준으로 자동 등록된다. 테이블을 추가해도 코드 수정이 필요 없다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameData", meta = (AssetBundles = "Data"))
	TArray<TSoftObjectPtr<class UDataTable>> mTables;
};
