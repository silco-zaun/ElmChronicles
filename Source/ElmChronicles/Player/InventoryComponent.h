// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "../Data/Tables/ItemTableRow.h"
#include "../Save/ElmSaveGame.h"
#include "ActiveGameplayEffectHandle.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemChanged, class UItemObject*, int32);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemCountChanged, int32, int32);

// 장비칸 (Plan.md 8.1). 값이 그대로 mEquipList의 칸 번호다.
UENUM(BlueprintType)
enum class EEquipSlot : uint8
{
	Weapon,
	Armor,
	Necklace,
	Count UMETA(Hidden)	// 칸 수. 칸이 아니다.
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
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

	// 장비칸. 칸 번호 = EEquipSlot, 빈 칸은 nullptr. 장비한 아이템은 가방(mItemList)에서 빠진다.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<TObjectPtr<class UItemObject>> mEquipList;

	// 장비칸마다 걸어 둔 능력치 GE. 벗거나 바꿀 때 이 Handle로 지운다.
	TArray<FActiveGameplayEffectHandle> mEquipEffectList;

protected:
	FOnInventoryItemChanged mOnItemChanged;
	FOnInventoryItemCountChanged mOnItemCountChanged;
	// 장비칸이 바뀌면 (아이템, 장비칸 번호)로 알린다. 가방 칸과 모양이 같아서 같은 델리게이트 타입을 쓴다.
	FOnInventoryItemChanged mOnEquipChanged;

public:
	class UItemObject* GetItem(int32 Index)
	{
		return mItemList[Index];
	}

	int32 GetInventoryMaxCount() const
	{
		return mItemList.Num();
	}

	int32 GetItemCount() const
	{
		return mItemCount;
	}

	class UItemObject* GetEquipItem(EEquipSlot Slot) const
	{
		return mEquipList[static_cast<int32>(Slot)];
	}

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	virtual void InitializeComponent();

public:
	bool AddItem(FName ItemKey, int32 Count = 1);
	void SwapItem(int32 SourceIndex, int32 ChangeIndex);
	bool UseItem(int32 Index);
	// 장비칸의 장비를 벗어 가방 빈 칸에 넣는다. 가방이 가득 차 있으면 벗지 않고 false.
	bool UnequipItem(EEquipSlot Slot);

public:
	UFUNCTION()	void InitInventory();
	// 가방 / 장비칸 → 메모. 레벨 이동 / 저장 직전에 캐릭터가 부른다.
	void WriteToSave(TArray<FInventorySaveEntry>& OutInventory, TArray<FInventorySaveEntry>& OutEquipment) const;

private:
	int32 FindItemIndex(FName ItemKey) const; // 같은 키가 있는 칸
	int32 FindEmptyIndex() const; // 빈 칸
	void UsePotion(int32 Index);
	// 가방 Index 칸의 장비를 장비칸에 끼운다. 끼고 있던 장비는 그 가방 칸으로 돌아간다.
	void EquipItem(int32 Index);
	// 장비칸 하나가 바뀐 뒤에 부른다. 능력치 GE를 다시 걸고, 무기칸이면 손에 든 무기를 보이거나 숨긴다.
	void RefreshEquipSlot(EEquipSlot Slot);
	void ReadFromSave(const TArray<FInventorySaveEntry>& Entries);
	void ReadEquipFromSave(const TArray<FInventorySaveEntry>& Entries);
	// 메모 한 칸 → 아이템. 테이블에서 지워진 아이템이면 nullptr.
	class UItemObject* CreateSavedItem(const FInventorySaveEntry& Entry);
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

	template <typename T>
	void AddEquipChangeCallback(T* Obj, void (T::* Func)(class UItemObject*, int32))
	{
		mOnEquipChanged.AddUObject(Obj, Func);
	}

	void RemoveCallbacks(const UObject* Obj)
	{
		mOnItemChanged.RemoveAll(Obj);
		mOnItemCountChanged.RemoveAll(Obj);
		mOnEquipChanged.RemoveAll(Obj);
	}
};
