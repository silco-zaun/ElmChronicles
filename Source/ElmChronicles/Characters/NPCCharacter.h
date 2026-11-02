// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "../Data/Tables/NPCCharacterTableRow.h"
#include "../Interaction/Interactable.h"
#include "AICharacter.h"
#include "NPCCharacter.generated.h"

// 마을 NPC. 몬스터와 같은 AI / 공격 구조(AICharacter)를 쓰고 팀만 NPC다.
// 외형과 능력치는 DT_NPC의 행(mRowName)에서 읽는다.
// 대화 행이 있으면 플레이어가 상호작용 키로 말을 걸 수 있다.
UCLASS()
class ELMCHRONICLES_API ANPCCharacter : public AAICharacter, public IInteractable
{
	GENERATED_BODY()

public:
	// AAICharacter가 만드는 AttributeSet을 UNPCAttributeSet으로 바꿔 끼운다.
	ANPCCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	// 필수 NPC(퀘스트 NPC). HP가 0이 되면 쓰러졌다가 mReviveTime초 뒤 일어난다. (스카이림의 Essential)
	bool mIsEssential = false;
	float mReviveTime = 10.f;
	// 대화 시작 행 (DT_Dialogue). None이면 대화할 수 없다.
	FName mDialogueRow;
	// 이 NPC가 주는 퀘스트 (DT_Quest). None이면 퀘스트가 없다.
	FName mQuestRow;
	FTimerHandle mReviveTimer;

	// 일어날 때 채울 HP 비율 (MaxHP 기준)
	static constexpr float ReviveHPRatio = 0.5f;

public:
	// 대화 컴포넌트가 대화를 끝까지 진행하면 이 퀘스트를 플레이어에게 준다.
	FName GetQuestRow() const { return mQuestRow; }

public:
	virtual void Death() override;

	// IInteractable
	virtual FText GetInteractText() const override;
	virtual bool CanInteract(const APlayerCharacter* Player) const override;
	virtual void Interact(APlayerCharacter* Player) override;

	// 대화 컴포넌트가 대화를 시작 / 끝낼 때 부른다.
	// 시작: BT를 멈추고 제자리에 서서 Listener를 바라본다. 끝: 바라보기를 풀고 BT를 다시 돌린다.
	void BeginTalk(APawn* Listener);
	void EndTalk();

protected:
	virtual void InitAttributes() override;
	virtual const FNPCCharacterTableRow* FindTableRow() const override;
	virtual void OnKilledBy(AActor* Killer) override;

private:
	void Revive();
	// 퀘스트 진행 상태에 맞는 대화 시작 행을 고른다.
	// (받기 전 = DialogueRow, 진행 중 / 완료 = 퀘스트의 대사)
	FName ChooseDialogueRow(const class UQuestComponent* Quest) const;
};
