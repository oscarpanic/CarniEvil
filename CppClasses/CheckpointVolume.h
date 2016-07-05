// The CarniEvil Corporation Incorporated own the rights and lefts of this project from today until the end of days.

#pragma once

#include "GameFramework/Actor.h"
#include "CheckpointVolume.generated.h"

UCLASS()
class CARNIEVILPROTOTYPE_API ACheckpointVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACheckpointVolume();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Box area
	UPROPERTY(EditAnywhere)
		FVector BoxExtent;

	UFUNCTION()
		virtual void OnOverlapBegin(class AActor* OtherActor,
									class UPrimitiveComponent* OtherComp,
									int32 OtherBodyIndex, bool bFromSweep,
									const FHitResult& SweepResult);

private:
	// Collider
	UPROPERTY(EditAnywhere)
		UBoxComponent* BoxCollider;

};
