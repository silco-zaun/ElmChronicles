// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterPreviewActor.h"
#include "../../Characters/ElmCharacterBase.h"
#include "Components/LightComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/TextureRenderTarget2D.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// AElmCharacterBase의 장비 메시 컴포넌트 수 (mMainhand, mOffhand, mHeadGear, mBackGear)
	constexpr int32 GearCount = 4;

	// 미리보기 메시 공통 설정
	void SetupPreviewPrimitive(UPrimitiveComponent* Primitive)
	{
		Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// SceneCapture에만 그린다. 메인 카메라에는 보이지 않는다.
		Primitive->bVisibleInSceneCaptureOnly = true;
		// 투구 챙 아래 얼굴, 팔 아래 몸통 같은 자기 그림자가 입체감을 만든다.
		// 캡처 전용 메시라 메인 화면에는 그림자를 드리우지 않는다. (엔진 FPrimitiveSceneProxy::IsShadowCast)
		Primitive->CastShadow = true;
	}
}

ACharacterPreviewActor::ACharacterPreviewActor()
{
	// 액터는 움직이지 않는다. (SceneCapture 컴포넌트는 따로 Tick하면서 찍는다)
	PrimaryActorTick.bCanEverTick = false;

	mRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(mRoot);

	// 플레이어처럼 -90도 돌려 +X 방향을 보게 한다. 발이 액터 원점에 온다.
	mBody = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Body"));
	mBody->SetupAttachment(mRoot);
	mBody->SetRelativeRotation(FRotator(0.0, -90.0, 0.0));
	// 메인 카메라에 그려지지 않는 메시는 기본 설정이면 뼈 갱신을 건너뛸 수 있다. 미리보기는 항상 움직이게 한다.
	mBody->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	SetupPreviewPrimitive(mBody);

	// 장비 메시. 붙일 소켓과 위치는 CopyAppearance가 플레이어 것을 그대로 옮긴다.
	for (int32 i = 0; i < GearCount; ++i)
	{
		UStaticMeshComponent* Gear = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Gear%d"), i));
		Gear->SetupAttachment(mBody);
		SetupPreviewPrimitive(Gear);
		mGearArray.Add(Gear);
	}

	// 카메라는 캐릭터 정면(+X)에 서서 뒤돌아(Yaw 180) 캐릭터를 본다. FOV를 좁게 해서 원근 왜곡을 줄인다.
	mCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Capture"));
	mCapture->SetupAttachment(mRoot);
	mCapture->SetRelativeLocationAndRotation(FVector(560.0, 0.0, 120.0), FRotator(0.0, 180.0, 0.0));
	mCapture->FOVAngle = 30.f;
	// RGB = 장면 색, A = 1 - 불투명도. 아무것도 없는 배경은 A = 1이라 머티리얼에서 뒤집어 투명하게 만든다.
	mCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	// 이 액터의 컴포넌트만 찍는다. 목록은 BeginPlay에서 채운다.
	mCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	// 창이 열릴 때만 켠다(SetCaptureActive).
	mCapture->bCaptureEveryFrame = false;
	mCapture->bCaptureOnMovement = false;
	// 맵의 안개와 하늘(대기)은 미리보기 색을 바꾸지 않게 끈다.
	// 스카이라이트는 켜 둔다. 게임 화면처럼 그림자 쪽을 하늘빛으로 밝힌다.
	mCapture->ShowFlags.SetFog(false);
	mCapture->ShowFlags.SetAtmosphere(false);

	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> RenderTarget(
		TEXT("/Script/Engine.TextureRenderTarget2D'/Game/UI/Game/RT_CharacterPreview.RT_CharacterPreview'"));

	if (RenderTarget.Succeeded())
		mCapture->TextureTarget = RenderTarget.Object;
}

void ACharacterPreviewActor::BeginPlay()
{
	Super::BeginPlay();

	// 생성자에서 this를 넣으면 CDO 포인터가 기본값으로 들어가므로, 실제 인스턴스가 된 뒤에 넣는다.
	mCapture->ShowOnlyActors.Add(this);

	// 레벨의 태양(Directional Light) 쪽으로 몸을 돌린다. 액터째 돌리므로 앞에 선 카메라도 함께 돈다.
	// 해의 방향은 맵마다 다르고, 그대로 두면 Field에서는 해가 등 뒤에 있어 얼굴이 그림자에 묻힌다.
	for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
	{
		const ULightComponent* Sun = It->GetLightComponent();

		if (!Sun)
			continue;

		// 빛이 오는 쪽 = 빛이 나아가는 방향(Forward)의 반대
		const double SunYaw = (-Sun->GetForwardVector()).Rotation().Yaw;

		// 해를 정면으로 받으면 얼굴이 납작해 보여서 35도 비스듬히(화면 왼쪽 위에서) 받게 한다.
		SetActorRotation(FRotator(0.0, SunYaw - 35.0, 0.0));
		break;
	}
}

void ACharacterPreviewActor::CopyAppearance(const AElmCharacterBase* Source)
{
	if (!Source)
		return;

	USkeletalMeshComponent* SourceBody = Source->GetMesh();

	// 몸 메시를 먼저, ABP를 나중에 정한다(InitAsset과 같은 순서). 같은 것이면 다시 넣지 않는다.
	// 다시 넣으면 ABP가 처음부터 시작해서 장비를 바꿀 때마다 Idle이 튄다.
	// ABP(UPlayerAnimInstance)는 주인이 Pawn이 아니면 이동 값을 읽지 않으므로 속도 0 = Idle로 서 있다.
	if (mBody->GetSkeletalMeshAsset() != SourceBody->GetSkeletalMeshAsset())
		mBody->SetSkeletalMeshAsset(SourceBody->GetSkeletalMeshAsset());

	if (mBody->GetAnimClass() != SourceBody->GetAnimClass())
		mBody->SetAnimInstanceClass(SourceBody->GetAnimClass());

	// 장비 메시는 소켓과 상대 위치까지 그대로 옮긴다(모자 / 망토는 AttachBindPoseGear가 계산한 위치다).
	// 보이기 상태도 따라가므로, 무기칸이 비어 숨긴 무기는 미리보기에서도 숨는다.
	const TArray<UStaticMeshComponent*> SourceGears = Source->GetGearComponents();

	for (int32 i = 0; i < mGearArray.Num() && i < SourceGears.Num(); ++i)
	{
		const UStaticMeshComponent* SourceGear = SourceGears[i];
		UStaticMeshComponent* Gear = mGearArray[i];

		Gear->SetStaticMesh(SourceGear->GetStaticMesh());
		Gear->AttachToComponent(mBody, FAttachmentTransformRules::KeepRelativeTransform, SourceGear->GetAttachSocketName());
		Gear->SetRelativeTransform(SourceGear->GetRelativeTransform());
		Gear->SetVisibility(SourceGear->IsVisible());
	}
}

void ACharacterPreviewActor::SetCaptureActive(bool Active)
{
	// 매 프레임 장면을 한 번 더 그리는 일이라, 창이 닫혀 있는 동안에는 찍지 않는다.
	mCapture->bCaptureEveryFrame = Active;
}
