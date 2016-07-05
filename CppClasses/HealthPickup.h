// The CarniEvil Corporation Incorporated own the rights and lefts of this project from today until the end of days.

#pragma once

#include "Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class CARNIEVILPROTOTYPE_API AHealthPickup : public APickup
{
	GENERATED_BODY()
	
public:

	// Overlap function override
	UFUNCTION()
		virtual void OnOverlapBegin(class AActor* OtherActor,
									class UPrimitiveComponent* OtherComp,
									int32 OtherBodyIndex, bool bFromSweep,
									const FHitResult& SweepResult) override;
	
	UFUNCTION(BlueprintImplementableEvent)
		void OnPlayHealthPickupSound();
};
