// The CarniEvil Corporation Incorporated own the rights and lefts of this project from today until the end of days.

#include "CarnievilPrototype.h"
#include "CarnievilPrototypeCharacter.h"
#include "HealthPickup.h"

void AHealthPickup::OnOverlapBegin(class AActor* OtherActor, 
									class UPrimitiveComponent* OtherComp,
									int32 OtherBodyIndex, bool bFromSweep,
									const FHitResult& SweepResult)
{
	// Get player character
	ACarnievilPrototypeCharacter* player =
		Cast<ACarnievilPrototypeCharacter>
		(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (OtherActor && (OtherActor == player) && OtherComp) // If the actor is the player
	{
		// Health is increased if health is not full
		if (player->currentHealth < player->startHealth){
			player->currentHealth = player->startHealth;
			OnPlayHealthPickupSound();
			Destroy();
		}
		
	}


}


