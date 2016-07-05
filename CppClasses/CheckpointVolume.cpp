// The CarniEvil Corporation Incorporated own the rights and lefts of this project from today until the end of days.

#include "CarnievilPrototype.h"
#include "CarnievilPrototypeCharacter.h"
#include "CheckpointVolume.h"
#include "Engine.h"


// Sets default values
ACheckpointVolume::ACheckpointVolume()
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
	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &ACheckpointVolume::OnOverlapBegin);

}

// Called when the game starts or when spawned
void ACheckpointVolume::BeginPlay()
{
	Super::BeginPlay();
	BoxCollider->SetCollisionProfileName(TEXT("CameraVolume")); // Stops it from colliding
}

// Called every frame
void ACheckpointVolume::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Player enters volume
void ACheckpointVolume::OnOverlapBegin(class AActor* OtherActor,
									class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
									bool bFromSweep, const FHitResult& SweepResult)
{
	// Get player character
	ACarnievilPrototypeCharacter* player =
		Cast<ACarnievilPrototypeCharacter>
		(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (OtherActor && (OtherActor == player) && OtherComp) // If the actor is the player
	{
		player->checkpoint = GetTransform();
		player->OnCheckpoint();
		SetLifeSpan(0.001f); // Destroys volume after this
	}
}

