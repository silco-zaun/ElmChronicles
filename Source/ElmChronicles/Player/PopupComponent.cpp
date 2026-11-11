// Fill out your copyright notice in the Description page of Project Settings.


#include "PopupComponent.h"
#include "../UI/Common/OptionsWidget.h"
#include "../UI/Common/PopupLayerWidget.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

UPopupComponent::UPopupComponent()
{
	// 열고 닫을 때만 일하므로 Tick이 필요 없다.
	PrimaryComponentTick.bCanEverTick = false;

	// 컨트롤러 C++ 클래스에는 BP 자식이 없어서 Details에서 고를 곳이 없다. HUD처럼 경로로 찾는다.
	static ConstructorHelpers::FClassFinder<UPopupLayerWidget>
		PopupLayerWidgetClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/Common/WBP_PopupLayer.WBP_PopupLayer_C'"));

	if (PopupLayerWidgetClass.Succeeded())
		mPopupLayerWidgetClass = PopupLayerWidgetClass.Class;
}

void UPopupComponent::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());

	// HUD와 같이, 이 컨트롤러가 조작하는 로컬 플레이어 화면에만 만든다.
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController() || !mPopupLayerWidgetClass)
		return;

	// 첫 번째 인자 = Owning Player. 팝업 위젯이 GetOwningPlayer()로 컨트롤러를 찾고, 거기서 이 컴포넌트를 찾는다.
	mPopupLayerWidget = CreateWidget<UPopupLayerWidget>(PlayerController, mPopupLayerWidgetClass);

	// 이 BeginPlay는 컨트롤러 BeginPlay의 Super 안에서 불려서 HUD보다 먼저 화면에 붙는다.
	// 그래도 그리는 순서는 붙인 순서가 아니라 ZOrder가 정하므로 HUD 위에 보인다.
	if (mPopupLayerWidget)
		mPopupLayerWidget->AddToViewport(PopupLayerZOrder);
}

UPopupComponent* UPopupComponent::Get(const APlayerController* PlayerController)
{
	return IsValid(PlayerController) ? PlayerController->FindComponentByClass<UPopupComponent>() : nullptr;
}

void UPopupComponent::OpenOptions()
{
	if (mPopupLayerWidget)
		mPopupLayerWidget->GetOptions()->Open();
}

void UPopupComponent::CloseOptions()
{
	if (mPopupLayerWidget)
		mPopupLayerWidget->GetOptions()->Close();
}
