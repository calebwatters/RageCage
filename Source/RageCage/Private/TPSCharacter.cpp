// Fill out your copyright notice in the Description page of Project Settings.

#include "TPSCharacter.h"
#include "../Public/TPSCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SHealthComponent.h"
#include "Public/Grabber.h"
#include "Public/PrimaryWeapon.h"
#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
// Sets default values
ATPSCharacter::ATPSCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateAbstractDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	GrabberComp = CreateDefaultSubobject<UGrabber>(TEXT("GrabberComp"));

	ZoomedFOV = 65.0f;
	ZoomInterpSpeed = 20.0f;

}

// Called when the game starts or when spawned
void ATPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	HealthComp->OnHealthChanged.AddDynamic(this, &ATPSCharacter::OnHealthChanged);

	DefaultFOV = CameraComp->FieldOfView;
	if (Role == ROLE_Authority) {
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CurrentWeapon = GetWorld()->SpawnActor<APrimaryWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Grip_Point");
		}
	}
}



void ATPSCharacter::BeginZoom()
{
	OnRep_BeginZoom();
}

void ATPSCharacter::EndZoom()
{
	bWantsToZoom = false;
	bIsAiming = false;
}

void ATPSCharacter::OnRep_BeginZoom()
{
	bWantsToZoom = true;
	bIsAiming = true;
}

void ATPSCharacter::Grab()
{
	GrabberComp->Grab();
}

void ATPSCharacter::Release()
{
	GrabberComp->Release();
}

void ATPSCharacter::StartFire()
{
	if (CurrentWeapon) {
		CurrentWeapon->StartFire();
	}
}

void ATPSCharacter::StopFire()
{
	if (CurrentWeapon) {
		CurrentWeapon->StopFire();
	}
}

void ATPSCharacter::OnHealthChanged(USHealthComponent* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied) {
		//Die
		bDied = true;

	}
}

void ATPSCharacter::OnRep_Meleed()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && MeleeMontage)
	{
		AnimInstance->Montage_Play(MeleeMontage, 1.35f);
	}
}

void ATPSCharacter::Melee()
{
	bMeleeing = true;
	OnRep_Meleed();
}

void ATPSCharacter::StopMelee()
{
	bMeleeing = false;
}


// Called every frame
void ATPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;

	float NewFOV = FMath::FInterpConstantTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

	CameraComp->SetFieldOfView(NewFOV);
}

// Called to bind functionality to input
void ATPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ATPSCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ATPSCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATPSCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ATPSCharacter::StopFire);

	PlayerInputComponent->BindAction("Melee", IE_Pressed, this, &ATPSCharacter::Melee);
	PlayerInputComponent->BindAction("Melee", IE_Released, this, &ATPSCharacter::StopMelee);
}

FVector ATPSCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}
	return Super::GetPawnViewLocation();
}

void ATPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPSCharacter, CurrentWeapon);
	DOREPLIFETIME(ATPSCharacter, bDied);
	DOREPLIFETIME(ATPSCharacter, bMeleeing);
	DOREPLIFETIME(ATPSCharacter, bWantsToZoom);
}