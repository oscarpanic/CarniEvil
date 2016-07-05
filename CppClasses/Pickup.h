// The CarniEvil Corporation Incorporated own the rights and lefts of this project from today until the end of days.

#pragma once

#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class CARNIEVILPROTOTYPE_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickup();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UStaticMeshComponent* GetMesh() const { return pickupMesh; }
	
	virtual void OnOverlapBegin(class AActor* OtherActor, 
								class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
								bool bFromSweep, const FHitResult& SweepResult);


private:
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* pickupMesh;
	
	/** sphere component */
	UPROPERTY(EditAnywhere)
		USphereComponent* Sphere1;

};
