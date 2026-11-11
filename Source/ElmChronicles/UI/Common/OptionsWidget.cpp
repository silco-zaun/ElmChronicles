// Fill out your copyright notice in the Description page of Project Settings.


#include "OptionsWidget.h"
#include "../../Player/PopupComponent.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"

UOptionsWidget::UOptionsWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UOptionsWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 위젯이 처음 만들어질 때 한 번만 연결한다. (NativeConstruct는 화면에 붙을 때마다 불려서 두 번 연결될 수 있다)
	mWindowModeCombo->OnSelectionChanged.AddDynamic(this, &UOptionsWidget::OnWindowModeChanged);
	mApplyButton->OnClicked.AddDynamic(this, &UOptionsWidget::OnApplyClicked);
	mBackButton->OnClicked.AddDynamic(this, &UOptionsWidget::OnBackClicked);

	// 항목도 한 번만 넣는다. 항목 순서(인덱스)가 곧 엔진 값이라 순서를 바꾸면 안 된다.
	// WBP에서 넣지 않고 코드에서 넣는 것도 순서를 코드와 한곳에서 맞추기 위해서다.
	mWindowModeCombo->AddOption(TEXT("전체 화면"));		// 0 = EWindowMode::Fullscreen
	mWindowModeCombo->AddOption(TEXT("테두리 없는 창"));	// 1 = EWindowMode::WindowedFullscreen
	mWindowModeCombo->AddOption(TEXT("창 모드"));			// 2 = EWindowMode::Windowed

	mQualityCombo->AddOption(TEXT("낮음"));		// 0
	mQualityCombo->AddOption(TEXT("중간"));		// 1
	mQualityCombo->AddOption(TEXT("높음"));		// 2
	mQualityCombo->AddOption(TEXT("최고"));		// 3

	// 모니터가 지원하는 해상도. (높이가 r.MinYResolutionForUI = 720 미만인 것은 엔진이 뺀다)
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(mResolutionList);

	// ini에 적힌 지금 해상도가 지원 목록에 없으면 맨 앞에 더한다. 더하지 않으면 해상도 콤보박스가 빈 채로 열린다.
	if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
		const FIntPoint Current = Settings->GetScreenResolution();

		if (!mResolutionList.Contains(Current))
			mResolutionList.Insert(Current, 0);
	}

	for (const FIntPoint& Resolution : mResolutionList)
		mResolutionCombo->AddOption(FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y));

	// 처음에는 닫혀 있다. 이 함수는 게임 중에만 불려서 WBP 디자이너에서는 보이는 채로 편집할 수 있다.
	SetVisibility(ESlateVisibility::Collapsed);
}

void UOptionsWidget::Open()
{
	// 지난번에 고르고 적용하지 않은 값은 버리고 지금 설정을 다시 보여 준다.
	ReadSettings();

	// 자기 자신은 클릭을 받지 않고 자식만 받는다. 화면 전체를 덮는 반투명 배경(Dim, Visible)이 뒤의 메뉴 버튼 클릭을 막는다.
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UOptionsWidget::Close()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UOptionsWidget::ReadSettings()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();

	if (!Settings)
		return;

	// 창 모드 항목 순서를 EWindowMode 값과 맞춰 두어서 값이 곧 인덱스다.
	mWindowModeCombo->SetSelectedIndex(Settings->GetFullscreenMode());
	mResolutionCombo->SetSelectedIndex(mResolutionList.IndexOfByKey(Settings->GetScreenResolution()));

	// 항목별 품질이 섞여 있으면 -1이다. (4 = 시네마틱도 항목에 없다)
	// 그때는 아무것도 고르지 않고, 적용할 때도 품질은 바꾸지 않는다.
	const int32 Quality = Settings->GetOverallScalabilityLevel();

	if (Quality >= 0 && Quality < mQualityCombo->GetOptionCount())
		mQualityCombo->SetSelectedIndex(Quality);
	else
		mQualityCombo->ClearSelection();

	// SetSelectedIndex는 고른 항목이 바뀔 때만 OnSelectionChanged를 부른다. 같은 항목이면 오지 않으므로 직접 맞춘다.
	UpdateResolutionEnabled();
}

void UOptionsWidget::UpdateResolutionEnabled()
{
	// 테두리 없는 창은 엔진이 해상도 값을 무시하고 바탕화면 해상도를 쓴다. 골라도 소용없으므로 끈다.
	const bool IsBorderless = mWindowModeCombo->GetSelectedIndex() == EWindowMode::WindowedFullscreen;

	mResolutionCombo->SetIsEnabled(!IsBorderless);
}

void UOptionsWidget::OnWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdateResolutionEnabled();
}

void UOptionsWidget::OnApplyClicked()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();

	if (!Settings)
		return;

	const int32 WindowMode = mWindowModeCombo->GetSelectedIndex();
	const int32 ResolutionIndex = mResolutionCombo->GetSelectedIndex();
	const int32 Quality = mQualityCombo->GetSelectedIndex();

	// 항목 순서를 EWindowMode 값과 맞춰 두어서 인덱스를 그대로 바꾼다.
	if (WindowMode >= 0)
		Settings->SetFullscreenMode(EWindowMode::ConvertIntToWindowMode(WindowMode));

	// 테두리 없는 창은 엔진이 바탕화면 해상도를 쓰므로 넣지 않는다. (넣어도 무시된다)
	if (WindowMode != EWindowMode::WindowedFullscreen && mResolutionList.IsValidIndex(ResolutionIndex))
		Settings->SetScreenResolution(mResolutionList[ResolutionIndex]);

	// 품질은 골랐을 때만 바꾼다. 항목별로 섞여 있던 품질(고르지 않음 = -1)은 그대로 둔다.
	// 고르면 그림자, 텍스처, 후처리 등 Scalability 항목 전부가 같은 단계가 된다.
	if (Quality >= 0)
		Settings->SetOverallScalabilityLevel(Quality);

	// "이 화면을 유지할까요?" 확인 단계가 없으므로 바로 확정한다.
	// 확정한 값은 되돌리기(RevertVideoMode)의 기준이고 ini에도 적힌다. ApplySettings 안에서 저장하므로 그 전에 확정한다.
	Settings->ConfirmVideoMode();

	// 화면에 적용하고 GameUserSettings.ini에 저장한다.
	// false = 명령줄 인자(-windowed, -ResX 등)로 고른 값을 덮어쓰지 않는다.
	// 창은 닫지 않는다. 반투명 배경 뒤로 바뀐 품질을 바로 볼 수 있다.
	Settings->ApplySettings(false);
}

void UOptionsWidget::OnBackClicked()
{
	// 열 때와 같은 길(팝업 컴포넌트)로 닫는다.
	// 게임에서는 닫힌 뒤 커서 / 입력 모드를 컨트롤러가 다시 맞춰야 하는데, 컴포넌트를 거쳐야 컨트롤러가 알 수 있다(5단계).
	if (UPopupComponent* Popup = UPopupComponent::Get(GetOwningPlayer()))
		Popup->CloseOptions();
}
