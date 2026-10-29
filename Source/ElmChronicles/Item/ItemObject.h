// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "../Data/Tables/ItemTableRow.h"
#include "UObject/Object.h"
#include "ItemObject.generated.h"

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UItemObject : public UObject
{
	GENERATED_BODY()
	
public:
	UItemObject();

protected:
	// DT_Item의 행 이름 (예: WoodenSword). 세이브에 저장되는 "아이템 신분증"
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FName mItemKey;

	// 테이블 값의 런타임 사본 (UI 표시용 캐시, 저장하지 않음)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FItemTableRow mItemInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 mUpgrade = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 mCount = 1;

public:
	FName GetItemKey() const
	{
		return mItemKey;
	}

	const FItemTableRow& GetItemInfo() const
	{
		return mItemInfo;
	}

	int32 GetItemUpgrade() const
	{
		return mUpgrade;
	}

	int32 GetItemCount() const
	{
		return mCount;
	}

public:
	void SetItemInfo(FName Key, const FItemTableRow& Info)
	{
		mItemKey = Key;
		mItemInfo = Info;
	}

	void SetItemUpgrade(int32 Upgrade)
	{
		mUpgrade = Upgrade;
	}

	void AddItemUpgrade(int32 Upgrade = 1)
	{
		mUpgrade += Upgrade;
	}

	void SetItemCount(int32 Count)
	{
		mCount = Count;
	}

	void AddItemCount(int32 Count = 1)
	{
		mCount += Count;
	}
};
