// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestComponent.h"
#include "ElmPlayerController.h"
#include "../AbilitySystem/Attributes/PlayerAttributeSet.h"
#include "../AbilitySystem/Effects/GameplayEffect_Reward.h"
#include "../AbilitySystem/ElmGameplayTags.h"
#include "../Data/Tables/MonsterCharacterTableRow.h"
#include "../Data/Tables/QuestTableRow.h"
#include "../Subsystem/AssetSubsystem.h"
#include "../Subsystem/SaveSubsystem.h"
#include "../UI/Game/PlayerHUDWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

UQuestComponent::UQuestComponent()
{
	// 퀘스트는 받을 때와 몬스터를 잡을 때만 바뀌므로 Tick은 쓰지 않는다.
	PrimaryComponentTick.bCanEverTick = false;
}

void UQuestComponent::BeginPlay()
{
	Super::BeginPlay();

	// 컨트롤러 BeginPlay의 Super 안에서 불려서 아직 HUD가 없다. 여기서는 메모만 읽고 HUD는 건드리지 않는다.
	// 목록을 되살린 뒤 알림을 받은 화면(추적창)이 DT_Quest에서 제목을 찾으므로, 테이블이 로드된 뒤에 되살린다.
	UAssetSubsystem* AssetSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UAssetSubsystem>();

	if (AssetSubsystem)
		AssetSubsystem->CallOrRegisterDataLoaded(this, &UQuestComponent::InitQuests);
}

void UQuestComponent::InitQuests()
{
	USaveSubsystem* SaveSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<USaveSubsystem>();
	const UElmSaveGame* Save = SaveSubsystem ? SaveSubsystem->GetSaveGame() : nullptr;

	// 메모가 없으면 새 게임이라 받은 퀘스트가 없다.
	if (!Save)
		return;

	// 진행도는 값만 있는 구조체라 통째로 복사하면 된다.
	mQuests = Save->Quests;

	UE_LOG(LogTestDebug, Warning, TEXT("InitQuests : 메모에서 퀘스트 %d개를 불러왔습니다."), mQuests.Num());

	mOnQuestChanged.Broadcast();
}

EQuestState UQuestComponent::GetQuestState(FName QuestRow) const
{
	for (const FQuestProgress& Progress : mQuests)
	{
		if (Progress.QuestRow == QuestRow)
			return Progress.State;
	}

	return EQuestState::None;
}

bool UQuestComponent::CanAcceptQuest(FName QuestRow) const
{
	return FindQuestInfo(QuestRow) && GetQuestState(QuestRow) == EQuestState::None;
}

void UQuestComponent::AcceptQuest(FName QuestRow)
{
	// 같은 퀘스트를 두 번 받거나, 테이블에 없는 이름(오타)을 받지 않는다.
	if (!CanAcceptQuest(QuestRow))
		return;

	FQuestProgress& Progress = mQuests.AddDefaulted_GetRef();
	Progress.QuestRow = QuestRow;
	Progress.State = EQuestState::InProgress;
	Progress.KillCount = 0;

	UE_LOG(LogTestDebug, Warning, TEXT("AcceptQuest : %s"), *QuestRow.ToString());

	mOnQuestChanged.Broadcast();
}

void UQuestComponent::NotifyMonsterKilled(FName MonsterRow)
{
	bool Changed = false;

	for (FQuestProgress& Progress : mQuests)
	{
		if (Progress.State != EQuestState::InProgress)
			continue;

		const FQuestTableRow* Info = FindQuestInfo(Progress.QuestRow);

		// 이 몬스터를 잡는 퀘스트가 아니면 넘어간다.
		if (!Info || Info->TargetMonster != MonsterRow)
			continue;

		++Progress.KillCount;
		Changed = true;

		UE_LOG(LogTestDebug, Warning, TEXT("퀘스트 진행 : %s %d / %d"), *Info->Title, Progress.KillCount, Info->TargetCount);

		// 목표 수를 채운 순간 완료된다. (NPC에게 돌아가 보고하지 않는다)
		if (Progress.KillCount >= Info->TargetCount)
			CompleteQuest(Progress, *Info);
	}

	// 한 마리가 여러 퀘스트를 진행시켜도 알림은 한 번만 보낸다.
	if (Changed)
		mOnQuestChanged.Broadcast();
}

const FQuestTableRow* UQuestComponent::FindQuestInfo(FName QuestRow) const
{
	// None을 FindRow에 넣으면 경고 로그가 남는다. 퀘스트가 없는 NPC / 대사도 여기를 거치게 되므로 먼저 거른다.
	if (QuestRow.IsNone())
		return nullptr;

	// 컴포넌트에는 GetGameInstance가 없어서 월드를 거친다. (UInventoryComponent와 같은 방식)
	UAssetSubsystem* AssetSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UAssetSubsystem>();

	return AssetSubsystem ? AssetSubsystem->FindRow<FQuestTableRow>(QuestRow) : nullptr;
}

FText UQuestComponent::MakeObjectiveText(const FQuestProgress& Progress) const
{
	const FQuestTableRow* Info = FindQuestInfo(Progress.QuestRow);

	if (!Info)
		return FText::GetEmpty();

	// 몬스터 이름은 DT_Monster의 Name(해골전사)을 쓴다. 행 이름이 틀렸으면 행 이름이라도 보여 준다.
	UAssetSubsystem* AssetSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UAssetSubsystem>();
	const FMonsterCharacterTableRow* Monster =
		AssetSubsystem ? AssetSubsystem->FindRow<FMonsterCharacterTableRow>(Info->TargetMonster) : nullptr;
	const FString MonsterName = Monster ? Monster->Name : Info->TargetMonster.ToString();

	// 완료한 뒤에 같은 몬스터를 더 잡아도 목표 수를 넘겨 보이지 않게 한다.
	const int32 Count = FMath::Min(Progress.KillCount, Info->TargetCount);

	return FText::Format(NSLOCTEXT("ElmQuest", "KillObjective", "{0} 처치 {1} / {2}"),
		FText::FromString(MonsterName), Count, Info->TargetCount);
}

void UQuestComponent::WriteToSave(TArray<FQuestProgress>& OutQuests) const
{
	OutQuests = mQuests;
}

void UQuestComponent::CompleteQuest(FQuestProgress& Progress, const FQuestTableRow& Info)
{
	Progress.State = EQuestState::Completed;

	UE_LOG(LogTestDebug, Warning, TEXT("퀘스트 완료 : %s"), *Info.Title);

	// 보상보다 먼저 띄운다. 보상 경험치로 레벨이 오르면 GiveReward 안에서 "LEVEL UP!"이 줄을 서므로,
	// 이 순서여야 "퀘스트 완료" → "LEVEL UP!" 차례로 보인다.
	const AElmPlayerController* PlayerController = GetOwner<AElmPlayerController>();

	if (IsValid(PlayerController) && PlayerController->GetPlayerHUDWidget())
		PlayerController->GetPlayerHUDWidget()->ShowCelebration(
			NSLOCTEXT("ElmQuest", "QuestCompleteTitle", "퀘스트 완료"), FText::FromString(Info.Title));

	GiveReward(Info);
}

void UQuestComponent::GiveReward(const FQuestTableRow& Info)
{
	// 플레이어의 ASC는 PlayerState가 가지고 있다. (경험치 / 골드 Attribute가 거기에 있다)
	const APlayerController* PlayerController = GetOwner<APlayerController>();
	UAbilitySystemComponent* ASC = IsValid(PlayerController) ?
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerController->GetPlayerState<APlayerState>()) : nullptr;

	if (!ASC)
		return;

	// 몬스터 처치 보상과 같은 GE를 쓴다. Attribute를 직접 바꾸지 않는다.
	// 몬스터 보상은 주는 쪽(몬스터) ASC로 스펙을 만들지만, 퀘스트는 주는 쪽이 따로 없으므로 플레이어 ASC로 만들어 자기에게 적용한다.
	FGameplayEffectSpecHandle RewardSpec = ASC->MakeOutgoingSpec(
		UGameplayEffect_Reward::StaticClass(), 1.f, ASC->MakeEffectContext());

	if (!RewardSpec.IsValid())
		return;

	// GE_Reward의 Modifier 두 개가 모두 SetByCaller이므로 둘 다 항상 채워야 한다.
	RewardSpec.Data->SetSetByCallerMagnitude(ElmTags::SetByCaller_Reward_Exp, static_cast<float>(Info.RewardExp));
	RewardSpec.Data->SetSetByCallerMagnitude(ElmTags::SetByCaller_Reward_Gold, static_cast<float>(Info.RewardGold));

	ASC->ApplyGameplayEffectSpecToSelf(*RewardSpec.Data.Get());

	// 경험치 / 골드를 보여 주는 UI가 아직 없으므로 받은 뒤의 값을 로그로 남긴다.
	UE_LOG(LogTestDebug, Warning, TEXT("퀘스트 보상 : 경험치 +%d → %.0f, 골드 +%d → %.0f"),
		Info.RewardExp, ASC->GetNumericAttribute(UPlayerAttributeSet::GetExpAttribute()),
		Info.RewardGold, ASC->GetNumericAttribute(UPlayerAttributeSet::GetGoldAttribute()));
}
