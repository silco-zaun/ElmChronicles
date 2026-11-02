// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "../Save/ElmSaveGame.h"
#include "Components/ActorComponent.h"
#include "QuestComponent.generated.h"

struct FQuestTableRow;

// 퀘스트 목록이 바뀌면(받기, 처치 수 증가, 완료, 메모에서 되살리기) 알린다. 추적창과 퀘스트 창이 듣는다. (Step 4, 5)
DECLARE_MULTICAST_DELEGATE(FOnQuestChanged);

// 플레이어가 받은 퀘스트를 들고 있다. 받기, 처치 세기, 완료와 보상을 맡는다.
// AElmPlayerController에 붙는다. 레벨을 옮기면 컨트롤러와 함께 새로 만들어지므로 진행도는 메모(UElmSaveGame::Quests)로 넘긴다.
UCLASS()
class ELMCHRONICLES_API UQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuestComponent();

protected:
	// 받은 퀘스트. 받은 순서대로 쌓이고 완료해도 지우지 않는다. (퀘스트 창에 완료로 보이고, 같은 퀘스트를 다시 받지 못하게 한다)
	TArray<FQuestProgress> mQuests;

	FOnQuestChanged mOnQuestChanged;

protected:
	virtual void BeginPlay() override;

public:
	const TArray<FQuestProgress>& GetQuests() const { return mQuests; }
	// 받지 않은 퀘스트면 None
	EQuestState GetQuestState(FName QuestRow) const;
	// DT_Quest에 있고 아직 받지 않은 퀘스트만 받을 수 있다.
	bool CanAcceptQuest(FName QuestRow) const;
	void AcceptQuest(FName QuestRow);
	// 플레이어가 몬스터를 잡으면 그 몬스터가 부른다. MonsterRow는 잡힌 몬스터의 DT_Monster 행 이름이다.
	void NotifyMonsterKilled(FName MonsterRow);
	// DT_Quest에서 행을 찾는다. 없거나 None이면 nullptr.
	const FQuestTableRow* FindQuestInfo(FName QuestRow) const;
	// 목표 글자 ("해골전사 처치 0 / 1"). 추적창과 퀘스트 창이 함께 쓴다.
	FText MakeObjectiveText(const FQuestProgress& Progress) const;
	// 퀘스트 목록 → 메모. 레벨 이동 / 저장 직전에 캐릭터가 부른다.
	void WriteToSave(TArray<FQuestProgress>& OutQuests) const;

	template <typename T>
	void AddQuestChangeCallback(T* Obj, void (T::* Func)())
	{
		mOnQuestChanged.AddUObject(Obj, Func);
	}

	void RemoveCallbacks(const UObject* Obj)
	{
		mOnQuestChanged.RemoveAll(Obj);
	}

private:
	// 테이블이 로드되면 메모에서 퀘스트 목록을 되살린다. (UInventoryComponent::InitInventory와 같은 방식)
	void InitQuests();
	void CompleteQuest(FQuestProgress& Progress, const FQuestTableRow& Info);
	// 보상 경험치 / 골드를 GE_Reward로 플레이어 ASC에 준다.
	void GiveReward(const FQuestTableRow& Info);
};
