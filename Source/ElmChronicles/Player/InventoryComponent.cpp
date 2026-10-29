// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "ElmPlayerState.h"
#include "../AbilitySystem/ElmGameplayTags.h"
#include "../AbilitySystem/Effects/GameplayEffect_Recovery.h"
#include "../Characters/PlayerCharacter.h"
#include "../Item/ItemObject.h"
#include "../Subsystem/AssetSubsystem.h"
#include "../Subsystem/SaveSubsystem.h"
#include "AbilitySystemComponent.h"
#include "Engine/GameInstance.h"

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

void UInventoryComponent::SwapItem(int32 StartIndex, int32 EndIndex)
{
	mItemList.Swap(StartIndex, EndIndex);

	UE_LOG(LogTestDebug, Warning, TEXT("SwapItem : Swap %d, %d"), StartIndex, EndIndex);

	if (mOnItemChanged.IsBound())
	{
		mOnItemChanged.Broadcast(mItemList[StartIndex], StartIndex);
		mOnItemChanged.Broadcast(mItemList[EndIndex], EndIndex);
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
		UseWeapon(Index);
		break;
	case EItemType::Armor:
		UseArmor(Index);
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
		ReadFromSave(Save->Inventory);
	else
		GiveStartingItems();
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

void UInventoryComponent::UseWeapon(int32 Index)
{
}

void UInventoryComponent::UseArmor(int32 Index)
{
}
// 가방 → 메모
void UInventoryComponent::WriteToSave(TArray<FInventorySaveEntry>& OutEntries) const
{
	OutEntries.Reset();

	for (int32 i = 0; i < mItemList.Num(); ++i)
	{
		const UItemObject* Item = mItemList[i];

		if (!Item)
			continue;

		FInventorySaveEntry& Entry = OutEntries.AddDefaulted_GetRef();
		Entry.SlotIndex = i;
		Entry.ItemKey = Item->GetItemKey();
		Entry.Count = Item->GetItemCount();
		Entry.Upgrade = Item->GetItemUpgrade();
	}
}

// 메모 → 가방 (데이터 테이블 로드가 끝난 뒤에만 호출해야 한다)
void UInventoryComponent::ReadFromSave(const TArray<FInventorySaveEntry>& Entries)
{
	UAssetSubsystem* AssetSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UAssetSubsystem>();

	if (!AssetSubsystem)
		return;

	for (int32 i = 0; i < mItemList.Num(); ++i)
		mItemList[i] = nullptr;

	mItemCount = 0;

	for (const FInventorySaveEntry& Entry : Entries)
	{
		// 인벤토리 칸 수가 줄었을 때 대비
		if (!mItemList.IsValidIndex(Entry.SlotIndex))
			continue;

		const FItemTableRow* ItemInfo = AssetSubsystem->FindRow<FItemTableRow>(Entry.ItemKey);

		// 패치로 테이블에서 삭제된 아이템이면 건너뛴다
		if (!ItemInfo)
		{
			UE_LOG(LogTestDebug, Warning, TEXT("세이브의 아이템 %s 이(가) DT_Item에 없어 건너뜁니다."), *Entry.ItemKey.ToString());
			continue;
		}

		UItemObject* Item = NewObject<UItemObject>(this);
		Item->SetItemInfo(Entry.ItemKey, *ItemInfo);
		Item->SetItemCount(Entry.Count);
		Item->SetItemUpgrade(Entry.Upgrade);

		mItemList[Entry.SlotIndex] = Item;
		++mItemCount;
	}

	// 이미 열려 있는 인벤토리 위젯이 있으면 갱신되도록 전 칸을 알린다
	if (mOnItemChanged.IsBound())
	{
		for (int32 i = 0; i < mItemList.Num(); ++i)
			mOnItemChanged.Broadcast(mItemList[i], i);
	}
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
