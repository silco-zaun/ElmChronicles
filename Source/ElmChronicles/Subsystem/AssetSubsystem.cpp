// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetSubsystem.h"
#include "Engine/AssetManager.h"
#include "../Data/GameDataAsset.h"

namespace
{
	// DefaultGame.ini의 PrimaryAssetTypesToScan에 등록한 타입 이름 + DA 에셋 이름.
	// 게임(비동기 로드)과 에디터(동기 로드)가 같은 DA를 보도록 한 곳에서 정한다.
	FPrimaryAssetId GetGameDataId()
	{
		return FPrimaryAssetId(TEXT("GameDataAsset"), TEXT("DA_GameData"));
	}
}

void UAssetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	UE_LOG(LogTestOrder, Log, TEXT("UAssetSubsystem::Initialize"));

	Super::Initialize(Collection);

	const FPrimaryAssetId AssetId = GetGameDataId();

	// DA + 테이블 전부 한 번에 비동기 로드
	UAssetManager::Get().LoadPrimaryAsset(AssetId, { FName(TEXT("Data")) },
		FStreamableDelegate::CreateUObject(this, &UAssetSubsystem::OnGameDataLoaded, AssetId));
}

void UAssetSubsystem::Deinitialize()
{
	mOnDataLoaded.Clear();
	Super::Deinitialize();
}

void UAssetSubsystem::OnGameDataLoaded(FPrimaryAssetId LoadId)
{
	UE_LOG(LogTestOrder, Log, TEXT("UAssetSubsystem::OnGameDataLoaded"));

	UGameDataAsset* DataAsset = UAssetManager::Get().GetPrimaryAssetObject<UGameDataAsset>(LoadId);

	if (!DataAsset)
	{
		UE_LOG(LogTestDebug, Error, TEXT("GameData 로드 실패 : %s"), *LoadId.ToString());
		return;
	}

	// 번들로 이미 로드되었으므로 Get()만 하면 된다
	for (const TSoftObjectPtr<UDataTable>& SoftTable : DataAsset->mTables)
	{
		UDataTable* Table = SoftTable.Get();

		if (!IsValid(Table) || !Table->GetRowStruct())
			continue;

		const FName Key = Table->GetRowStruct()->GetFName();

		if (mTables.Contains(Key))
			UE_LOG(LogTestDebug, Warning, TEXT("RowStruct 중복 테이블 : %s"), *Table->GetName());

		mTables.Add(Key, Table);
	}

	mIsDataLoaded = true;
	mOnDataLoaded.Broadcast();
	mOnDataLoaded.Clear();   // 1회성 이벤트이므로 정리
}

TArray<FString> UAssetSubsystem::GetRowNamesInEditor(const UScriptStruct* RowStruct)
{
	TArray<FString> Names;

	// 에디터에는 GameInstance가 없어 이 서브시스템도 없지만, AssetManager는 엔진이 켜질 때 만들어진다.
	// 경로를 코드에 적는 대신 PrimaryAssetId로 DA 위치를 묻는다.
	const FSoftObjectPath DataPath = UAssetManager::Get().GetPrimaryAssetPath(GetGameDataId());
	// 에디터 메모리에 이미 있으면 그대로 돌려주고, 없으면 디스크에서 읽는다.
	const UGameDataAsset* DataAsset = Cast<UGameDataAsset>(DataPath.TryLoad());

	if (!DataAsset)
	{
		UE_LOG(LogTestDebug, Warning, TEXT("GameData를 찾지 못했습니다 : %s"), *GetGameDataId().ToString());
		return Names;
	}

	// 게임(OnGameDataLoaded)과 같은 규칙으로, 행 구조체가 같은 테이블을 고른다.
	for (const TSoftObjectPtr<UDataTable>& SoftTable : DataAsset->mTables)
	{
		const UDataTable* Table = SoftTable.LoadSynchronous();

		if (!IsValid(Table) || Table->GetRowStruct() != RowStruct)
			continue;

		for (const FName& Name : Table->GetRowNames())
			Names.Add(Name.ToString());

		break;
	}

	return Names;
}
