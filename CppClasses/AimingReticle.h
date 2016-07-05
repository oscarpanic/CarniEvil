// Copyright 2016 Oscar Gerardo Rosales Fuentevilla

#pragma once

#include "GameFramework/Actor.h"
#include "AimingReticle.generated.h"

UCLASS()
class CARNIEVILPROTOTYPE_API AAimingReticle : public AActor
{
	GENERATED_BODY()

	// Sets default values for this actor's properties
	AAimingReticle();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;


/* Variables */
private:
	// Actor's Mesh and MeshComponent
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;
	UStaticMesh* myMesh;

	// Contains current target
	AActor* currentTarget;

	// Current target's last checked location, used for following target when it moves
	FVector lastTargetLocation;

	// Contains offset for aiming location
	FVector aimingNoise;

	// Contains input for movement
	FVector aimingInput;

	// Base upon which noise is generated
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float aimingNoiseBase = 5.0f;

	// Speed of reticle
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float aimingSpeed = 0.015f;

	// Noise added to reticle's initial position when targetting
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float initialAimingOffset = 40.0f;

	// Timer for resetting noise
	FTimerHandle resetTargLocTimer;

	// Time for noise resetting
	float resetTime = 0.5f;

	// Basically reticles movement speed, used to lerp to a target location
	UPROPERTY(EditAnywhere, Category = "Tuning")
	float lerpSpeed = 0.8f;

	// Used for aiming rotation according to camera
	AActor* currentRotTarget;
	bool invertedRate = false;

/* Functions */
public:
	// Makes reticle aim a certain target
	void TargetActor(AActor* target);

	// Stops targetting at an enemy
	void UntargetActor();

	// Tells whether it is targetting something
	bool HasTarget();

	// Returns currentTarget
	AActor* GetTarget();

	// Gets input from character to adjust reticle position
	void AddInputX(float Rate); // X axis
	void AddInputY(float Rate); // Y axis

private:

	// Sets aimingNoise value
	void AddAimingNoise();

	// Adjusts vectors used for reticle movement in the X and Y axis
	FVector AdjustVectorXY(FRotator rot, float Rate);

	// Adjusts an input rate to the distance to the player
	float AdjustInputToDistance(float Rate);

};
