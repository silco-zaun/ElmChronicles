// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterInfoWidget.h"
#include "CharacterPreviewActor.h"
#include "SlotWidget.h"
#include "../../AbilitySystem/Attributes/PlayerAttributeSet.h"
#include "../../Characters/ElmCharacterBase.h"
#include "../../Data/Tables/LevelStatTableRow.h"
#include "../../Player/ElmPlayerController.h"
#include "../../Player/InventoryComponent.h"
#include "../../Subsystem/AssetSubsystem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"

namespace
{
	// 캐릭터 창이 지켜보는 Attribute. 하나라도 바뀌면 두 탭의 글자를 다시 만든다.
	TArray<FGameplayAttribute> GetWatchedAttributes()
	{
		return {
			UBaseAttributeSet::GetLevelAttribute(),
			UBaseAttributeSet::GetPhysicalAttackAttribute(), UBaseAttributeSet::GetMagicalAttackAttribute(),
			UBaseAttributeSet::GetPhysicalDefenseAttribute(), UBaseAttributeSet::GetMagicalDefenseAttribute(),
			UBaseAttributeSet::GetHPAttribute(), UBaseAttributeSet::GetMaxHPAttribute(),
			UBaseAttributeSet::GetMPAttribute(), UBaseAttributeSet::GetMaxMPAttribute(),
			UBaseAttributeSet::GetAttackSpeedAttribute(), UBaseAttributeSet::GetMoveSpeedAttribute(),
			UBaseAttributeSet::GetAttackRangeAttribute(),
			UPlayerAttributeSet::GetExpAttribute(), UPlayerAttributeSet::GetGoldAttribute() };
	}

	// Attribute는 float이라 화면에는 반올림한 정수로 보여 준다.
	int32 GetIntStat(const UAbilitySystemComponent* ASC, const FGameplayAttribute& Attribute)
	{
		return FMath::RoundToInt(ASC->GetNumericAttribute(Attribute));
	}

	// "30 (+10)" : 지금 값과 Base 값의 차이 = 장비 GE가 얹은 양. 얹은 것이 없으면 숫자만 보여 준다.
	FText MakeEquipStatText(const UAbilitySystemComponent* ASC, const FGameplayAttribute& Attribute)
	{
		const int32 Current = GetIntStat(ASC, Attribute);
		const int32 Bonus = Current - FMath::RoundToInt(ASC->GetNumericAttributeBase(Attribute));

		if (Bonus == 0)
			return FText::AsNumber(Current);

		return FText::Format(NSLOCTEXT("ElmCharacterInfo", "StatWithBonus", "{0} (+{1})"), Current, Bonus);
	}

	// 미리보기 캐릭터를 세울 곳. 메인 카메라에는 그려지지 않지만, 맵의 물체나 빛과 섞이지 않게 맵 위 높은 곳에 둔다.
	const FVector PreviewLocation(0.0, 0.0, 100000.0);
}

UCharacterInfoWidget::UCharacterInfoWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UCharacterInfoWidget::SetPreviewActive(bool Active)
{
	if (ACharacterPreviewActor* Preview = mPreviewActor.Get())
	{
		// 열 때마다 지금 플레이어의 겉모습을 다시 복사한다.
		// (위젯이 생기는 시점에는 데이터 로드 전이라 DT 외형이 아직 적용되지 않았을 수 있다)
		if (Active)
			Preview->CopyAppearance(Cast<AElmCharacterBase>(GetOwningPlayerPawn()));

		Preview->SetCaptureActive(Active);
	}
}

void UCharacterInfoWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 위젯이 처음 만들어질 때 한 번만 연결한다. (NativeConstruct는 화면에 붙을 때마다 불려서 두 번 연결될 수 있다)
	mCloseButton->OnClicked.AddDynamic(this, &UCharacterInfoWidget::OnCloseClicked);
	mEquipTabButton->OnClicked.AddDynamic(this, &UCharacterInfoWidget::OnEquipTabClicked);
	mStatTabButton->OnClicked.AddDynamic(this, &UCharacterInfoWidget::OnStatTabClicked);

	// 배열 순서 = EEquipSlot 순서. 슬롯 번호가 곧 장비칸 번호라서 아이콘이 GetEquipItem / UnequipItem에 그대로 넘긴다.
	mEquipSlotArray = { mWeaponSlot, mArmorSlot, mNecklaceSlot };

	for (int32 i = 0; i < mEquipSlotArray.Num(); ++i)
	{
		mEquipSlotArray[i]->SetSlotIndex(i);
		mEquipSlotArray[i]->SetSlotType(ESlotType::Equipment);
	}

	SelectTab(0);
}

void UCharacterInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 장비칸 : 인벤토리 창처럼 Pawn의 인벤토리 컴포넌트를 구독한다.
	APawn* Pawn = GetOwningPlayerPawn();
	UInventoryComponent* Inventory = Pawn ? Pawn->FindComponentByClass<UInventoryComponent>() : nullptr;

	if (Inventory)
	{
		mInventoryComponent = Inventory;

		for (int32 i = 0; i < mEquipSlotArray.Num(); ++i)
		{
			mEquipSlotArray[i]->SetInventoryComponent(Inventory);
			// 구독하기 전에 이미 끼고 있던 장비(레벨 이동 뒤 메모에서 되살린 장비)로 한 번 그린다.
			mEquipSlotArray[i]->SetItem(Inventory->GetEquipItem(static_cast<EEquipSlot>(i)));
		}

		Inventory->AddEquipChangeCallback(this, &UCharacterInfoWidget::EquipChange);
	}

	// 능력치 : 상태바처럼 PlayerState의 ASC를 구독한다.
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwningPlayerState());

	if (ASC)
	{
		mASC = ASC;

		for (const FGameplayAttribute& Attribute : GetWatchedAttributes())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(Attribute)
				.AddUObject(this, &UCharacterInfoWidget::OnAttributeChanged);
		}

		// 구독 이전의 현재 값으로 한 번 그린다.
		RefreshStats();

		// 가운데 캐릭터 미리보기. 하나만 세워 두고, 창이 열려 있을 때만 찍는다(SetPreviewActive).
		// 레벨을 이동하면 월드와 함께 사라지고, 새 맵의 HUD가 만들어질 때 다시 생긴다.
		if (!mPreviewActor.IsValid())
		{
			if (UWorld* World = GetWorld())
				mPreviewActor = World->SpawnActor<ACharacterPreviewActor>(PreviewLocation, FRotator::ZeroRotator);
		}
	}
}

void UCharacterInfoWidget::NativeDestruct()
{
	if (UInventoryComponent* Inventory = mInventoryComponent.Get())
		Inventory->RemoveCallbacks(this);

	if (UAbilitySystemComponent* ASC = mASC.Get())
	{
		for (const FGameplayAttribute& Attribute : GetWatchedAttributes())
			ASC->GetGameplayAttributeValueChangeDelegate(Attribute).RemoveAll(this);
	}

	// 레벨 이동이면 월드와 함께 이미 사라졌을 수 있어서 살아 있을 때만 지운다.
	if (ACharacterPreviewActor* Preview = mPreviewActor.Get())
		Preview->Destroy();

	mInventoryComponent.Reset();
	mASC.Reset();
	mPreviewActor.Reset();

	Super::NativeDestruct();
}

void UCharacterInfoWidget::OnCloseClicked()
{
	// 닫으면서 커서와 입력 모드도 되돌려야 하므로 C 키와 같은 길(컨트롤러)로 닫는다.
	if (AElmPlayerController* PlayerController = GetOwningPlayer<AElmPlayerController>())
		PlayerController->SetCharacterInfoOpen(false);
}

void UCharacterInfoWidget::OnEquipTabClicked()
{
	SelectTab(0);
}

void UCharacterInfoWidget::OnStatTabClicked()
{
	SelectTab(1);
}

void UCharacterInfoWidget::SelectTab(int32 TabIndex)
{
	mTabSwitcher->SetActiveWidgetIndex(TabIndex);

	// 지금 탭의 버튼은 끈다. 꺼진 버튼은 흐리게 그려져서 "선택됨" 표시가 되고, 같은 탭을 다시 누를 일도 없다.
	mEquipTabButton->SetIsEnabled(TabIndex != 0);
	mStatTabButton->SetIsEnabled(TabIndex != 1);
}

void UCharacterInfoWidget::EquipChange(UItemObject* Item, int32 Index)
{
	if (mEquipSlotArray.IsValidIndex(Index))
		mEquipSlotArray[Index]->SetItem(Item);

	// 무기를 끼고 벗으면 미리보기 손의 무기도 맞춘다.
	// (RefreshEquipSlot이 플레이어 무기 표시를 먼저 바꾼 뒤 알리므로 바뀐 상태가 복사된다)
	if (ACharacterPreviewActor* Preview = mPreviewActor.Get())
		Preview->CopyAppearance(Cast<AElmCharacterBase>(GetOwningPlayerPawn()));
}

void UCharacterInfoWidget::OnAttributeChanged(const FOnAttributeChangeData& Data)
{
	RefreshStats();
}

void UCharacterInfoWidget::RefreshStats()
{
	const UAbilitySystemComponent* ASC = mASC.Get();

	if (!ASC)
		return;

	// 장비 탭 : 장비가 바꾸는 네 개. 장비 몫은 괄호로 따로 보여 준다.
	mEquipStatText->SetText(FText::Format(
		NSLOCTEXT("ElmCharacterInfo", "EquipStats", "공격력 {0}\n마법 공격력 {1}\n방어력 {2}\n마법 방어력 {3}"),
		MakeEquipStatText(ASC, UBaseAttributeSet::GetPhysicalAttackAttribute()),
		MakeEquipStatText(ASC, UBaseAttributeSet::GetMagicalAttackAttribute()),
		MakeEquipStatText(ASC, UBaseAttributeSet::GetPhysicalDefenseAttribute()),
		MakeEquipStatText(ASC, UBaseAttributeSet::GetMagicalDefenseAttribute())));

	// 경험치 : 지금 레벨 행(DT_LevelStat)의 RequiredExp가 다음 레벨까지 필요한 양이다. 0이면 최대 레벨.
	// 데이터 로드나 InitAttributes 전(Level 0)에는 행을 찾지 않는다. 찾으면 "행이 없다" 로그가 남는다.
	const int32 Level = GetIntStat(ASC, UBaseAttributeSet::GetLevelAttribute());
	const int32 Exp = GetIntStat(ASC, UPlayerAttributeSet::GetExpAttribute());
	FText ExpText = FText::AsNumber(Exp);

	const UGameInstance* GameInstance = GetGameInstance();
	const UAssetSubsystem* AssetSubsystem = GameInstance ? GameInstance->GetSubsystem<UAssetSubsystem>() : nullptr;

	if (Level > 0 && AssetSubsystem && AssetSubsystem->IsDataLoaded())
	{
		if (const FLevelStatTableRow* LevelStat = AssetSubsystem->FindRow<FLevelStatTableRow>(FName(*FString::FromInt(Level))))
		{
			ExpText = LevelStat->RequiredExp > 0 ?
				FText::Format(NSLOCTEXT("ElmCharacterInfo", "Exp", "{0} / {1}"), Exp, LevelStat->RequiredExp) :
				NSLOCTEXT("ElmCharacterInfo", "MaxLevel", "최대 레벨");
		}
	}

	// 능력치 탭
	mDetailStatText->SetText(FText::Format(
		NSLOCTEXT("ElmCharacterInfo", "DetailStats",
			"레벨 {0}\n경험치 {1}\n골드 {2}\n\nHP {3} / {4}\nMP {5} / {6}\n\n공격 속도 {7}\n이동 속도 {8}\n사거리 {9}"),
		Level, ExpText, GetIntStat(ASC, UPlayerAttributeSet::GetGoldAttribute()),
		GetIntStat(ASC, UBaseAttributeSet::GetHPAttribute()), GetIntStat(ASC, UBaseAttributeSet::GetMaxHPAttribute()),
		GetIntStat(ASC, UBaseAttributeSet::GetMPAttribute()), GetIntStat(ASC, UBaseAttributeSet::GetMaxMPAttribute()),
		FText::AsPercent(ASC->GetNumericAttribute(UBaseAttributeSet::GetAttackSpeedAttribute())),
		GetIntStat(ASC, UBaseAttributeSet::GetMoveSpeedAttribute()),
		GetIntStat(ASC, UBaseAttributeSet::GetAttackRangeAttribute())));
}
