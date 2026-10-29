// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "../AbilitySystem/Components/PlayerAbilitySystemComponent.h"
#include "../AbilitySystem/Attributes/PlayerAttributeSet.h"
#include "../AbilitySystem/Abilities/GameplayAbility_Melee.h"
#include "../AI/TeamTypes.h"
#include "../Animations/AnimInstances/PlayerAnimInstance.h"
#include "../Data/Tables/LevelStatTableRow.h"
#include "../Data/Tables/PlayerCharacterTableRow.h"
#include "../Input/DefaultInput.h"
#include "../Player/ElmPlayerState.h"
#include "../Player/InventoryComponent.h"
#include "../Save/ElmSaveGame.h"
#include "../Subsystem/AssetSubsystem.h"
#include "../Subsystem/SaveSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/GameInstance.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	UE_LOG(LogTestOrder, Log, TEXT("APlayerCharacter::Constructor - %s"), *GetName());

	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 외형(메시, ABP, 무기, 캡슐)과 능력치는 DT_Player의 이 행에서 읽는다. (InitAttributes)
	mRowName = TEXT("Kyle");

	mSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	mCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	//mHPBarBillboardWidgetComponent = CreateDefaultSubobject<UBillboardWidgetComponent>(TEXT("HPBarBillboardWidgetComponent"));

	mSpringArm->SetupAttachment(GetMesh());
	mCamera->SetupAttachment(mSpringArm);
	mSpringArm->TargetArmLength = 500.f;
	mSpringArm->bUsePawnControlRotation = true;
	// 메시 원점(발)에서 250 위를 중심으로 돈다.
	// 회전은 bUsePawnControlRotation이 컨트롤러 회전으로 덮어쓰므로 정하지 않는다.
	mSpringArm->SetRelativeLocation(FVector(0.0, 0.0, 250.0));
	//mHPBarBillboardWidgetComponent->SetupAttachment(GetMesh(), TEXT("HealthBar"));

	// KayKit 메시는 +Y를 정면으로 만들어져 있어서 액터 정면(+X)에 맞게 돌린다.
	// 높이(Z)는 InitAsset이 캡슐 크기에 맞춰 정한다.
	GetMesh()->SetRelativeRotation(FRotator(0.0, -90.0, 0.0));

	mInventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	// 선택1. 마우스를 돌리면 캐릭터 몸도 즉시 같이 돎(FPS / TPS 슈터 스타일)
	//bUseControllerRotationYaw = true;
	//GetCharacterMovement()->bOrientRotationToMovement = false;

	// 선택2. 카메라만 자유롭게 돌고, 캐릭터는 이동하는 방향으로 부드럽게 돎 (액션 RPG 스타일)
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// 충돌 설정
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// 캐릭터는 모두 Pawn이다. 아군 / 적 구분은 팀(mTeam)이 한다.
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

	mTeam = ETeamType::Player;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTestOrder, Log,
		TEXT("APlayerCharacter::BeginPlay - %s / Controller : %s / PlayerState : %s"),
		*GetName(),
		IsValid(GetController()) ? *GetController()->GetName() : TEXT("null"),
		IsValid(GetPlayerState<AElmPlayerState>()) ? TEXT("Valid") : TEXT("null"));

	// InputMappingContext를 지정한다.
	// PlayerController를 얻어온다.
	// Pawn종류는 자신에게 빙의된 컨트롤러를 반환하는 기능을 제공한다.
	// 언리얼엔진에서는 Cast 함수를 이용해서 UObject에 대한 형변환을
	// 진행한다.
	TObjectPtr<APlayerController> PlayerController = Cast<APlayerController>(GetController());

	// UObject타입 객체가 유효한 객체인지를 체크한다.
	if (IsValid(PlayerController))
	{
		// PlayerController가 가지고 있는 InputSystem을 언어온다.
		TObjectPtr<UEnhancedInputLocalPlayerSubsystem> EnhancedInputLocalPlayerSubsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
				PlayerController->GetLocalPlayer());

		// InputMappingContext를 가지고 있는 CDO를 얻어온다.
		const UDefaultInput* DefaultInputCDO = GetDefault<UDefaultInput>();

		// InputMappingContext를 InputSystem에 등록한다.
		EnhancedInputLocalPlayerSubsystem->AddMappingContext(DefaultInputCDO->mInputMappingContext, 0);
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// InputConponent에 InputAction을 등록하고 해당 inputAction이 동작될때
	// 호출할 함수를 등록한다.
	// UInputComponent 타입을 UEnhancedInputComponent 타입으로 변환한다.
	TObjectPtr<UEnhancedInputComponent> PlayerEnhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (IsValid(PlayerEnhancedInputComponent))
	{
		// InputAction을 가지고 있는 CDO를 얻어온다.
		const UDefaultInput* DefaultInputCDO = GetDefault<UDefaultInput>();

		// 1번인자 : 바인딩할 InputAction을 지정한다.
		// 2번인자 : 이 Action에 저장된 Key가 어떻게 반응할지 지정한다.
		// 3번인자 : 이 Action이 동작할 때 호출할 함수를 가지고 있는 UObject
		// 객체를 지정한다.
		// 4번인자 : 이 Action이 동작할 때 호출할 함수의 주소를 등록한다.
		PlayerEnhancedInputComponent->BindAction(DefaultInputCDO->FindInputAction(TEXT("Move")),
			ETriggerEvent::Triggered, this, &APlayerCharacter::MoveKey);

		PlayerEnhancedInputComponent->BindAction(DefaultInputCDO->FindInputAction(TEXT("Look")),
			ETriggerEvent::Triggered, this, &APlayerCharacter::LookKey);

		PlayerEnhancedInputComponent->BindAction(DefaultInputCDO->FindInputAction(TEXT("Jump")),
			ETriggerEvent::Started, this, &APlayerCharacter::JumpKey);

		PlayerEnhancedInputComponent->BindAction(DefaultInputCDO->FindInputAction(TEXT("Attack")),
			ETriggerEvent::Started, this, &APlayerCharacter::AttackKey);
	}
}

UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	return mASC.Get();
}

// 서버(싱글플레이 포함)에서 호출된다.
void APlayerCharacter::PossessedBy(AController* NewController)
{
	// Super 안에서 Controller의 PlayerState가 Pawn에 연결된다.
	Super::PossessedBy(NewController);

	InitAbilitySystem();
}

// 클라이언트에서 PlayerState가 복제되어 왔을 때 호출된다. (멀티플레이용)
void APlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitAbilitySystem();
}

void APlayerCharacter::MoveKey(const FInputActionValue& Value)
{
	FVector2D Axis = Value.Get<FVector2D>();

	// Pitch는 빼고 Yaw만 사용(위를 볼 때 앞으로 가면 땅에 박히지 않도록)
	FRotator YawRotation(0.0, GetControlRotation().Yaw, 0.0);

	FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Axis.X);
	AddMovementInput(Right, Axis.Y);
}

// 1. 회전키 Action을 하나 더 만든다.
// 2. 마우스를 왼쪽, 오른쪽으로 움직이거나 위, 아래로 움직일 때 반응한다.
// 3. 마우스를 왼쪽, 오른쪽으로 움직일 때 SpringArm을 회전시킨다.
void APlayerCharacter::LookKey(const FInputActionValue& Value)
{
	FVector2D Axis = Value.Get<FVector2D>() * mMouseSensitivity;

	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void APlayerCharacter::JumpKey(const FInputActionValue& Value)
{
	if (CanJump())
		Jump();
}

void APlayerCharacter::AttackKey(const FInputActionValue& Value)
{
	// 비용·쿨다운·State.Dead·콤보 구간 검사는 모두 어빌리티가 한다.
	// 테이블이 로드되기 전(mAttackAbility가 비어 있을 때)에는 아무 일도 하지 않는다.
	if (mASC.IsValid() && mAttackAbility)
		mASC->TryActivateAbilityByClass(mAttackAbility);
}

void APlayerCharacter::Death()
{
	//Destroy();
}

void APlayerCharacter::InitAbilitySystem()
{
	AElmPlayerState* State = GetPlayerState<AElmPlayerState>();

	if (!IsValid(State))
		return;

	UPlayerAbilitySystemComponent* ASC = State->GetPlayerAbilitySystemComponent();

	// OwnerActor = PlayerState, AvatarActor = 캐릭터
	ASC->InitAbilityActorInfo(State, this);

	mASC = ASC;

	// 이동속도 / 피격 / 사망 델리게이트 연결. (부모 공통 처리, 재빙의 때 중복 연결도 막아준다)
	BindAttributeDelegates(ASC, State->GetAttributeSet());

	if (UAssetSubsystem* AssetSubsystem = GetGameInstance()->GetSubsystem<UAssetSubsystem>())
		AssetSubsystem->CallOrRegisterDataLoaded(this, &APlayerCharacter::InitAttributes);
}

void APlayerCharacter::InitAttributes()
{
	UAssetSubsystem* AssetSubsystem = GetGameInstance()->GetSubsystem<UAssetSubsystem>();

	if (!AssetSubsystem || !AssetSubsystem->IsDataLoaded())
		return;

	const FPlayerCharacterTableRow* Info = AssetSubsystem->FindRow<FPlayerCharacterTableRow>(mRowName);

	if (!Info)
		return;

	// 외형은 눈에 보이는 부분이므로 서버와 클라이언트 모두 적용한다.
	InitAsset(*Info);

	// 기본 공격 어빌리티. 테이블에서 비워두면 근접 공격(Melee)을 쓴다. 몽타주 섹션이 여러 개면 콤보가 된다.
	// 입력(AttackKey)이 이 클래스로 실행하므로 서버와 클라이언트 모두 정한다.
	mAttackAbility = Info->AttackAbility.LoadSynchronous();

	if (!mAttackAbility)
		mAttackAbility = UGameplayAbility_Melee::StaticClass();

	// 능력치와 어빌리티 부여는 서버에서만 한다.
	if (!HasAuthority() || !mASC.IsValid())
		return;

	// ASC는 PlayerState 소유라 다시 빙의해도 어빌리티가 남아 있다. 없을 때만 준다.
	if (!mASC->FindAbilitySpecFromClass(mAttackAbility))
		mASC->GiveAbility(FGameplayAbilitySpec(mAttackAbility, 1));

	// 이전 레벨에서 넘어온 메모. nullptr이면 새 게임이다.
	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	const UElmSaveGame* Save = SaveSubsystem ? SaveSubsystem->GetSaveGame() : nullptr;

	// 새 게임이면 DT_Player의 시작 레벨, 이어하기면 메모의 레벨로 레벨 스탯을 찾는다. (DT_LevelStat 행 이름 = "1" ~ "10")
	const int32 PlayerLevel = Save ? Save->Level : Info->Level;
	const FLevelStatTableRow* Stat =
		AssetSubsystem->FindRow<FLevelStatTableRow>(FName(*FString::FromInt(PlayerLevel)));

	if (!Stat)
		return;

	// 레벨 스탯. Max 값을 먼저 넣고 HP / MP를 가득 채운다.
	mASC->InitLevelStats(PlayerLevel, *Stat);
	mASC->InitCombatStats(Info->MoveSpeed, Info->AttackSpeed, Info->AttackRange, Info->AttackRadius);

	if (!Save)
		return;

	// 이어하기면 가득 찬 HP / MP를 메모 값으로 덮는다.
	// MaxHP / MaxMP가 위에서 먼저 들어가 있어야 PreAttributeBaseChange의 Clamp에 0으로 잘리지 않는다.
	mASC->SetNumericAttributeBase(UBaseAttributeSet::GetHPAttribute(), Save->HP);
	mASC->SetNumericAttributeBase(UBaseAttributeSet::GetMPAttribute(), Save->MP);
	mASC->SetNumericAttributeBase(UPlayerAttributeSet::GetExpAttribute(), Save->Exp);
	mASC->SetNumericAttributeBase(UPlayerAttributeSet::GetGoldAttribute(), Save->Gold);
}

void APlayerCharacter::OnDamagedBy(AActor* Causer, float Damage)
{
	UPlayerAnimInstance* Anim = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());

	if (IsValid(Anim))
		Anim->PlayHit();
}

void APlayerCharacter::OnKilledBy(AActor* Killer)
{
	// State.Dead 태그와 어빌리티 취소는 부모(ElmCharacterBase)가 이미 했다.
	// 조작을 막고 제자리에 멈춘다.
	if (APlayerController* PlayerController = GetController<APlayerController>())
		DisableInput(PlayerController);

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	UE_LOG(LogTestDebug, Warning, TEXT("Player Death - Killer : %s"), *GetNameSafe(Killer));
}

bool APlayerCharacter::AddInventoryItem(FName ItemKey)
{
	return mInventory->AddItem(ItemKey);
}

void APlayerCharacter::WriteToSave(UElmSaveGame& Save) const
{
	// 가방은 칸 번호까지 적는다.
	mInventory->WriteToSave(Save.Inventory);

	// 능력치는 PlayerState의 ASC에 있다. 레벨을 옮기면 PlayerState째로 사라지므로 숫자만 적어 둔다.
	// 읽을 때 SetNumericAttributeBase로 넣으므로 적을 때도 Base 값을 읽는다. (지속형 GE의 버프가 섞이지 않는다)
	if (!mASC.IsValid())
		return;

	// Level Attribute는 float라 2.9999 같은 값이 나올 수 있어서 버리지 않고 반올림한다.
	Save.Level = FMath::RoundToInt(mASC->GetNumericAttributeBase(UBaseAttributeSet::GetLevelAttribute()));
	Save.HP = mASC->GetNumericAttributeBase(UBaseAttributeSet::GetHPAttribute());
	Save.MP = mASC->GetNumericAttributeBase(UBaseAttributeSet::GetMPAttribute());
	Save.Exp = mASC->GetNumericAttributeBase(UPlayerAttributeSet::GetExpAttribute());
	Save.Gold = mASC->GetNumericAttributeBase(UPlayerAttributeSet::GetGoldAttribute());
}
