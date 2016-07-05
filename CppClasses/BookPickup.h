// The CarniEvil Corporation Incorporated own the rights and lefts of this project from today until the end of days.

#pragma once

#include "Pickup.h"
#include "BookPickup.generated.h"

/**
 * 
 */
UCLASS()
class CARNIEVILPROTOTYPE_API ABookPickup : public APickup
{
	GENERATED_BODY()
	
	
protected:
	// Overlap function override
	UFUNCTION()
		virtual void OnOverlapBegin(class AActor* OtherActor,
									class UPrimitiveComponent* OtherComp,
									int32 OtherBodyIndex, bool bFromSweep,
									const FHitResult& SweepResult) override;
	
};
