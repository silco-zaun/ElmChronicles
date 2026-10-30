// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"
#include "ElmPlayerController.h"
#include "../Characters/PlayerCharacter.h"
#include "../ElmCollisionChannels.h"
#include "../Interaction/Interactable.h"
#include "../UI/Game/PlayerHUDWidget.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "TimerManager.h"

UInteractionComponent::UInteractionComponent()
{
	// 대상 검색은 타이머로 0.1초마다 하므로 Tick은 쓰지 않는다.
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// 안내 문구를 띄울 화면이 있는 로컬 플레이어만 찾는다.
	// 이 함수는 컨트롤러 BeginPlay의 Super 안에서 불려서 아직 HUD가 없다. 첫 검색은 0.1초 뒤라 그때는 HUD가 있다.
	const APlayerController* PlayerController = GetOwner<APlayerController>();

	if (IsValid(PlayerController) && PlayerController->IsLocalController())
	{
		GetWorld()->GetTimerManager().SetTimer(mInteractTimer, this,
			&UInteractionComponent::UpdateInteractTarget, InteractScanInterval, true);
	}
}

void UInteractionComponent::InteractKey()
{
	const AElmPlayerController* PlayerController = GetOwner<AElmPlayerController>();
	APlayerCharacter* PlayerCharacter = IsValid(PlayerController) ? PlayerController->GetPawn<APlayerCharacter>() : nullptr;
	IInteractable* Interactable = Cast<IInteractable>(mInteractTarget.Get());

	// 죽은 뒤 첫 검색(0.1초) 전에는 mInteractTarget이 남아 있다.
	// 죽은 채로 포탈을 타면 HP 0이 메모에 적힌다.
	if (mIsBlocked || !IsValid(PlayerCharacter) || PlayerCharacter->IsDead() || !Interactable ||
		!Interactable->CanInteract(PlayerCharacter))
		return;

	Interactable->Interact(PlayerCharacter);
}

void UInteractionComponent::SetBlocked(bool Blocked)
{
	mIsBlocked = Blocked;

	// 풀 때는 할 일이 없다. 다음 검색이 대상과 문구를 다시 채운다.
	if (!Blocked)
		return;

	// 막는 순간 대상을 비우고 문구를 숨긴다. 막혀 있는 동안 검색은 아무것도 하지 않으므로 문구가 다시 뜨지 않는다.
	mInteractTarget = nullptr;

	const AElmPlayerController* PlayerController = GetOwner<AElmPlayerController>();

	if (IsValid(PlayerController) && PlayerController->GetPlayerHUDWidget())
		PlayerController->GetPlayerHUDWidget()->SetInteractPrompt(FText::GetEmpty());
}

void UInteractionComponent::UpdateInteractTarget()
{
	const AElmPlayerController* PlayerController = GetOwner<AElmPlayerController>();

	// 막혀 있는 동안(대화 중)은 찾지 않는다. 대상과 문구는 SetBlocked(true)가 이미 비웠다.
	if (mIsBlocked || !IsValid(PlayerController))
		return;

	APlayerCharacter* PlayerCharacter = PlayerController->GetPawn<APlayerCharacter>();
	AActor* NewTarget = nullptr;

	if (IsValid(PlayerCharacter) && !PlayerCharacter->IsDead())
	{
		// Interaction 채널에 응답하는 컴포넌트(NPC 캡슐, 포탈 박스)만 걸린다. 벽, 몬스터, 플레이어는 Ignore라 걸리지 않는다.
		// (Lyra의 GrantNearbyInteraction도 플레이어 주변 구를 타이머로 Overlap해서 찾는다)
		TArray<FOverlapResult> Overlaps;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(Interact), false, PlayerCharacter);

		GetWorld()->OverlapMultiByChannel(Overlaps, PlayerCharacter->GetActorLocation(), FQuat::Identity,
			ElmChannels::Interaction, FCollisionShape::MakeSphere(InteractRadius), Params);

		double NearestDistSq = TNumericLimits<double>::Max();

		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* Actor = Overlap.GetActor();
			// 인터페이스를 구현한 액터만 Cast가 성공한다. (NPC인지 상자인지는 몰라도 된다)
			IInteractable* Interactable = Cast<IInteractable>(Actor);

			if (!Interactable || !Interactable->CanInteract(PlayerCharacter))
				continue;

			// 여러 명이 걸리면 가장 가까운 대상
			const double DistSq = FVector::DistSquared(PlayerCharacter->GetActorLocation(), Actor->GetActorLocation());

			if (DistSq < NearestDistSq)
			{
				NearestDistSq = DistSq;
				NewTarget = Actor;
			}
		}
	}

	mInteractTarget = NewTarget;

	// 대상이 사라지거나 문구가 바뀌어도 바로 반영되도록 매번 갱신한다. (0.1초마다라 부담이 없다)
	UPlayerHUDWidget* HUD = PlayerController->GetPlayerHUDWidget();

	if (HUD)
	{
		IInteractable* Interactable = Cast<IInteractable>(NewTarget);
		HUD->SetInteractPrompt(Interactable ? Interactable->GetInteractText() : FText::GetEmpty());
	}
}
