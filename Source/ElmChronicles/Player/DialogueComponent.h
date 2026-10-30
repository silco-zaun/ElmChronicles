// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "Components/ActorComponent.h"
#include "DialogueComponent.generated.h"

struct FDialogueTableRow;

// NPC와의 대화를 진행한다. 대화창에 한 줄씩 띄우고, 대화 중에는 입력을 대화용(IMC_Dialogue)으로 바꾼다.
// AElmPlayerController에 붙는다. 대사는 DT_Dialogue에서 NextRow를 따라가며 읽는다.
UCLASS()
class ELMCHRONICLES_API UDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDialogueComponent();

protected:
	// 대화 중인 NPC. NPC가 사라지면 자동으로 nullptr이 되도록 약한 포인터로 들고 있는다.
	TWeakObjectPtr<class ANPCCharacter> mDialogueNPC;
	// 지금 대화창에 보이는 DT_Dialogue 행. None이면 대화 중이 아니다.
	// NPC가 사라져도 대화 중인 것은 알아야 정리할 수 있으므로, 대화 중인지는 NPC가 아니라 이 값으로 판단한다.
	FName mDialogueRow;

public:
	bool IsInDialogue() const { return !mDialogueRow.IsNone(); }

	// NPC의 Interact가 부른다. StartRow부터 대화창을 띄우고 입력을 대화용(IMC_Dialogue)으로 바꾼다.
	void StartDialogue(class ANPCCharacter* NPC, FName StartRow);
	// 다음 대사로 넘긴다. 마지막 줄이면 대화를 끝낸다. (IA_DialogueNext: F / Space / 마우스 왼쪽)
	void AdvanceDialogue();
	// 대화창을 닫고 원래 조작(IMC_Default)으로 돌린다. 대화 중이 아니면 아무것도 하지 않는다.
	void EndDialogue();

private:
	// DT_Dialogue에서 행을 찾는다. 없으면 nullptr.
	const FDialogueTableRow* FindDialogueLine(FName Row) const;
	// true면 IMC_Default를 빼고 IMC_Dialogue를 넣는다. false면 반대로 한다.
	void SetDialogueInput(bool Dialogue);
};
