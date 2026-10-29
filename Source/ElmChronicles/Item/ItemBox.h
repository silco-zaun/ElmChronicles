// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "../Data/Tables/ItemTableRow.h"
#include "GameFramework/Actor.h"
#include "ItemBox.generated.h"

class UBoxComponent;

UCLASS()
class ELMCHRONICLES_API AItemBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemBox();

protected:
	UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> mBox;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> mMesh;
	UPROPERTY(VisibleAnywhere) FName mDropItemKey;

public:
	void SetDropItemKey(FName Key)
	{
		mDropItemKey = Key;
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION()
	void ItemOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);
};
