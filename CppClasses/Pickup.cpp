// The CarniEvil Corporation Incorporated own the rights and lefts of this project from today until the end of days.

#include "CarnievilPrototype.h"
#include "Pickup.h"


// Sets default values
APickup::APickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	pickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pickup"));
	pickupMesh->AttachTo(RootComponent);

	Sphere1 = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere1"));
	Sphere1->InitSphereRadius(100.0f);
	Sphere1->AttachTo(RootComponent);

	// Set up overlap function
	Sphere1->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnOverlapBegin);

}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APickup::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Called whenever there is an overlap with the Pickup
void APickup::OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, 
						int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Destroy();
}