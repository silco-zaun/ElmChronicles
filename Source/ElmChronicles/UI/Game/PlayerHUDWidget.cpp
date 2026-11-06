// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"
#include "CelebrationWidget.h"
#include "CharacterInfoWidget.h"
#include "ConfirmWidget.h"
#include "DialogueWidget.h"
#include "InventoryWidget.h"
#include "ItemInfoWidget.h"
#include "MenuWidget.h"
#include "QuestWidget.h" 
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
	SetQuestVisible(false);
	SetCharacterInfoVisible(false);
}

void UPlayerHUDWidget::SetPlayerName(const FString& Name)
{
	mStatusBarWidget->SetName(Name);
}

void UPlayerHUDWidget::SetItemInfo(UItemObject* Item)
{
	mItemInfoWidget->SetItemInfo(Item->GetItemInfo(), Item->GetItemUpgrade());
}

void UPlayerHUDWidget::ShowItemInfoWidget(bool IsShow, ESlotType From)
{
	if (IsShow)
	{
		// 창은 드래그로 옮길 수 있으므로 띄울 때마다 자리를 다시 잡는다.
		// 캐릭터 창(화면 왼쪽)은 창 오른쪽에, 인벤토리(화면 오른쪽)는 창 왼쪽에 붙여서 화면 밖으로 나가지 않게 한다.
		if (From == ESlotType::Equipment)
			PlaceItemInfoNextTo(mCharacterInfoWidget, false);
		else
			PlaceItemInfoNextTo(mInventoryWidget, true);

		// 정보창은 보이기만 한다. SelfHitTestInvisible이면 자식(배경 등)이 마우스를 받아서,
		// 정보창이 아이콘과 겹칠 때 아이콘의 Hover를 빼앗아 깜빡인다. 그래서 자식까지 통과시킨다.
		mItemInfoWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
		mItemInfoWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UPlayerHUDWidget::PlaceItemInfoNextTo(const UWidget* Window, bool PlaceOnLeft)
{
	UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(mItemInfoWidget->Slot);
	UPanelWidget* ParentPanel = mItemInfoWidget->GetParent();

	if (!PanelSlot || !ParentPanel)
		return;

	// 아래 위치는 Canvas 왼쪽 위 기준으로 계산하므로 앵커를 왼쪽 위로 고정한다. (WBP 앵커가 바뀌어도 화면 밖으로 나가지 않는다)
	// Alignment가 Position을 정보창의 어느 모서리로 볼지 정한다. 왼쪽에 붙일 때 (1, 0) = 오른쪽 위 모서리,
	// 오른쪽에 붙일 때 (0, 0) = 왼쪽 위 모서리. 그래서 정보창 크기를 몰라도 창 가장자리에 맞출 수 있다.
	PanelSlot->SetAnchors(FAnchors(0.f, 0.f));
	PanelSlot->SetAlignment(FVector2D(PlaceOnLeft ? 1.0 : 0.0, 0.0));

	// 창의 왼쪽 위 / 오른쪽 위 모서리를 화면 좌표로 구한다.
	// 타이틀바 드래그로 옮긴 양(RenderTranslation)도 Geometry에 들어 있으므로 옮긴 자리를 따라간다.
	const FGeometry& WindowGeometry = Window->GetCachedGeometry();
	const FVector2D WindowCorner = WindowGeometry.LocalToAbsolute(
		PlaceOnLeft ? FVector2D::ZeroVector : FVector2D(WindowGeometry.GetLocalSize().X, 0.0));

	// Canvas 기준 좌표로 바꾸고, 창과 딱 붙지 않게 조금 띄운다.
	FVector2D WidgetPos = ParentPanel->GetCachedGeometry().AbsoluteToLocal(WindowCorner);
	WidgetPos.X += PlaceOnLeft ? -8.0 : 8.0;

	PanelSlot->SetPosition(WidgetPos);
}

void UPlayerHUDWidget::SetInventoryVisible(bool Visible)
{
	mInventoryWidget->SetVisibility(Visible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	// 아이콘 위에 마우스를 올린 채로 창을 닫으면 MouseLeave가 오지 않는다.
	// 그래서 정보창도 같이 닫는다.
	if (!Visible)
		ShowItemInfoWidget(false);
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
		ShowItemInfoWidget(false);
	}

	mMenuWidget->SetVisibility(Visible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

bool UPlayerHUDWidget::IsMenuVisible() const
{
	// IsInventoryVisible과 같은 이유로 직접 설정한 값과 비교한다.
	return mMenuWidget->GetVisibility() == ESlateVisibility::Visible;
}

void UPlayerHUDWidget::SetQuestVisible(bool Visible)
{
	// 인벤토리 창처럼 Visible로 둔다. 창 위를 클릭해도 뒤의 게임(공격)으로 새지 않는다.
	mQuestWidget->SetVisibility(Visible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

bool UPlayerHUDWidget::IsQuestVisible() const
{
	// IsInventoryVisible과 같은 이유로 직접 설정한 값과 비교한다.
	return mQuestWidget->GetVisibility() == ESlateVisibility::Visible;
}

void UPlayerHUDWidget::SetCharacterInfoVisible(bool Visible)
{
	// 인벤토리 창처럼 Visible로 둔다. 창 위를 클릭해도 뒤의 게임(공격)으로 새지 않는다.
	mCharacterInfoWidget->SetVisibility(Visible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	// 미리보기는 창이 보일 때만 찍는다.
	mCharacterInfoWidget->SetPreviewActive(Visible);

	// SetInventoryVisible과 같은 이유로 정보창도 닫는다. (장비칸 아이콘 위에서 C를 누르면 MouseLeave가 오지 않는다)
	if (!Visible)
		ShowItemInfoWidget(false);
}

bool UPlayerHUDWidget::IsCharacterInfoVisible() const
{
	// IsInventoryVisible과 같은 이유로 직접 설정한 값과 비교한다.
	return mCharacterInfoWidget->GetVisibility() == ESlateVisibility::Visible;
}

bool UPlayerHUDWidget::IsAnyWindowOpen() const
{
	// 나중에 스킬창이 생기면 여기에 || 로 추가한다.
	// 확인 상자는 창이 없을 때(Alt+F4)도 뜨므로, 그때도 커서가 보이게 같이 본다.
	return IsInventoryVisible() || IsMenuVisible() || IsQuestVisible() || IsCharacterInfoVisible() || IsConfirmVisible();
}

void UPlayerHUDWidget::OpenConfirm(const FText& Message, FSimpleDelegate OnYes)
{
	// SetMenuVisible과 같은 이유로 정보창을 닫는다. (아이콘 위에 마우스를 둔 채 Alt+F4를 누르면 남는다)
	ShowItemInfoWidget(false);

	mConfirmWidget->Open(Message, MoveTemp(OnYes));
}

void UPlayerHUDWidget::CloseConfirm()
{
	mConfirmWidget->Close();
}

bool UPlayerHUDWidget::IsConfirmVisible() const
{
	// IsInventoryVisible과 같은 이유로 직접 설정한 값과 비교한다.
	return mConfirmWidget->GetVisibility() == ESlateVisibility::Visible;
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

void UPlayerHUDWidget::ShowCelebration(const FText& Title, const FText& SubText)
{
	mCelebrationWidget->Push(Title, SubText);
}
