// Fill out your copyright notice in the Description page of Project Settings.

#include "CarnievilPrototype.h"
#include "CameraVolume.h"
#include "CarnievilPrototypeCharacter.h"
#include "CarnievilPrototypeGameMode.h"
#include "Engine.h"


// Sets default values
ACameraVolume::ACameraVolume()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Sets default box size
	BoxExtent.Set(100, 100, 100);

	// Sets up collider
	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	BoxCollider->InitBoxExtent(BoxExtent);
	BoxCollider->AttachTo(RootComponent);

	// Sets up function called on collision
	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &ACameraVolume::OnOverlapBegin);
}

// Called when the game starts or when spawned
void ACameraVolume::BeginPlay()
{
	Super::BeginPlay();
	BoxCollider->SetCollisionProfileName(TEXT("CameraVolume")); // Stops it from colliding
	
}

// Called every frame
void ACameraVolume::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Player enters volume
void ACameraVolume::OnOverlapBegin(class AActor* OtherActor, 
									class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
										bool bFromSweep, const FHitResult& SweepResult)
{
	// Get player character
	ACarnievilPrototypeCharacter* player =
		Cast<ACarnievilPrototypeCharacter>
		(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (OtherActor && (OtherActor == player) && OtherComp) // If the actor is the player
	{

		// Get player controller
		APlayerController* controller =
			UGameplayStatics::GetPlayerController(GetWorld(), 0);

		// Get game mode
		ACarnievilPrototypeGameMode* gm = Cast<ACarnievilPrototypeGameMode>
			(UGameplayStatics::GetGameMode(GetWorld()));

		// Check if fixed cameras are enabled
		if (!gm->fixedCamera)
			return;

		if (camera != NULL)
		{
			// There is a camera associated to this volume and thus switches to fixed camera

			// Sets associated camera as current View Target aka active camera
			controller->SetViewTargetWithBlend(camera, BlendSpeed);
			player->isFixed = true; // Camera is now fixed
			player->fixedCameraMovementChange = false; // When this becomes true, movement direction changes
		}
		else
		{
			// No camera associated to this volume and thus switches to over the shoulder camera

			// Gets Follow Camera from player character
			AActor* playerCamera = player->GetFollowCamera()->GetOwner();
			// Sets this camera as current view target aka active camera
			controller->SetViewTargetWithBlend(playerCamera, BlendSpeed);
			controller -> SetControlRotation(playerCamera->GetActorRotation());
			player->isFixed = false; // Not fixed camera anymore
		}
	}
}