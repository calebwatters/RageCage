// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

class USHealthComponent;
class USphereComponent;
class USoundCue;
UCLASS()
class RAGECAGE_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USHealthComponent* HealthComp;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USphereComponent* SphereComp;

	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	FVector GetNextPathPoint();

	FVector NextPathPoint;

	UPROPERTY(EditDefaultsOnly, Category = "Trackerbot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "Trackerbot")
	float RequiredDistanceToTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Trackerbot")
	bool bUseVelocityChange;

	UMaterialInstanceDynamic* MatInst;

	void SelfDestruct();

	UPROPERTY(EditDefaultsOnly, Category = "Trackerbot")
	UParticleSystem* ExplosionEffect;

	bool bExploded;

	bool bStartedSelfDestruction;

	UPROPERTY(EditDefaultsOnly, Category = "Trackerbot")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Trackerbot")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Trackerbot")
	float SelfDamageInterval;

	FTimerHandle TimeHandle_SelfDamage;

	void DamageSelf();

	UPROPERTY(EditDefaultsOnly, Category = "Trackerbot")
	USoundCue* SelfDestructSound;

	UPROPERTY(EditDefaultsOnly, Category = "Trackerbot")
	USoundCue* ExplodeSound;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

protected: 

	void OnCheckNearbyBots();

	int32 PowerLevel;
};
