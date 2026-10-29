// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "Engine/DataTable.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AssetSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UAssetSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	// Key : 행 구조체 이름 (예: LevelStatTableRow)
	UPROPERTY()
	TMap<FName, TObjectPtr<UDataTable>> mTables;

	FSimpleMulticastDelegate mOnDataLoaded;   // 엔진 기본 타입
	bool mIsDataLoaded = false;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool IsDataLoaded() const { return mIsDataLoaded; }

	// 이미 로드됐으면 즉시 호출, 아니면 로드 완료 시점에 호출
	template <typename T>
	void CallOrRegisterDataLoaded(T* Obj, void (T::* Func)())
	{
		if (mIsDataLoaded)
			(Obj->*Func)();
		else
			mOnDataLoaded.AddUObject(Obj, Func);
	}

	template <typename RowType>
	const RowType* FindRow(const FName& Name) const
	{
		const TObjectPtr<UDataTable>* Table = mTables.Find(RowType::StaticStruct()->GetFName());

		if (!Table)
		{
			UE_LOG(LogTestDebug, Error, TEXT("%s 테이블이 등록되지 않았습니다. DA_GameData의 mTables를 확인하세요."),
				*RowType::StaticStruct()->GetName());
			return nullptr;
		}

		return (*Table)->FindRow<RowType>(Name, TEXT("UAssetSubsystem::FindRow"));
	}

	// 서브시스템이 없는 에디터(Details 콤보박스의 GetOptions)에서 쓴다.
	// AssetManager에게 DA_GameData 위치를 물어 동기로 읽고, RowStruct 테이블의 행 이름을 돌려준다.
	static TArray<FString> GetRowNamesInEditor(const UScriptStruct* RowStruct);

private:
	void OnGameDataLoaded(FPrimaryAssetId LoadId);
};
