// Copyright 2016 Oscar Gerardo Rosales Fuentevilla

#include "CarnievilPrototype.h"
#include "AimingReticle.h"
#include "CarnievilPrototypeCharacter.h"
#include "Engine.h"

#define COLLISION_WEAPON ECC_GameTraceChannel1 = "Weapon"


// Sets default values
AAimingReticle::AAimingReticle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Mesh initalization
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AimingReticle"));
	Mesh->AttachTo(RootComponent);
	FVector scale = FVector(0.1, 0.1, 0.1);
	Mesh->SetWorldScale3D(scale);

}

// Called when the game starts or when spawned
void AAimingReticle::BeginPlay()
{
	Super::BeginPlay();

	// Gets player and sets him as current target to follow him around
	ACarnievilPrototypeCharacter* player =
		Cast<ACarnievilPrototypeCharacter>
		(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	currentTarget = player;

	// Sets StaticMesh
	myMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, 
		//TEXT("StaticMesh'/Game/Geometry/Meshes/1M_Cube.1M_Cube'")));
		TEXT("StaticMesh'/Game/Art/Laser_Sight/Mesh_Reticle.Mesh_Reticle'")));

	// If Mesh is valid, set it as StaticMeshComponent's mesh
	if (myMesh){
		Mesh->SetStaticMesh(myMesh); // Sets Mesh
		Mesh->SetHiddenInGame(true, false); // Is hidden at the beginning of the game
		Mesh->SetCollisionProfileName(TEXT("NoCollision")); // Stops it from colliding
		Mesh->SetWorldScale3D(FVector(1.0f) * 0.3); // Sets new scale for mesh

	}
	
	aimingNoise = FVector::ZeroVector;
	aimingInput = FVector::ZeroVector;
	lastTargetLocation = FVector::ZeroVector;

}

// Called every frame
void AAimingReticle::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	// Follows current target if existing
	if(currentTarget){

		// Sets target and noise
		FVector target = currentTarget->GetActorLocation();
		AddAimingNoise();	

		// Get reference to player to check if lockOnFollow is activated
		ACarnievilPrototypeCharacter* player =
			Cast<ACarnievilPrototypeCharacter>
			(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

		// Check if lock on following is activated and gets target location
		FVector targetLocation;
		if (player->lockOnFollow){
			targetLocation = GetActorLocation() + aimingInput + aimingNoise
				+ target - lastTargetLocation;
		}
		else{
			targetLocation = GetActorLocation() + aimingInput + aimingNoise
				+ target;
		}

		// Go to target location
		SetActorLocation(FMath::Lerp(GetActorLocation(), targetLocation, lerpSpeed));

		// Updates target's location
		lastTargetLocation = currentTarget->GetActorLocation();
	

	}
}

// Makes reticle aim a certain target
void AAimingReticle::TargetActor(AActor* target)
{
	//Mesh->SetHiddenInGame(false, false); // Shows mesh when aiming
	currentTarget = target; // Sets target
	AddAimingNoise(); // Adds noise to aiming location
	
	/* Noise is generated to avoid directly aiming at enemy on lock on */

	// Get player and its rotation
	ACarnievilPrototypeCharacter* player =
		Cast<ACarnievilPrototypeCharacter>
		(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	FRotator rot = player->GetActorRotation();

	// Use rotation for noise generation on X and Y
	float initialNoiseX = FMath::RandRange(-initialAimingOffset, initialAimingOffset);
	FVector initialNoiseLocation = AdjustVectorXY(rot, initialNoiseX);
	// Noise on Z
	initialNoiseLocation.Z = FMath::RandRange(-initialAimingOffset, initialAimingOffset); 

	// Set reticle location
	if (HasTarget()){
		// Saves its current location
		lastTargetLocation = currentTarget->GetActorLocation();
		SetActorLocation(currentTarget->GetActorLocation() + initialNoiseLocation);
	}
	else{
		FVector Bounds;
		FVector Origin;
		currentTarget->GetActorBounds(false, Origin, Bounds);
		float limitZ = Origin.Z - Bounds.Z/2;
		FVector offset = player->GetActorForwardVector();
		offset.X *= 500;
		offset.Y *= 500;

		// Saves its current location and last target location
		SetActorLocation(player->GetMesh()->GetSocketLocation("Hand_RSocket") + offset);
		lastTargetLocation = currentTarget->GetActorLocation();
	}
	
}

// Stops targetting at an enemy
void AAimingReticle::UntargetActor()
{
	// Gets player and sets him as current target to follow him around
	ACarnievilPrototypeCharacter* player =
		Cast<ACarnievilPrototypeCharacter>
		(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	// Hides mesh when not aiming
	Mesh->SetHiddenInGame(true, false);
	
	// Sets player as current target
	currentTarget = player;
}

// Tells whether it is targetting something
bool AAimingReticle::HasTarget()
{
	// Gets player
	ACarnievilPrototypeCharacter* player =
		Cast<ACarnievilPrototypeCharacter>
		(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	// Check if currentTarget is not the player
	if (currentTarget != player && currentTarget != NULL)
		return true;
	else
		return false;
}

// Returns currentTarget
AActor* AAimingReticle::GetTarget()
{
	return currentTarget;
}

// Sets aimingNoise value
void AAimingReticle::AddAimingNoise()
{

	// Gets player, its stress level and its rotation
	ACarnievilPrototypeCharacter* player =
		Cast<ACarnievilPrototypeCharacter>
		(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	float stressLevel = player->GetStressLevel();
	FRotator rot = player->GetActorRotation();

	// Gets distance to player
	float distance = (GetActorLocation() - player->GetActorLocation()).Size();

	// Creates noise value based on distance and the two noise base variables
	float noise = aimingNoiseBase;

	// noise variables
	float noiseX = 0.0f;
	float noiseZ = 0.0f;

	noiseX = FMath::RandRange(-stressLevel * noise,
		stressLevel * noise);
	noiseZ = FMath::RandRange(-stressLevel * noise,
		stressLevel * noise);

	// Adjust and assign noise in Y and X, according to Yaw, based on noiseX
	FVector adjustment = AdjustVectorXY(rot, noiseX * aimingSpeed);

	// Assign noise in X and Y according to adjustment obtained
	aimingNoise.X = adjustment.X;
	aimingNoise.Y = adjustment.Y;

	// Assign noise in Z
	aimingNoise.Z = noiseZ * aimingSpeed;

	// Adjust rate to distance from player
	aimingNoise.X = AdjustInputToDistance(aimingNoise.X);
	aimingNoise.Y = AdjustInputToDistance(aimingNoise.Y);
	aimingNoise.Z = AdjustInputToDistance(aimingNoise.Z);
	
}

// Gets input from character on x axis to adjust reticle position
void AAimingReticle::AddInputX(float Rate)
{

	// Adjust rate to distance from player and aimingSpeed
	Rate = AdjustInputToDistance(Rate)  * aimingSpeed;

	// Gets player and the rotation
	ACarnievilPrototypeCharacter* player =
		Cast<ACarnievilPrototypeCharacter>
		(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	FRotator rot;

	// Check for fixed camera angle aiming
	if (player->isFixed){
		// Fixed camera rotation varies

		// fixed camera aiming rotation changes
		if (Rate == 0){
			player->fixedCameraAimChange = false;
			invertedRate = false;
		}
					

		// Check for aiming change
		if (!player->fixedCameraAimChange)
		{

			// Get character and camera rotation
			FRotator camRot = player->GetController()->GetViewTarget()->GetActorRotation();
			FRotator playerRot = player->GetActorRotation();
			
			// Normalize rotation to 360 degree scale
			float yaw = playerRot.Yaw - camRot.Yaw;

			if (yaw < -179){
				yaw += 360;
			}
			else if (yaw > 180){
				yaw -= 360;
			}

			// Checks if facing camera to invert rate
			if (yaw < -135 || yaw > 135){
				invertedRate = true;
			}

			currentRotTarget = player;

			player->fixedCameraAimChange = true;
		}
	}
	else{
		// Rotation depends on character
		currentRotTarget = player;
	}

	// Inverts rate if facing camera
	if (invertedRate){
		Rate = -Rate;
	}

	// Rotation is set
	rot = currentRotTarget->GetActorRotation();

	// Adjust input rate according to rotation
	FVector result = AdjustVectorXY(rot, Rate);

	// Use obtained input
	aimingInput.X = result.X;
	aimingInput.Y = result.Y;

}

// Gets input from character on y axis to adjust reticle position
void AAimingReticle::AddInputY(float Rate)
{
	aimingInput.Z = -Rate;
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::SanitizeFloat(Rate)); // Debug

	// Adjust rate to distance from player
	aimingInput.Z = AdjustInputToDistance(aimingInput.Z) * aimingSpeed;
}


// Adjusts vectors used for reticle movement in the X and Y axis
FVector AAimingReticle::AdjustVectorXY(FRotator rot, float Rate){

	// Initialize resulting vector
	FVector result = FVector::ZeroVector;

	// 1st Cuadrant,   0 - 90
	if (rot.Yaw >= 0 && rot.Yaw <= 90){
		result.X = -Rate * (rot.Yaw / 90);
		result.Y = Rate * (1 - (rot.Yaw / 90));

	} // 2nd Cuadrant, 90 - 180
	else if (rot.Yaw > 90 && rot.Yaw <= 180){
		result.Y = -Rate * ((rot.Yaw - 90) / 90);
		result.X = -Rate * (1 - (rot.Yaw - 90) / 90);

	}// 4th Cuadrant, 0 - -90
	else if (rot.Yaw < 0 && rot.Yaw >= -90){
		result.X = -Rate * (rot.Yaw / 90);
		result.Y = Rate * (1 + (rot.Yaw / 90));

	} // 3rd Cuadrant, -90 - -180
	else if (rot.Yaw < -90 && rot.Yaw >= -180){
		result.Y = Rate * ((rot.Yaw + 90) / 90);
		result.X = Rate * (1 + (rot.Yaw + 90) / 90);

	}

	return result;

}


// Adjusts an input rate to the distance to the player
float AAimingReticle::AdjustInputToDistance(float Rate)
{
	// Gets player
	ACarnievilPrototypeCharacter* player =
		Cast<ACarnievilPrototypeCharacter>
		(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	// Gets distance to player
	FVector distanceVector = player->GetActorLocation() - GetActorLocation();
	float distance = 0;
	FVector dir = FVector::ZeroVector;
	distanceVector.ToDirectionAndLength(dir, distance);

	// Adjusts the vector
	float adjustedRate = Rate * distance;

	return adjustedRate;
}