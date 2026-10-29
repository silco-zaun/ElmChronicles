// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "../Input/ElmInputConfig.h"
#include "ElmCharacterBase.h"
#include "PlayerCharacter.generated.h"

struct FItemTableRow;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class ELMCHRONICLES_API APlayerCharacter : public AElmCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> mSpringArm;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> mCamera;
	UPROPERTY(VisibleAnywhere) TObjectPtr<class UInventoryComponent> mInventory;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")	float mMouseSensitivity = 0.5f;
	// DT_Player 행 이름. 생성자에서 정하고, 외형과 능력치를 이 행에서 읽는다.
	UPROPERTY(EditAnywhere, Category = "Player") FName mRowName;

	// 소유는 PlayerState가 한다. 캐릭터는 참조만 한다.
	TWeakObjectPtr<class UPlayerAbilitySystemComponent> mASC;
	// 기본 공격 어빌리티. DT_Player의 AttackAbility이고, 비어 있으면 Melee다.
	// 테이블에서 Soft로 불러온 BP 어빌리티가 GC되지 않도록 UPROPERTY로 잡아 둔다.
	UPROPERTY()	TSubclassOf<class UElmGameplayAbility> mAttackAbility;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

private:
	void MoveKey(const FInputActionValue& Value);
	void LookKey(const FInputActionValue& Value);
	void JumpKey(const FInputActionValue& Value);
	void AttackKey(const FInputActionValue& Value);

public:
	virtual void Death();

protected:
	virtual void OnDamagedBy(AActor* Causer, float Damage) override;
	virtual void OnKilledBy(AActor* Killer) override;

private:
	void InitAbilitySystem();
	// DT_Player 행으로 외형과 기본 공격(서버 / 클라 모두),
	// 능력치(서버만)를 적용한다. 테이블이 로드된 뒤에 불린다.
	void InitAttributes();

public:
	bool AddInventoryItem(FName ItemKey);
	// 레벨 이동 / 저장 직전에 지금 상태를 메모에 적는다. (읽기는 각 부품이 자기 초기화 때 한다)
	void WriteToSave(class UElmSaveGame& Save) const;
};
