// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
// Modified by Oscar Rosales, All Rights Reserved.
#pragma once
#include "AimingReticle.h"
#include "GameFramework/Character.h"
#include "CarnievilPrototypeCharacter.generated.h"

UCLASS(config = Game)
class ACarnievilPrototypeCharacter : public ACharacter
{
	GENERATED_BODY()

		/** Camera boom positioning the camera behind the character */
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	ACarnievilPrototypeCharacter();


	virtual void BeginPlay() override;

	// Need to tick
	virtual void Tick(float DeltaSeconds) override;

	// Overrides take damage function
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, 
							class AController* EventInstigator, 
							class AActor* DamageCauser) override;
	
	// Called upon death
	virtual void LifeSpanExpired() override;


	/* VARIABLES */
public:

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	/** Speed is multiplied by this whenever the character is not sprinting. 
		The higher it is, the faster the character is while walking  */
	UPROPERTY(EditAnywhere, Category = "Tuning")
	float WalkSpeed = 0.3f;
	/** Overall movement speed. Same rules as with the walk speed, but applies
		to both walking and sprinting */
	UPROPERTY(EditAnywhere, Category = "Tuning")
	float MovementSpeed = 0.9f;

	/** Angle in which the character can see enemies */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float VisibilityAngle = 45.0f;

	/** Radius in which the character can see enemies */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float SightRadius = 5000.0f;

	/** How far the character can shoot */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float AttackRange = 100000.0f;

	/** How far the character can shoot */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		FName AimingSocket = "Laser_socket";

	/** Components for laser sight*/
	UPROPERTY(EditAnywhere, Category = "Laser Sight")
	class UParticleSystemComponent* LaserBeamPSC;
	UPROPERTY(EditAnywhere, Category = "Laser Sight")
		UParticleSystem* LaserBeamFX;
	UPROPERTY(BlueprintReadOnly)
		FVector LaserPointPosition = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly)
		FRotator LaserPointRotation = FRotator::ZeroRotator;
	UPROPERTY(BlueprintReadOnly)
		FRotator LaserHit = FRotator::ZeroRotator;

	/** Components for laser sight*/
	UPROPERTY(EditAnywhere, Category = "Blood")
		UParticleSystem* BloodParticles;

	/** Components for laser sight*/
	UPROPERTY(EditAnywhere, Category = "Shooting")
		UParticleSystem* ShootingParticles;

	/** For noise emitting*/
	UPROPERTY(EditAnywhere, Category = "Noise Emmitter")
		UPawnNoiseEmitterComponent* NoiseEmitter;

	/** State variables */
	// Tells whether in fixed camera mode or not
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool isFixed = false;
	// Tells whether character is hiding or not
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool isHiding = false; 
	// Tells whether character is in a position to hide or not
	bool canHide = false; 
	// Tells whether character is locked onto an enemy for shooting
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool isLockedOn = false; 
	// Tells whether character is sprinting
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool isSprinting = false;
	// Tells whether player fell down and hasn't stood up
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool isDown = false; 
	// Tells whether player is currently reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool isReloading = false;
	// Tells whether player is currently reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool isShooting = false;
	// Tells whether player is currently pushing
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool isPushing = false;
	// Tells whether player is currently dying
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool isDying = false;
	// Tells whether player has grabbed the book pickup
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool hasBook = false;
	// When false, cannot sprint
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool maySprint = true; 
	// To stop reticle up movement
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool clampUp = false;
	// To stop reticle down movement
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool clampDown = false;
	// To enable or disable enemy following when locking on
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool lockOnFollow = true;

	/*
	 * Thes variables keep track on states that depend on input, delays, among others
	 */
	bool camSnapping = false;
	bool fixedCameraMovementChange = false;
	bool fixedCameraAimChange = false;
	bool usingMouse = false;

	FRotator snapRotTarget;

	/* Character's max health */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning")
		float maxHealth = 50;

	/* Character's starting health */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning")
		float startHealth = 50;

	/* Character's current health */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning")
		float currentHealth;

	/* Character's starting ammo */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		int32 startAmmo = 24;

	/* Character's current ammo */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning")
		int32 currentAmmo;

	/* Character's current total ammo */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning")
		int32 totalAmmo;

	/* Character's maximum ammo */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning")
		int32 maxAmmo = 24;

	/* Ammo cartridge size */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning")
		int32 ammoMagazineSize = 12;

	/* Ammo cartridge size */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning")
		int32 currentMagazines;

	/* Character's starting stress */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float startStress = 0;

	/* Character's maximum stress */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning")
		float maxStress = 30;

	/* Character's current stress */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
		float currentStress;

	/* Time it takes for character to stand up */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float reloadTime = 2.0f;

	/* Time it takes for character to stand up */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float standUpTime = 2.0f;

	/* Stand up time is increased by this each fall */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float standUpStep = 1.5f;

	/* Time elapsed for shot checks */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float shootIntervalTime = 0.1f;

	/* Time elapsed for push checks */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float pushIntervalTime = 0.8f;

	/* Time it takes for dying */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float deathDelayTime = 1.0f;

	/* Increase applied to the time running counter */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float timeRunningIncrease = 0.01f;
	
	/* Damage character's attacks deal */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		int32 damage = 50;

	/* Increase applied to the time running counter */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float aimingRotationSpeed = 0.1f;

	/* Input scale for aiming with the mouse */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float aimingInputScale = 1.5f;

	/* Input scale for aiming with the controller */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float cAimingInputScale = 2.0f;

	/* Field of View while locking on */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float lockOnFoV = 42.0f;

	/* Field of View while running */
	UPROPERTY(EditAnywhere, Category = "Tuning")
		float sprintFoV = 70.0f;

	/* Normal Field of View */
	float FoV;


	/* Character's current stress */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
		TArray<AActor*> actorsHidden;

	/* Contains transform of current checkpoint */
	FTransform checkpoint;

protected:

	FRotator cameraRotation;

	/* Direction towards which the character will rotate when locking on.
	* Updates rotation each tick
	*/
	FRotator lockOnRotation;

	/** Called for forwards/backward input */
	TArray<APawn*> VisiblePawns;

	/** Aiming reticle for shooting */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AAimingReticle* reticle;

	/* Offset added to angle to determine toggling targets */
	float lockOnOffset = 0;

	/* Timer for standing up after tripping */
	FTimerHandle StandUpTimerHandle;

	/* Timer for tripping check */
	FTimerHandle ReloadTimerHandle;

	/* Timer for shooting */
	FTimerHandle ShootingTimerHandle;

	/* Timer for shooting */
	FTimerHandle PushingTimerHandle;

	/* The higher it is, the longer the character has been running */
	float timeRunning = 0.0f;

	/* FUNCTIONS */
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** Normalizes movement input */
	void MoveNormalization();

	/**
	* Called via input to turn at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	void TurnMouse(float Rate);

	/**
	* Called via input to turn look up/down at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void LookUpAtRate(float Rate);

	void LookUpMouse(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	/** Indicates that player is locking on */
	void OnLockOnPressed();

	/** Stops locking on */
	void OnLockOnReleased();

	/** Changes locked on target to the one on the left */
	void LockOnLeft();

	/** Changes locked on target to the one on the right */
	void LockOnRight();

	/* Locks on to a certain pawn for shooting */
	UFUNCTION()
		void LockOn(APawn* Pawn);

	/** Refreshes enemies VisiblePawns array with all enemies in line of sight */
	void GetVisibleEnemies();

	/** Gets the target closest to the character */
	APawn* GetClosestTarget();

	/** Gets the target to the right of the locked character */
	APawn* GetRightTarget();

	/** Shoots */
	void OnShoot();

	/** Makes character run faster */
	void OnSprint();

	/** Character stops running */
	void OnSprintExit();

	/** Tells whether a pawn is in visible range */
	bool IsInRange(APawn* Pawn);

	/* Sets laser sight for shooting */
	void SetLaserSight();

	/* Reloads ammo */
	void OnReload();

	/* Stops reloading */
	void ReloadEnd();

	/* Shooting ends and is enabled once more */
	void ShotEnd();

	/** Character pushes enemy standing in front of it */
	void OnPush();

	/** Character has already pushed an enemy standing in front of it */
	void OnPushEnd();

	/* If character is too stressed, character may trip */
	void MayTrip();

	/* Character stands up after tripping */
	void StandUp();

	/* Snaps camera to face wherever the player is facing */
	void CamSnap();

	/* Stops camera snap */
	void CamSnapReleased();

	/* Toggles reticle's following of locked on enemies */
	void OnLockOnFollowToggle();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// Wwsie gunshot blueprint call
	UFUNCTION(BlueprintImplementableEvent)
		void OnPlayGunshotSound();

	// Event called through Wwise and animation when damaged
	UFUNCTION(BlueprintImplementableEvent)
		void OnPlayerDamage();

	// Event called through Wwise and animation when dead
	UFUNCTION(BlueprintImplementableEvent)
		void OnPlayerDeath();

	// Event called when pushing
	UFUNCTION(BlueprintImplementableEvent)
		void OnImplementPush(AAIBaronSamediPawn* enemyPawn );

	// Event called when locking on
	UFUNCTION(BlueprintImplementableEvent)
		void OnLockOnCamera();

	// Event called to destroy objects
	UFUNCTION(BlueprintImplementableEvent)
		void OnImplementDestroyable(AActor* actor);

	// Event called on a checkpoint
	UFUNCTION(BlueprintImplementableEvent)
		void OnCheckpoint();

	// Event called when out of ammo
	UFUNCTION(BlueprintImplementableEvent)
		void NoAmmo();

	void SetRotation();

	/* Applies a certain amount of damage to the character */
	UFUNCTION(BlueprintCallable, Category = "Carnievil")
	void ApplyCharDamage(int32 dmg);

	// Interacts with environment element such as hiding spots
	void Interact();

	/* Returns stress level calculation */
	UFUNCTION(BlueprintCallable, Category = "Carnievil")
	float GetStressLevel();

	/* Applies a specific amount of stress on the character */
	UFUNCTION(BlueprintCallable, Category = "Carnievil")
	void ApplyStress(float stress);


	/* Disables and stops sprinting */
	UFUNCTION(BlueprintCallable, Category = "Carnievil")
		void SprintDeactivation();

	/* Reenables sprinting */
	UFUNCTION(BlueprintCallable, Category = "Carnievil")
		void SprintActivation();

	// Hit function for tripping
	UFUNCTION()
		void OnHit(AActor* SelfActor, AActor* OtherActor, 
					FVector NormalImpulse, const FHitResult& Hit);

};

