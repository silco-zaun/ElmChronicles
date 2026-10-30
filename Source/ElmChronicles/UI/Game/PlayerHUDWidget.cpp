// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"
#include "DialogueWidget.h"
#include "InventoryWidget.h"
#include "ItemInfoWidget.h"
#include "MenuWidget.h"
#include "StatusBarWidget.h"
#include "../../Input/DefaultInput.h"
#include "../../Item/ItemObject.h"

UPlayerHUDWidget::UPlayerHUDWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UPlayerHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 처음에는 상호작용 대상도, 대화도 없으므로 숨긴다.
	SetInteractPrompt(FText::GetEmpty());
	SetDialogueVisible(false);
	SetMenuVisible(false);
}

void UPlayerHUDWidget::SetPlayerName(const FString& Name)
{
	mStatusBarWidget->SetName(Name);
}

void UPlayerHUDWidget::SetItemInfo(UItemObject* Item)
{
	mItemInfoWidget->SetItemInfo(Item->GetItemInfo(), Item->GetItemUpgrade());
}

void UPlayerHUDWidget::EnableItemInfoWidget(bool Enable)
{
	if (Enable)
		mItemInfoWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	else
		mItemInfoWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UPlayerHUDWidget::ComputeItemInfoLocation(const FPointerEvent& InMouseEvent)
{
	// ItemInfoWidget의 CanvasPanelSlot을 구한다.
	UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(mItemInfoWidget->Slot);

	// ScreenSpace에서의 마우스 위치를 구한다.
	FVector2D MousePos = InMouseEvent.GetScreenSpacePosition();

	// ItemInfoWidget의 부모 Panel을 구한다.
	UPanelWidget* ParentPanel = mItemInfoWidget->GetParent();

	// 마우스 위치를 이 위젯 내부에서의 위치로 구한다.
	FVector2D WidgetPos = ParentPanel->GetCachedGeometry().AbsoluteToLocal(MousePos);

	// 정확하게 마우스의 위치로 나오기 때문에 이 위치에서 오른쪽으로
	// 약간 떨어지게 좌표를 지정한다.
	WidgetPos.X += 20.0;

	PanelSlot->SetPosition(WidgetPos);
}

void UPlayerHUDWidget::SetInventoryVisible(bool Visible)
{
	mInventoryWidget->SetVisibility(Visible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	// 아이콘 위에 마우스를 올린 채로 창을 닫으면 MouseLeave가 오지 않는다.
	// 그래서 정보창도 같이 닫는다.
	if (!Visible)
		EnableItemInfoWidget(false);
}

bool UPlayerHUDWidget::IsInventoryVisible() const
{
	// UWidget::IsVisible()은 Slate 위젯이 만들어진 뒤에만 정확하므로
	// 우리가 직접 설정한 값과 비교한다.
	return mInventoryWidget->GetVisibility() == ESlateVisibility::Visible;
}

void UPlayerHUDWidget::SetMenuVisible(bool Visible)
{
	if (Visible)
	{
		// 열 때마다 지금 상황(전투 중, 사망)에 맞춰 버튼과 문구를 다시 맞춘다.
		mMenuWidget->Refresh();

		// 아이콘 위에 마우스를 올린 채로 메뉴를 열면 MouseLeave가 오지 않아 정보창이 남는다.
		EnableItemInfoWidget(false);
	}

	mMenuWidget->SetVisibility(Visible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

bool UPlayerHUDWidget::IsMenuVisible() const
{
	// IsInventoryVisible과 같은 이유로 직접 설정한 값과 비교한다.
	return mMenuWidget->GetVisibility() == ESlateVisibility::Visible;
}

bool UPlayerHUDWidget::IsAnyWindowOpen() const
{
	// 나중에 스킬창, 퀘스트창이 생기면 여기에 || 로 추가한다.
	return IsInventoryVisible() || IsMenuVisible();
}

void UPlayerHUDWidget::SetInteractPrompt(const FText& ActionText)
{
	if (ActionText.IsEmpty())
	{
		mInteractText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// 키 글자를 코드에 적지 않고, IA_Interact에 지금 매핑된 키를 물어본다. IMC에서 키를 바꿔도 문구가 따라간다.
	const FText KeyText = GetDefault<UDefaultInput>()->FindMappedKeyText(GetOwningLocalPlayer(), TEXT("Interact"));

	// 시작 직후처럼 키를 아직 모를 때는 동작 이름만 띄운다. (0.1초마다 다시 불리므로 곧 키가 붙는다)
	if (KeyText.IsEmpty())
		mInteractText->SetText(ActionText);
	else
		mInteractText->SetText(FText::Format(NSLOCTEXT("ElmHUD", "InteractPrompt", "[{0}] {1}"), KeyText, ActionText));

	// 글자일 뿐이므로 마우스 클릭을 가로채지 않게 한다.
	mInteractText->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UPlayerHUDWidget::SetDialogueLine(const FString& Speaker, const FString& Text)
{
	mDialogueWidget->SetLine(Speaker, Text);
	SetDialogueVisible(true);
}

void UPlayerHUDWidget::SetDialogueVisible(bool Visible)
{
	// 대사 넘기기는 Enhanced Input(마우스 왼쪽 포함)이 받는다. 대화창은 보이기만 하고 클릭을 가로채지 않게 한다.
	// SelfHitTestInvisible은 자기만 통과시키고 자식(배경 Border)은 클릭을 받으므로, 자식까지 통과시키는 HitTestInvisible을 쓴다.
	mDialogueWidget->SetVisibility(Visible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
