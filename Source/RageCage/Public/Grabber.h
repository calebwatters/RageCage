// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Grabber.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RAGECAGE_API UGrabber : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrabber();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void FindPhysicsHandleComponent();

	const FHitResult GetFirstPhysicsBodyInReach();
	//Return end of reach line
	FVector GetReachLineEnd();

	FVector GetReachLineStart();

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool bIsGrabbing;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, Category = "Telekinesis")
	float Reach;

	UPhysicsHandleComponent* PhysicsHandle = nullptr;
	//Ray-cast and grab in reach
	UFUNCTION()
	void Grab();

	UFUNCTION()
	void Release();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerGrab();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRelease();

};
