// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "../Data/Tables/ItemTableRow.h"
#include "../Save/ElmSaveGame.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemChanged, class UItemObject*, int32);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemCountChanged, int32, int32);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ELMCHRONICLES_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<TObjectPtr<class UItemObject>> mItemList;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 mItemCount = 0;

protected:
	FOnInventoryItemChanged mOnItemChanged;
	FOnInventoryItemCountChanged mOnItemCountChanged;

public:
	int32 GetItemCount() const
	{
		return mItemCount;
	}

	int32 GetInventoryMaxCount() const
	{
		return mItemList.Num();
	}

	class UItemObject* GetItem(int32 Index)
	{
		return mItemList[Index];
	}

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	virtual void InitializeComponent();

public:
	bool AddItem(FName ItemKey, int32 Count = 1);
	void SwapItem(int32 StartIndex, int32 EndIndex);
	bool UseItem(int32 Index);

public:
	UFUNCTION()	void InitInventory();
	// 가방 → 메모. 레벨 이동 / 저장 직전에 캐릭터가 부른다.
	void WriteToSave(TArray<FInventorySaveEntry>& OutEntries) const;

private:
	int32 FindItemIndex(FName ItemKey) const; // 같은 키가 있는 칸
	int32 FindEmptyIndex() const; // 빈 칸
	void UsePotion(int32 Index);
	void UseWeapon(int32 Index);
	void UseArmor(int32 Index);
	void ReadFromSave(const TArray<FInventorySaveEntry>& Entries);
	void GiveStartingItems();

public:
	template <typename T>
	void AddItemChangeCallback(T* Obj, void (T::* Func)(class UItemObject*, int32))
	{
		mOnItemChanged.AddUObject(Obj, Func);
	}

	template <typename T>
	void AddItemCountChangeCallback(T* Obj, void (T::* Func)(int32, int32))
	{
		mOnItemCountChanged.AddUObject(Obj, Func);
	}

	void RemoveCallbacks(const UObject* Obj)
	{
		mOnItemChanged.RemoveAll(Obj);
		mOnItemCountChanged.RemoveAll(Obj);
	}
};
