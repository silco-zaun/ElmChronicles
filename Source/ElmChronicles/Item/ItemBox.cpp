// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBox.h"
#include "../Characters/PlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

// Sets default values
AItemBox::AItemBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	mMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	SetRootComponent(mBox);

	mMesh->SetupAttachment(mBox);

	static ConstructorHelpers::FObjectFinder<UStaticMesh>
		BoxMesh(TEXT("/Script/Engine.StaticMesh'/Game/PurePoly/FreeLowPolyFantasyRPGWeapons/Meshes/SM_PP_Theme_04_Spellbook_003.SM_PP_Theme_04_Spellbook_003'"));

	if (BoxMesh.Succeeded())
		mMesh->SetStaticMesh(BoxMesh.Object);
	else
	{
		UE_LOG(LogTestDebug, Warning, TEXT("Find BoxMesh Failed."));
	}

	mBox->SetCollisionProfileName(TEXT("ItemBox"));

	mMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	mMesh->SetRelativeLocation(FVector(0.0, 13.0, -17.5));

	mBox->SetBoxExtent(FVector(10.0, 15.0, 20.0));
}

// Called when the game starts or when spawned
void AItemBox::BeginPlay()
{
	Super::BeginPlay();
	
	mBox->OnComponentBeginOverlap.AddDynamic(this, &AItemBox::ItemOverlap);
}

// Called every frame
void AItemBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemBox::ItemOverlap(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
	bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);

	if (IsValid(PlayerCharacter))
	{
		if (PlayerCharacter->AddInventoryItem(mDropItemKey))
			Destroy();
	}
}

