// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelPortal.h"
#include "../ElmCollisionChannels.h"
#include "../Subsystem/SaveSubsystem.h"
#include "Components/BoxComponent.h"
#include "Engine/GameInstance.h"

ALevelPortal::ALevelPortal()
{
	PrimaryActorTick.bCanEverTick = false;

	mBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(mBox);

	mBox->SetBoxExtent(FVector(200.0, 200.0, 200.0));

	// 몸은 막지 않고, 플레이어의 상호작용 검색(Interaction 채널)에만 걸린다. (NPC 캡슐과 같은 방식)
	// 문이나 벽은 맵에 있는 메시가 막으므로 박스는 "여기서 E를 누를 수 있다"는 영역만 맡는다.
	mBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	mBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	mBox->SetCollisionResponseToChannel(ElmChannels::Interaction, ECR_Overlap);

	mInteractText = NSLOCTEXT("ElmPortal", "Enter", "이동하기");
}

FText ALevelPortal::GetInteractText() const
{
	return mInteractText;
}

bool ALevelPortal::CanInteract(const APlayerCharacter* Player) const
{
	// 갈 레벨을 고르지 않은 포탈은 안내 문구도 띄우지 않는다.
	return !mTargetLevel.IsNull();
}

void ALevelPortal::Interact(APlayerCharacter* Player)
{
	// 메모 적기와 레벨 열기는 서브시스템이 한다. 포탈은 "어디로 갈지"만 안다.
	if (USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>())
		SaveSubsystem->TravelToLevel(Player, mTargetLevel, mTargetStartTag);
}
