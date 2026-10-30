// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueInput.h"
#include "UObject/ConstructorHelpers.h"

UDialogueInput::UDialogueInput()
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DialogueMappingContext(
		TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Input/Dialogue/IMC_Dialogue.IMC_Dialogue'"));

	if (DialogueMappingContext.Succeeded())
		mInputMappingContext = DialogueMappingContext.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> DialogueNextInputAction(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Dialogue/IA_DialogueNext.IA_DialogueNext'"));

	if (DialogueNextInputAction.Succeeded())
		mInputActionMap.Add(TEXT("DialogueNext"), DialogueNextInputAction.Object);
}
