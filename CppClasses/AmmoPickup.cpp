// The CarniEvil Corporation Incorporated own the rights and lefts of this project from today until the end of days.

#include "CarnievilPrototype.h"
#include "CarnievilPrototypeCharacter.h"
#include "AmmoPickup.h"


void AAmmoPickup::OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp,
								int32 OtherBodyIndex, bool bFromSweep, 
								const FHitResult& SweepResult)
{
	// Get player character
	ACarnievilPrototypeCharacter* player =
		Cast<ACarnievilPrototypeCharacter>
		(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (OtherActor && (OtherActor == player) && OtherComp) // If the actor is the player
	{
		// Ammo is increased
		player->currentMagazines++;

		if (player->currentAmmo > player->maxAmmo){
			player->currentAmmo = player->maxAmmo;
		}
		OnPlayAmmoPickupSound();
		Destroy();
	}


}

