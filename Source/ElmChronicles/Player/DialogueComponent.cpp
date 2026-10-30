// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueComponent.h"
#include "ElmPlayerController.h"
#include "InteractionComponent.h"
#include "../Characters/NPCCharacter.h"
#include "../Data/Tables/DialogueTableRow.h"
#include "../Input/DefaultInput.h"
#include "../Input/DialogueInput.h"
#include "../Subsystem/AssetSubsystem.h"
#include "../UI/Game/PlayerHUDWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

UDialogueComponent::UDialogueComponent()
{
	// 대사 넘기기는 입력이 올 때만 하므로 Tick은 쓰지 않는다.
	PrimaryComponentTick.bCanEverTick = false;
}

void UDialogueComponent::StartDialogue(ANPCCharacter* NPC, FName StartRow)
{
	AElmPlayerController* PlayerController = GetOwner<AElmPlayerController>();

	// 이미 대화 중이면 새로 시작하지 않는다. 대화창을 띄울 HUD가 없어도 시작하지 않는다.
	if (IsInDialogue() || !IsValid(NPC) || !IsValid(PlayerController) || !PlayerController->GetPlayerHUDWidget())
		return;

	// 첫 줄을 먼저 찾아 본다. 없는데 시작하면 입력만 대화용으로 바뀌고 창은 비어서 조작이 막힌다.
	const FDialogueTableRow* Line = FindDialogueLine(StartRow);

	if (!Line)
	{
		UE_LOG(LogTestDebug, Error, TEXT("StartDialogue : %s의 대화 시작 행 '%s'을(를) DT_Dialogue에서 찾지 못했습니다."),
			*NPC->GetName(), *StartRow.ToString());
		return;
	}

	mDialogueNPC = NPC;
	mDialogueRow = StartRow;

	// 인벤토리를 닫는다. RefreshInputMode가 커서도 함께 숨긴다.
	PlayerController->SetInventoryOpen(false);

	// 대화 중에는 다른 대상과 상호작용하지 않는다. 대상을 비우고 안내 문구를 숨기고 검색을 멈춘다.
	PlayerController->GetInteraction()->SetBlocked(true);

	SetDialogueInput(true);
	PlayerController->GetPlayerHUDWidget()->SetDialogueLine(Line->Speaker, Line->Text);
}

void UDialogueComponent::AdvanceDialogue()
{
	AElmPlayerController* PlayerController = GetOwner<AElmPlayerController>();

	if (!IsInDialogue() || !IsValid(PlayerController) || !PlayerController->GetPlayerHUDWidget())
		return;

	const FDialogueTableRow* Current = FindDialogueLine(mDialogueRow);

	// NextRow가 None이면 마지막 줄이다. FindRow에 None을 넣으면 경고 로그가 남으므로 먼저 거른다.
	if (!Current || Current->NextRow.IsNone())
	{
		EndDialogue();
		return;
	}

	const FDialogueTableRow* Next = FindDialogueLine(Current->NextRow);

	// 다음 행 이름이 틀렸어도(오타) 대화창에 갇히지 않도록 끝낸다. 어떤 이름이 틀렸는지는 FindRow가 경고 로그로 남긴다.
	if (!Next)
	{
		EndDialogue();
		return;
	}

	mDialogueRow = Current->NextRow;
	PlayerController->GetPlayerHUDWidget()->SetDialogueLine(Next->Speaker, Next->Text);
}

void UDialogueComponent::EndDialogue()
{
	// 마지막 줄, 전투, 피격처럼 여러 곳에서 불러도 한 번만 정리한다.
	if (!IsInDialogue())
		return;

	mDialogueNPC = nullptr;
	mDialogueRow = NAME_None;

	AElmPlayerController* PlayerController = GetOwner<AElmPlayerController>();

	if (!IsValid(PlayerController))
		return;

	if (PlayerController->GetPlayerHUDWidget())
		PlayerController->GetPlayerHUDWidget()->SetDialogueVisible(false);

	SetDialogueInput(false);

	// 다음 검색(0.1초 안)부터 다시 대상을 찾아 안내 문구를 띄운다.
	PlayerController->GetInteraction()->SetBlocked(false);
}

const FDialogueTableRow* UDialogueComponent::FindDialogueLine(FName Row) const
{
	// 컴포넌트에는 GetGameInstance가 없어서 월드를 거친다. (UInventoryComponent와 같은 방식)
	UAssetSubsystem* AssetSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UAssetSubsystem>();

	return AssetSubsystem ? AssetSubsystem->FindRow<FDialogueTableRow>(Row) : nullptr;
}

void UDialogueComponent::SetDialogueInput(bool Dialogue)
{
	const APlayerController* PlayerController = GetOwner<APlayerController>();

	if (!IsValid(PlayerController))
		return;

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	if (!InputSubsystem)
		return;

	const UDefaultInput* DefaultInputCDO = GetDefault<UDefaultInput>();
	const UDialogueInput* DialogueInputCDO = GetDefault<UDialogueInput>();

	// 옵션을 넘기지 않으면 기본값(bIgnoreAllPressedKeysUntilRelease)이 바꾸는 순간 눌려 있던 키를 뗄 때까지 무시한다.
	// 그래서 대화를 시작한 F가 첫 줄을 넘기지 않고, 마지막 줄을 넘긴 F / Space / 클릭이 상호작용 / 점프 / 공격이 되지 않는다.
	// (Value Type이 Boolean인 액션에만 적용된다. IA_DialogueNext를 Digital(bool)로 만든 이유다)
	if (Dialogue)
	{
		InputSubsystem->RemoveMappingContext(DefaultInputCDO->mInputMappingContext);
		InputSubsystem->AddMappingContext(DialogueInputCDO->mInputMappingContext, 0);
	}
	else
	{
		InputSubsystem->RemoveMappingContext(DialogueInputCDO->mInputMappingContext);
		InputSubsystem->AddMappingContext(DefaultInputCDO->mInputMappingContext, 0);
	}
}
