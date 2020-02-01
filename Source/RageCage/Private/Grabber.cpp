// Fill out your copyright notice in the Description page of Project Settings.
#include "Grabber.h"
#include "DrawDebugHelpers.h"
#include "../Public/Grabber.h"
#include "Net/UnrealNetwork.h"
#define OUT
// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();

	// ...
	FindPhysicsHandleComponent();
	
}

void UGrabber::FindPhysicsHandleComponent() {
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s missing physics handle component"), *GetOwner()->GetName())
	}
}


void UGrabber::Grab() {
	if (GetOwnerRole() < ROLE_Authority) {
		ServerGrab();
	}
	bIsGrabbing = true;
	//Try and reach any actors with physics body collision channel set
	auto HitResult = GetFirstPhysicsBodyInReach();
	auto ComponentToGrab = HitResult.GetComponent();
	auto ActorHit = HitResult.GetActor();
	AActor* MyOwner = GetOwner();
	FName SocketName = "TelekinesisGripPoint";
	///If we hit something then atttach a physics handle
	if (!PhysicsHandle) { return; }
	if (ActorHit) {
		PhysicsHandle->GrabComponentAtLocation(
			ComponentToGrab,
			NAME_None,
			ComponentToGrab->GetOwner()->GetActorLocation()
		);
	}
}

void UGrabber::ServerGrab_Implementation()
{
	Grab();
}

bool UGrabber::ServerGrab_Validate()
{
	return true;
}

void UGrabber::Release()
{
	bIsGrabbing = false;
	if (!PhysicsHandle) { return; }
	PhysicsHandle->ReleaseComponent();
}

void UGrabber::ServerRelease_Implementation()
{
	Release();
}

bool UGrabber::ServerRelease_Validate()
{
	return true;
}

const FHitResult UGrabber::GetFirstPhysicsBodyInReach() {

	FCollisionQueryParams TraceParameters(FName(TEXT("")), false, GetOwner());
	//Ray-cast out to reach distance
	FHitResult Hit;
	GetWorld()->LineTraceSingleByObjectType(
		OUT Hit,
		GetReachLineStart(),
		GetReachLineEnd(),
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParameters
	);
	//See what we hit
	AActor* ActorHit = Hit.GetActor();
	if (ActorHit) {
		UE_LOG(LogTemp, Warning, TEXT("Line trace hit: %s"), *(ActorHit->GetName()))
	}
	return Hit;
}

// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PhysicsHandle) { return; }

	if (PhysicsHandle->GrabbedComponent)
	{
		PhysicsHandle->SetTargetLocation(GetReachLineEnd());
	}

}

FVector UGrabber::GetReachLineEnd() {
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);
	return PlayerViewPointLocation + PlayerViewPointRotation.Vector() * Reach;
}

FVector UGrabber::GetReachLineStart() {
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);
	return PlayerViewPointLocation;
}

void UGrabber::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGrabber, bIsGrabbing);
}