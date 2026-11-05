// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "ElmPlayerState.h"
#include "../AbilitySystem/ElmGameplayTags.h"
#include "../AbilitySystem/Effects/GameplayEffect_Equipment.h"
#include "../AbilitySystem/Effects/GameplayEffect_Recovery.h"
#include "../Characters/PlayerCharacter.h"
#include "../Item/ItemObject.h"
#include "../Subsystem/AssetSubsystem.h"
#include "../Subsystem/SaveSubsystem.h"
#include "AbilitySystemComponent.h"
#include "Engine/GameInstance.h"

namespace
{
	// 아이템 종류 → 장비칸. 장비가 아니면(포션) false.
	bool FindEquipSlot(EItemType Type, EEquipSlot& OutSlot)
	{
		switch (Type)
		{
		case EItemType::Weapon:
			OutSlot = EEquipSlot::Weapon;
			return true;
		case EItemType::Armor:
			OutSlot = EEquipSlot::Armor;
			return true;
		case EItemType::Necklace:
			OutSlot = EEquipSlot::Necklace;
			return true;
		default:
			return false;
		}
	}

	// 장비 아이템의 옵션을 모아 능력치 GE 하나로 건다. 벗을 때 지울 수 있게 Handle을 돌려준다.
	FActiveGameplayEffectHandle ApplyEquipEffect(UAbilitySystemComponent* ASC, const FItemTableRow& ItemInfo)
	{
		// UsePotion처럼 옵션들을 모아서 GE 한 번으로 적용한다.
		float PhysicalAttack = 0.f;
		float MagicalAttack = 0.f;
		float PhysicalDefense = 0.f;
		float MagicalDefense = 0.f;

		for (const FItemOption& Value : ItemInfo.Options)
		{
			switch (Value.Type)
			{
			case EItemOptionType::PhysicalAttack:
				PhysicalAttack += Value.Value;
				break;
			case EItemOptionType::MagicalAttack:
				MagicalAttack += Value.Value;
				break;
			case EItemOptionType::PhysicalDefense:
				PhysicalDefense += Value.Value;
				break;
			case EItemOptionType::MagicalDefense:
				MagicalDefense += Value.Value;
				break;
			default:
				break;
			}
		}

		FGameplayEffectSpecHandle EquipSpec = ASC->MakeOutgoingSpec(
			UGameplayEffect_Equipment::StaticClass(), 1.f, ASC->MakeEffectContext());

		if (!EquipSpec.IsValid())
			return FActiveGameplayEffectHandle();

		// GE_Equipment의 Modifier 네 개가 모두 SetByCaller이므로 넷 다 항상 채워야 한다. (목검처럼 공격력만 있어도 나머지는 0)
		EquipSpec.Data->SetSetByCallerMagnitude(ElmTags::SetByCaller_Equip_PhysicalAttack, PhysicalAttack);
		EquipSpec.Data->SetSetByCallerMagnitude(ElmTags::SetByCaller_Equip_MagicalAttack, MagicalAttack);
		EquipSpec.Data->SetSetByCallerMagnitude(ElmTags::SetByCaller_Equip_PhysicalDefense, PhysicalDefense);
		EquipSpec.Data->SetSetByCallerMagnitude(ElmTags::SetByCaller_Equip_MagicalDefense, MagicalDefense);

		return ASC->ApplyGameplayEffectSpecToSelf(*EquipSpec.Data.Get());
	}

	// 칸 목록 → 메모. 빈 칸은 적지 않고, 몇 번 칸이었는지 SlotIndex에 남긴다. (가방과 장비칸이 같이 쓴다)
	void WriteItemList(const TArray<TObjectPtr<UItemObject>>& Items, TArray<FInventorySaveEntry>& OutEntries)
	{
		OutEntries.Reset();

		for (int32 i = 0; i < Items.Num(); ++i)
		{
			const UItemObject* Item = Items[i];

			if (!Item)
				continue;

			FInventorySaveEntry& Entry = OutEntries.AddDefaulted_GetRef();
			Entry.SlotIndex = i;
			Entry.ItemKey = Item->GetItemKey();
			Entry.Count = Item->GetItemCount();
			Entry.Upgrade = Item->GetItemUpgrade();
		}
	}
}

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bWantsInitializeComponent = true;
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTestDebug, Warning, TEXT("InventoryComponent BeginPlay"));

	UAssetSubsystem* AssetSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UAssetSubsystem>();

	// 이미 로드됐으면 즉시, 아니면 로드 완료 시점에 ItemInfoLoadComplete가 호출된다.
	if (AssetSubsystem)
		AssetSubsystem->CallOrRegisterDataLoaded(this, &UInventoryComponent::InitInventory);
}

void UInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();

	mItemList.Init(nullptr, 50);

	// 장비칸은 EEquipSlot 수만큼 만들고, 칸마다 능력치 GE Handle도 하나씩 둔다.
	mEquipList.Init(nullptr, static_cast<int32>(EEquipSlot::Count));
	mEquipEffectList.SetNum(static_cast<int32>(EEquipSlot::Count));
}

bool UInventoryComponent::AddItem(FName ItemKey, int32 Count)
{
	UAssetSubsystem* AssetSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UAssetSubsystem>();

	if (!AssetSubsystem)
		return false;

	const FItemTableRow* ItemInfo = AssetSubsystem->FindRow<FItemTableRow>(ItemKey);

	if (!ItemInfo)
		return false;

	// 1) 겹치는 아이템이면 같은 키를 가진 칸에 먼저 더한다. (가방이 꽉 차도 포션은 겹쳐져야 한다)
	if (ItemInfo->UILayerType == EItemUILayerType::Layered)
	{
		int32 Index = FindItemIndex(ItemKey);

		if (Index != INDEX_NONE)
		{
			mItemList[Index]->AddItemCount(Count);

			if (mOnItemCountChanged.IsBound())
				mOnItemCountChanged.Broadcast(mItemList[Index]->GetItemCount(), Index);

			return true;
		}
	}

	// 2) 빈 칸에 새로 넣는다.
	int32 EmptyIndex = FindEmptyIndex();

	if (EmptyIndex == INDEX_NONE)
		return false;	// 가방이 가득 참

	UItemObject* Item = NewObject<UItemObject>(this);
	Item->SetItemInfo(ItemKey, *ItemInfo);
	Item->SetItemCount(Count);

	mItemList[EmptyIndex] = Item;
	++mItemCount;

	if (mOnItemChanged.IsBound())
		mOnItemChanged.Broadcast(Item, EmptyIndex);

	return true;
}

void UInventoryComponent::SwapItem(int32 SourceIndex, int32 ChangeIndex)
{
	mItemList.Swap(SourceIndex, ChangeIndex);

	UE_LOG(LogTestDebug, Warning, TEXT("SwapItem : Swap %d, %d"), SourceIndex, ChangeIndex);

	if (mOnItemChanged.IsBound())
	{
		mOnItemChanged.Broadcast(mItemList[SourceIndex], SourceIndex);
		mOnItemChanged.Broadcast(mItemList[ChangeIndex], ChangeIndex);
	}
}

bool UInventoryComponent::UseItem(int32 Index)
{
	if (Index < 0 || Index >= mItemList.Num())
		return false;
	else if (!IsValid(mItemList[Index]))
		return false;

	switch (mItemList[Index]->GetItemInfo().Type)
	{
	case EItemType::Weapon:
	case EItemType::Armor:
	case EItemType::Necklace:
		// 세 종류 모두 "맞는 장비칸에 끼운다"로 같다. 칸만 다르다.
		EquipItem(Index);
		break;
	case EItemType::Potion:
		UsePotion(Index);
		break;
	}

	return true;
}

void UInventoryComponent::InitInventory()
{
	USaveSubsystem* SaveSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<USaveSubsystem>();
	const UElmSaveGame* Save = SaveSubsystem ? SaveSubsystem->GetSaveGame() : nullptr;

	// 메모가 있으면 이전 레벨의 가방을 칸 위치까지 그대로 되살린다. 없으면 새 게임이라 시작 아이템을 준다.
	if (Save)
	{
		ReadFromSave(Save->Inventory);
		ReadEquipFromSave(Save->Equipment);
	}
	else
		GiveStartingItems();

	// 장비칸을 모두 한 번 맞춘다. 메모가 있으면 능력치 GE를 다시 걸고(이전 ASC와 함께 사라졌다),
	// 새 게임이면 빈 무기칸에 맞춰 DT_Player 무기를 숨긴다. (무기 메시 컴포넌트는 처음에 보이는 상태다)
	for (int32 i = 0; i < mEquipList.Num(); ++i)
		RefreshEquipSlot(static_cast<EEquipSlot>(i));
}

int32 UInventoryComponent::FindItemIndex(FName ItemKey) const
{
	for (int32 i = 0; i < mItemList.Num(); ++i)
	{
		if (mItemList[i] && mItemList[i]->GetItemKey() == ItemKey)
			return i;
	}

	return INDEX_NONE;
}

int32 UInventoryComponent::FindEmptyIndex() const
{
	for (int32 i = 0; i < mItemList.Num(); ++i)
	{
		if (!mItemList[i])
			return i;
	}

	return INDEX_NONE;
}

void UInventoryComponent::UsePotion(int32 Index)
{
	APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(GetOwner());

	if (!OwnerCharacter)
		return;

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();

	if (!ASC)
		return;

	// 죽은 상태에서는 포션을 쓸 수 없다. (아이템도 소모하지 않는다)
	if (ASC->HasMatchingGameplayTag(ElmTags::State_Dead))
		return;

	const FItemTableRow& ItemInfo = mItemList[Index]->GetItemInfo();

	// 옵션들을 모아서 GE 한 번으로 적용한다.
	float HPRecovery = 0.f;
	float MPRecovery = 0.f;

	for (const FItemOption& Value : ItemInfo.Options)
	{
		switch (Value.Type)
		{
		case EItemOptionType::HPRecovery:
			HPRecovery += Value.Value;
			break;
		case EItemOptionType::MPRecovery:
			MPRecovery += Value.Value;
			break;
		default:
			break;
		}
	}

	FGameplayEffectSpecHandle RecoverySpec = ASC->MakeOutgoingSpec(
		UGameplayEffect_Recovery::StaticClass(), 1.f, ASC->MakeEffectContext());

	if (RecoverySpec.IsValid())
	{
		// GE_Recovery의 Modifier 두 개가 모두 SetByCaller이므로 둘 다 항상 채워야 한다.
		RecoverySpec.Data->SetSetByCallerMagnitude(ElmTags::SetByCaller_Recovery_HP, HPRecovery);
		RecoverySpec.Data->SetSetByCallerMagnitude(ElmTags::SetByCaller_Recovery_MP, MPRecovery);

		ASC->ApplyGameplayEffectSpecToSelf(*RecoverySpec.Data.Get());
	}

	mItemList[Index]->AddItemCount(-1);
	if (mItemList[Index]->GetItemCount() == 0)
	{
		mItemList[Index] = nullptr;
		--mItemCount;

		if (mOnItemChanged.IsBound())
			mOnItemChanged.Broadcast(mItemList[Index], Index);
	}
	else
	{
		if (mOnItemCountChanged.IsBound())
			mOnItemCountChanged.Broadcast(mItemList[Index]->GetItemCount(), Index);
	}
}
void UInventoryComponent::EquipItem(int32 Index)
{
	UItemObject* NewItem = mItemList[Index];
	EEquipSlot Slot;

	if (!FindEquipSlot(NewItem->GetItemInfo().Type, Slot))
		return;

	const int32 EquipIndex = static_cast<int32>(Slot);

	// 가방 칸과 장비칸을 맞바꾼다. 끼고 있던 장비는 방금 비운 가방 칸으로 가므로 가방이 가득 차 있어도 된다.
	UItemObject* OldItem = mEquipList[EquipIndex];

	mEquipList[EquipIndex] = NewItem;
	mItemList[Index] = OldItem;

	// 끼고 있던 장비가 없었으면 가방에서 하나가 빠진 것이다.
	if (!OldItem)
		--mItemCount;

	if (mOnItemChanged.IsBound())
		mOnItemChanged.Broadcast(OldItem, Index);

	RefreshEquipSlot(Slot);
}

bool UInventoryComponent::UnequipItem(EEquipSlot Slot)
{
	const int32 EquipIndex = static_cast<int32>(Slot);
	UItemObject* Item = mEquipList[EquipIndex];

	if (!Item)
		return false;

	// 가방이 가득 차 있으면 벗지 않는다. (벗은 장비가 갈 곳이 없어 사라진다)
	const int32 EmptyIndex = FindEmptyIndex();

	if (EmptyIndex == INDEX_NONE)
		return false;

	mEquipList[EquipIndex] = nullptr;
	mItemList[EmptyIndex] = Item;
	++mItemCount;

	if (mOnItemChanged.IsBound())
		mOnItemChanged.Broadcast(Item, EmptyIndex);

	RefreshEquipSlot(Slot);
	return true;
}

void UInventoryComponent::RefreshEquipSlot(EEquipSlot Slot)
{
	const int32 EquipIndex = static_cast<int32>(Slot);
	UItemObject* Item = mEquipList[EquipIndex];
	APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(GetOwner());

	if (!OwnerCharacter)
		return;

	// 1) 능력치. 이 칸에 걸어 둔 GE를 지우고, 새 장비가 있으면 다시 건다.
	if (UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
	{
		if (mEquipEffectList[EquipIndex].IsValid())
			ASC->RemoveActiveGameplayEffect(mEquipEffectList[EquipIndex]);

		mEquipEffectList[EquipIndex] = Item ? ApplyEquipEffect(ASC, Item->GetItemInfo()) : FActiveGameplayEffectHandle();
	}
	else
		UE_LOG(LogTestDebug, Warning, TEXT("RefreshEquipSlot : ASC가 없어 장비 능력치를 걸지 못했습니다."));
	
	// 2) 겉모습. 장비는 메시를 바꾸지 않는다. 무기칸이 차 있으면 DT_Player의 무기를 보이고, 비어 있으면 숨긴다.
	if (Slot == EEquipSlot::Weapon)
		OwnerCharacter->SetWeaponVisibility(Item != nullptr);

	// 3) 캐릭터 창(2단계)이 장비칸을 다시 그리도록 알린다.
	if (mOnEquipChanged.IsBound())
		mOnEquipChanged.Broadcast(Item, EquipIndex);
}

// 가방 / 장비칸 → 메모
void UInventoryComponent::WriteToSave(TArray<FInventorySaveEntry>& OutInventory, TArray<FInventorySaveEntry>& OutEquipment) const
{
	WriteItemList(mItemList, OutInventory);
	WriteItemList(mEquipList, OutEquipment);
}

// 메모 → 가방 (데이터 테이블 로드가 끝난 뒤에만 호출해야 한다)
void UInventoryComponent::ReadFromSave(const TArray<FInventorySaveEntry>& Entries)
{
	for (int32 i = 0; i < mItemList.Num(); ++i)
		mItemList[i] = nullptr;

	mItemCount = 0;

	for (const FInventorySaveEntry& Entry : Entries)
	{
		if (!mItemList.IsValidIndex(Entry.SlotIndex))
			continue;

		mItemList[Entry.SlotIndex] = CreateSavedItem(Entry);

		if (mItemList[Entry.SlotIndex])
			++mItemCount;
	}

	// 이미 열려 있는 인벤토리 위젯이 있으면 갱신되도록 전 칸을 알린다
	if (mOnItemChanged.IsBound())
	{
		for (int32 i = 0; i < mItemList.Num(); ++i)
			mOnItemChanged.Broadcast(mItemList[i], i);
	}
}

// 메모 → 장비칸 (ReadFromSave처럼 데이터 로드 뒤에만). 능력치와 무기 표시는 InitInventory가 이어서 맞춘다.
void UInventoryComponent::ReadEquipFromSave(const TArray<FInventorySaveEntry>& Entries)
{
	for (const FInventorySaveEntry& Entry : Entries)
	{
		// 장비칸 수가 줄었을 때 대비
		if (mEquipList.IsValidIndex(Entry.SlotIndex))
			mEquipList[Entry.SlotIndex] = CreateSavedItem(Entry);
	}
}

// 메모 한 칸 → 아이템
UItemObject* UInventoryComponent::CreateSavedItem(const FInventorySaveEntry& Entry)
{
	UAssetSubsystem* AssetSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UAssetSubsystem>();
	const FItemTableRow* ItemInfo = AssetSubsystem ? AssetSubsystem->FindRow<FItemTableRow>(Entry.ItemKey) : nullptr;

	// 패치로 테이블에서 삭제된 아이템이면 건너뛴다
	if (!ItemInfo)
	{
		UE_LOG(LogTestDebug, Warning, TEXT("세이브의 아이템 %s 이(가) DT_Item에 없어 건너뜁니다."), *Entry.ItemKey.ToString());
		return nullptr;
	}

	UItemObject* Item = NewObject<UItemObject>(this);
	Item->SetItemInfo(Entry.ItemKey, *ItemInfo);
	Item->SetItemCount(Entry.Count);
	Item->SetItemUpgrade(Entry.Upgrade);

	return Item;
}

void UInventoryComponent::GiveStartingItems()
{
	// 새 게임 시작 아이템 (임시). 나중에는 DT_Player 등 데이터로 옮긴다.
	TArray<FName> ItemKeys;

	ItemKeys.Add(TEXT("WoodenSword"));
	ItemKeys.Add(TEXT("ClothArmor"));
	ItemKeys.Add(TEXT("HPPotion"));
	
	for (int32 i = 0; i < ItemKeys.Num(); ++i)
	{
		//AddItem(ItemKeys[FMath::RandRange(0, ItemKeys.Num() - 1)]);
		AddItem(ItemKeys[i]);
	};
}
