// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AmmoComponent.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChangedSignature, UAmmoComponent*, AmmoComponent, float, AmmoCount, float, AmmoCountDelta);
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RAGECAGE_API UAmmoComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAmmoComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//UPROPERTY(ReplicatedUsing = OnRep_Ammo, EditDefaultsOnly, BlueprintReadOnly, Category = "AmmoComponent")
	//float AmmoCount;

	//UFUNCTION()
	//void OnRep_Ammo(float OldAmmo);

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoComponent")
	//float DefaultAmmo;

	//UFUNCTION()
	//void HandleUsedAmmo(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//UPROPERTY(BlueprintAssignable, Category = "Events")
	//FOnAmmoChangedSignature OnAmmoChanged;
};
