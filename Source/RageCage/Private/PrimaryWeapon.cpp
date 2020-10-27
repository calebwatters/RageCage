// Fill out your copyright notice in the Description page of Project Settings.

#include "PrimaryWeapon.h"
#include "../Public/PrimaryWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "RageCage.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"
#include "GameFramework/ForceFeedbackEffect.h"
#include "GameFramework/PlayerController.h"

static int32 DebugWeaponDrawing = 0;
 FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapons"), DebugWeaponDrawing, TEXT("Draw Debug Lines for Weapons"), ECVF_Cheat);


// Sets default values
APrimaryWeapon::APrimaryWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";
	BaseDamage = 20.0f;
	BulletSpread = 2.0f;
	RateOfFire = 0.5f;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

// Called when the game starts or when spawned
void APrimaryWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void APrimaryWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	if (SelectedEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void APrimaryWeapon::Fire()
{
	// Trace the world, from pawn eyes to crossshair location
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerFire();
	}

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	PlayerController->ClientPlayForceFeedback(ControllerFeedback, false, false, NAME_None);
	UGameplayStatics::SpawnSoundAttached(FireSound, RootComponent);

	//if (Role == ROLE_Authority) {
	//	FActorSpawnParameters SpawnParams;
	//	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	//	WeaponShell = GetWorld()->SpawnActor<AActor>(SetWeaponShell, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	//	if (WeaponShell)
	//	{
	//		WeaponShell->SetOwner(this);
	//		WeaponShell->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "BulletSocket");
	//	}
	//}

	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		//Bullet Spread
		float HalfRad = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);


		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		//Particle "Target" param
		FVector TracerEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType = SurfaceType_Default;
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			//Blocking hit. Process Damage
			AActor* HitActor = Hit.GetActor();
			
			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float ActualDamage = BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.0f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);

			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

			TracerEndPoint = Hit.ImpactPoint;

			HitScanTrace.SurfaceType = SurfaceType;
		}

		if (DebugWeaponDrawing > 0) 
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}

		if (MuzzleEffect) 
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
		}

		if (TracerEffect) {
			FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

			UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
			if (TracerComp)
			{
				TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
			}
		}

		AActor* MyOwner = Cast<APawn>(GetOwner());
		if (MyOwner)
		{
			APlayerController* PC = Cast<APlayerController>(MyOwner->GetInstigatorController());
			if (PC)
			{
				PC->ClientPlayCameraShake(FireCamShake);
			}
		}
		if (GetLocalRole() == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}
	}

}

void APrimaryWeapon::ServerFire_Implementation()
{
	Fire();
}

bool APrimaryWeapon::ServerFire_Validate()
{
	return true;
}

void APrimaryWeapon::OnRep_HitScanTrace()
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect) {
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, HitScanTrace.TraceTo);
		}
	}

	AActor* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetInstigatorController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}

	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void APrimaryWeapon::StartFire()
{
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &APrimaryWeapon::Fire, RateOfFire, true, 0.0f);
}

void APrimaryWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void APrimaryWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APrimaryWeapon, HitScanTrace, COND_SkipOwner);
}
