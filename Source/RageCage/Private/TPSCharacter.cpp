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

}

// Called when the game starts or when spawned
void ATPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	HealthComp->OnHealthChanged.AddDynamic(this, &ATPSCharacter::OnHealthChanged);

	DefaultFOV = CameraComp->FieldOfView;
	if (GetLocalRole() == ROLE_Authority) {
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
	if (CurrentWeapon && !bIsUsingTelekinesis && !bMeleeing) {
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

	//if (MatInst == nullptr)
	//{
	//	MatInst = GetMesh()->CreateAndSetMaterialInstanceDynamicFromMaterial(1, GetMesh()->GetMaterial(1));
	//}
	//if (MatInst)
	//{
	//	MatInst->SetScalarParameterValue("HealthPickupAlpha", 0.1f);
	//}
}

void ATPSCharacter::Melee()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerMelee();
	}
	if (CurrentWeapon)
	{
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Grip_PointL");
	}
	bMeleeing = true;
	OnRep_Meleed();
}

void ATPSCharacter::OnRep_Meleed()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && MeleeMontage)
	{
		AnimInstance->Montage_Play(MeleeMontage, 1.35f);
	}
	GetWorld()->GetTimerManager().SetTimer(_loopTimerHandle, this, &ATPSCharacter::MeleeWeaponReattach, 0.5f, false);
}


void ATPSCharacter::ServerMelee_Implementation()
{
	Melee();
}

bool ATPSCharacter::ServerMelee_Validate()
{
	return true;
}


void ATPSCharacter::StopMelee()
{
	bMeleeing = false;
}

void ATPSCharacter::MeleeWeaponReattach()
{
	if (CurrentWeapon)
	{
	CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Grip_Point"); 
	}
}

// Called every frame
void ATPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ATPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

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
}