// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPSCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USHealthComponent;
class UGrabber;
class APrimaryWeapon;
class USHealthComponent;
class UAnimMontage;

UCLASS()
class RAGECAGE_API ATPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATPSCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveRight(float Value);

	void MoveForward(float Value);

	void BeginZoom();

	void EndZoom();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	UGrabber* GrabberComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComp;

	UPROPERTY(ReplicatedUsing=OnRep_BeginZoom, EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool bIsAiming;

	UFUNCTION()
	void OnRep_BeginZoom();

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool bWantsToZoom;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterpSpeed;

	float DefaultFOV;

	UFUNCTION(BlueprintCallable, Category = "Telekinesis")
	void Grab();

	UFUNCTION(BlueprintCallable, Category = "Telekinesis")
	void Release();

	UPROPERTY(Replicated, BlueprintReadWrite)
	APrimaryWeapon* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<APrimaryWeapon> StarterWeaponClass;

	void StartFire();

	void StopFire();

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bDied;

	UPROPERTY(ReplicatedUsing=OnRep_Meleed, VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bMeleeing;

	UFUNCTION()
	void OnRep_Meleed();

	void Melee();

	void StopMelee();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
		UAnimMontage* MeleeMontage;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;
	
};
