// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
// Modified by Oscar Rosales, All Rights Reserved.

#include "CarnievilPrototype.h"
#include "CarnievilAIController.h"
#include "CarnievilPrototypeGameMode.h"
#include "Engine.h"
#include "CarnievilPrototypeCharacter.h"

#define COLLISION_WEAPON ECC_GameTraceChannel1 = "Weapon"

//////////////////////////////////////////////////////////////////////////
// ACarnievilPrototypeCharacter

ACarnievilPrototypeCharacter::ACarnievilPrototypeCharacter()
{

	// Initializes to current rotation
	lockOnRotation = GetActorRotation();

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->AttachTo(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->AttachTo(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	/* Laser Sight components initialization */
	LaserBeamPSC = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("LaserBeam"));
	LaserBeamPSC->SetActive(true);
	LaserBeamPSC->AttachTo(RootComponent);

	/* Noise emitter initialization */
	NoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Noise Emitter"));

	// Hit function setup
	OnActorHit.AddDynamic(this, &ACarnievilPrototypeCharacter::OnHit);

}

//////////////////////////////////////////////////////////////////////////
// Input

void ACarnievilPrototypeCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// Set up gameplay key bindings
	check(InputComponent);
	InputComponent->BindAction("Interact", IE_Pressed, this, &ACarnievilPrototypeCharacter::Interact);
	InputComponent->BindAction("Shoot", IE_Pressed, this, &ACarnievilPrototypeCharacter::OnShoot);
	InputComponent->BindAction("LockOn", IE_Pressed, this, &ACarnievilPrototypeCharacter::OnLockOnPressed);
	InputComponent->BindAction("LockOn", IE_Released, this, &ACarnievilPrototypeCharacter::OnLockOnReleased);
	InputComponent->BindAction("LeftTarget", IE_Pressed, this, &ACarnievilPrototypeCharacter::LockOnLeft);
	InputComponent->BindAction("RightTarget", IE_Pressed, this, &ACarnievilPrototypeCharacter::LockOnRight);
	InputComponent->BindAction("Sprint", IE_Pressed, this, &ACarnievilPrototypeCharacter::OnSprint);
	InputComponent->BindAction("CamSnap", IE_Pressed, this, &ACarnievilPrototypeCharacter::CamSnap);
	InputComponent->BindAction("CamSnap", IE_Released, this, &ACarnievilPrototypeCharacter::CamSnapReleased);
	InputComponent->BindAction("Reload", IE_Pressed, this, &ACarnievilPrototypeCharacter::OnReload);
	InputComponent->BindAction("Push", IE_Pressed, this, &ACarnievilPrototypeCharacter::OnPush);
	InputComponent->BindAction("LockOnFollow", IE_Pressed, this, &ACarnievilPrototypeCharacter::OnLockOnFollowToggle);

	InputComponent->BindAxis("MoveForward", this, &ACarnievilPrototypeCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ACarnievilPrototypeCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &ACarnievilPrototypeCharacter::TurnMouse);
	InputComponent->BindAxis("TurnRate", this, &ACarnievilPrototypeCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &ACarnievilPrototypeCharacter::LookUpMouse);
	InputComponent->BindAxis("LookUpRate", this, &ACarnievilPrototypeCharacter::LookUpAtRate);

	// handle touch devices
	InputComponent->BindTouch(IE_Pressed, this, &ACarnievilPrototypeCharacter::TouchStarted);
	InputComponent->BindTouch(IE_Released, this, &ACarnievilPrototypeCharacter::TouchStopped);

}

// Executed when game starts
void ACarnievilPrototypeCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize health and stress
	currentHealth = startHealth;
	currentStress = startStress;

	FoV = FollowCamera->FieldOfView;

	// Initialize ammo and magazine values
	totalAmmo = startAmmo;
	if (totalAmmo >= ammoMagazineSize){
		currentMagazines = totalAmmo / ammoMagazineSize - 1;
		currentAmmo = totalAmmo - currentMagazines * ammoMagazineSize;
	}
	else{
		currentMagazines = 0;
		currentAmmo = startAmmo;
	}

	// Initialize start point as checkpoint
	checkpoint = GetTransform();

	// Spawns reticle
	reticle = Cast<AAimingReticle>(GetWorld()->SpawnActor<AAimingReticle>());

	reticle->SetActorLocation(GetActorLocation());

	// Assign laser sight particle to component
	LaserBeamPSC->Template = LaserBeamFX;

	isDying = false;
}


// Executed every Tick
void ACarnievilPrototypeCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// If no movement, stop sprinting if sprinting was active
	if (GetVelocity().Size() == 0.0f){
		OnSprintExit();

		// If fixed, make fixed camera movement depend on fixed camera after stopping
		if (isFixed){
			fixedCameraMovementChange = true;
			GetController()->SetControlRotation(GetController()->GetViewTarget()->GetActorRotation());
		}
	}

	
	if (isSprinting){
		// Adds to current sprint time if running
		timeRunning += timeRunningIncrease;

		// Zoom in
		FollowCamera->FieldOfView = FMath::Lerp(FollowCamera->FieldOfView, sprintFoV, 0.2f);


	}
	else {
		// Zoom out
		FollowCamera->FieldOfView = FMath::Lerp(FollowCamera->FieldOfView, FoV, 0.2f);

		if (timeRunning > 0){
			timeRunning -= timeRunningIncrease;
		}
	}


	// When locked on, rotates towards AimingReticle
	if (isLockedOn){
		// Is locked on

		SetLaserSight(); // Set and draw laser sight


		// Gets direction of AimingReticle to rotate towards it
		FVector direction = reticle->GetActorLocation() - GetActorLocation();
		direction.Z = 0; // No rotation on z axis
		lockOnRotation = direction.Rotation(); // Gets rotation
		SetActorRotation(FMath::Lerp(GetActorRotation(), lockOnRotation, aimingRotationSpeed)); // Applies rotation

		// If no target, looks for one
		if (reticle->GetTarget()->IsPendingKill()){
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "No Target"); // Debug
			LockOnRight();
		}

		// If not fixed camera, snap camera behind character
		if (!isFixed){

			// Gets direction of AimingReticle
			FVector direction = reticle->GetActorLocation() - GetActorLocation();
			FRotator targetCamRotation = direction.Rotation(); // Gets rotation
			targetCamRotation.Yaw += 10;
	
			GetController()->SetControlRotation
				(FMath::Lerp(GetController()->GetControlRotation(), direction.Rotation(), 0.3f));
			
			// Lerps field of view
			FollowCamera->FieldOfView = FMath::Lerp(FollowCamera->FieldOfView, lockOnFoV, 0.2f);
		}

	}
	else
	{

		// If not locked on, field of view returns to normal
		if (!isFixed){
			FollowCamera->FieldOfView = FMath::Lerp(FollowCamera->FieldOfView, FoV, 0.2f);
		}
		
	}

	// Lerps camera rotation when snapping and not in fixed camera angle
	if (camSnapping && !isFixed){
		GetController()->SetControlRotation
			(FMath::Lerp(GetController()->GetControlRotation(), snapRotTarget, 0.2f));
	}

}


// Called upon death
void ACarnievilPrototypeCharacter::LifeSpanExpired()
{
	// Get controller
	APlayerController* playerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	
	// Current hidden actors
	TArray<AActor*> hiddenA = actorsHidden;

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Dying");
	reticle->SetLifeSpan(0.001f); // Destroy reticle along with character
	Super::LifeSpanExpired();
	
	// Loop througt all the AI pawns
	for (TActorIterator<AAIBaronSamediPawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ActorItr->GetMesh()->SetVisibility(true, true);
	}

	// Restart player with controller
	if (playerController)
	{
		GetWorld()->GetAuthGameMode()->RestartPlayer(playerController);
		ACarnievilPrototypeCharacter* player =
			Cast<ACarnievilPrototypeCharacter>
			(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		player->SetActorLocation(checkpoint.GetLocation());
		player->SetActorRotation(checkpoint.GetRotation());
		player->isDying = false;
		player->checkpoint = checkpoint;
		player->actorsHidden = hiddenA;
	}
}


void ACarnievilPrototypeCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	// jump, but only on the first touch
	if (FingerIndex == ETouchIndex::Touch1)
	{
		Jump();
	}
}

void ACarnievilPrototypeCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (FingerIndex == ETouchIndex::Touch1)
	{
		StopJumping();
	}
}

// Right analog stick X Axis with controller
void ACarnievilPrototypeCharacter::TurnAtRate(float Rate)
{
	// If locked on, used to move reticle
	if (isLockedOn){
		if (Rate != 0){
			// Adds input
			reticle->AddInputX(Rate * aimingInputScale);
			usingMouse = false; // Using controller
		}
		else if (!usingMouse){
			// Stops input if using the controller
			reticle->AddInputX(0);
		}
		return;
	}

	// If fixed, doesn't modify rotation
	if (isFixed)
		return;

	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

// Right analog stick X Axis with mouse
void ACarnievilPrototypeCharacter::TurnMouse(float Rate)
{
	// If locked on, used to move reticle
	if (isLockedOn){
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::SanitizeFloat(Rate * aimingInputScale)); // Debug
		if (Rate != 0){
			usingMouse = true; // Using mouse
			reticle->AddInputX(Rate * aimingInputScale);
		}
		else if (usingMouse){
			// Stops input if using mouse
			reticle->AddInputY(0);
		}
			
		return;
	}

	// If fixed, doesn't modify rotation
	if (isFixed)
		return;

	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate);
}

// Right analog stick Y Axis with controller
void ACarnievilPrototypeCharacter::LookUpAtRate(float Rate)
{
	// If locked on, used to move reticle
	if (isLockedOn){

		// If character can point higher
		if (!clampDown && Rate > 0){
			reticle->AddInputY(Rate * cAimingInputScale);
			usingMouse = false; // Using controller

		} // If character can point lower
		else if (!clampUp && Rate < 0){
			reticle->AddInputY(Rate * cAimingInputScale);
			usingMouse = false; // Using controller
		}
		else if(!usingMouse){
			// Stops input if using the controller
			reticle->AddInputY(0.0f);
		}

		return;
	}

	// If fixed, doesn't modify rotation
	if (isFixed)
		return;

	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

// Right analog stick Y Axis with mouse
void ACarnievilPrototypeCharacter::LookUpMouse(float Rate)
{
	// If locked on, used to move reticle
	if (isLockedOn){

		// If character can point higher
		if (!clampDown && Rate > 0){
			reticle->AddInputY(Rate * aimingInputScale);
			usingMouse = true; // Using mouse

		} // If character can point lower
		else if (!clampUp && Rate < 0){
			reticle->AddInputY(Rate * aimingInputScale);
			usingMouse = true; // Using mouse
		}
		else if (usingMouse){
			// Stops input if using the controller
			reticle->AddInputY(0);
		}

		return;
	}

	// If fixed, doesn't modify rotation
	if (isFixed)
		return;

	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate);
}

void ACarnievilPrototypeCharacter::MoveForward(float Value)
{
	// If locked on, down, reloading, dying, or pushing, character does not move
	if (isLockedOn || isDown || isReloading || isDying ||isPushing)
		return;

	// Scales value to match movement speed
	Value = Value * MovementSpeed;

	if ((Controller != NULL) && (Value != 0.0f) && !isHiding)
	{
		FRotator Rotation;
		// find out which way is right
		if (!isFixed || !fixedCameraMovementChange)
		{
			Rotation = Controller->GetControlRotation();
		}
		else{
			Rotation = Controller->GetViewTarget()->GetActorRotation();
		}
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Checks if character is sprinting
		if (!isSprinting)
			Value = Value * WalkSpeed; // Reduces speed if that is the case

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
	MoveNormalization();
}

void ACarnievilPrototypeCharacter::MoveRight(float Value)
{
	// If locked on, down, reloading, or dying, character does not move
	if (isLockedOn || isDown || isReloading || isDying || isPushing)
		return;

	// Scales value to match movement speed
	Value = Value * MovementSpeed;

	if ((Controller != NULL) && (Value != 0.0f) && !isHiding)
	{
		FRotator Rotation;
		// find out which way is right
		if (!isFixed || !fixedCameraMovementChange)
		{
			Rotation = Controller->GetControlRotation();
		}
		else{
			Rotation = Controller->GetViewTarget()->GetActorRotation();
		}

		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Checks if character is sprinting
		if (!isSprinting)
			Value = Value * WalkSpeed; // Reduces speed if that is the case

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
	MoveNormalization();
}

// Normalizes movement input
void ACarnievilPrototypeCharacter::MoveNormalization()
{
	GetPendingMovementInputVector().Normalize(MovementSpeed);
}

void ACarnievilPrototypeCharacter::SetRotation()
{
	if (Controller != NULL){
		cameraRotation = Controller->GetViewTarget()->GetActorRotation();
	}

}


// Interacts with environment
void ACarnievilPrototypeCharacter::Interact()
{
	if (!isHiding){
		// Is not currently hiding
		if (canHide){
			// Is in range of hiding spot
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Interacting");
			isHiding = true;
			GetMesh()->SetHiddenInGame(true, false);
		}
		else{
			// Out of hiding spot range
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "You cannot interact, dummy");
		}
	}
	else{
		// Is currently hiding
		isHiding = false; // Leaves hiding spot
		GetMesh()->SetHiddenInGame(false, false);
	}

}


// Executes a ranged attack
void ACarnievilPrototypeCharacter::OnShoot()
{

	// If not locked on or is already shooting, does nothing
	if (!isLockedOn || isShooting)
		return;

	// Call blueprint function to use Wwise
	OnPlayGunshotSound();

	// If no ammo, cannot shoot
	if (currentAmmo <= 0){
		NoAmmo();
		return;
	}

	// Shooting is on
	isShooting = true;

	// Calculate startpoint of trace
	const FVector StartTrace = GetMesh()->GetSocketLocation(AimingSocket);

	// Calculate endpoint of trace  
	const FVector EndTrace = StartTrace
		+ GetMesh()->GetSocketRotation(AimingSocket).Vector() * AttackRange;

	// Prepare for raycasting
	FHitResult Hit; // Will contain information about raycast collision
	FCollisionQueryParams Line(FName("Collision param"), true);
	UWorld* World = GEngine->GameViewport->GetWorld(); // World

	// For showing raycast debug line
	/*const FName TraceTag("MyTraceTag");
	World->DebugDrawTraceTag = TraceTag;
	Line.TraceTag = TraceTag;*/

	// Traces raycast
	World->LineTraceSingleByChannel(Hit, StartTrace,
		EndTrace, ECC_GameTraceChannel1, Line);

	// One ammunition has been used
	currentAmmo--;
	totalAmmo--;

	// Spawns particles in gun
	UGameplayStatics::SpawnEmitterAttached(ShootingParticles,
											GetMesh(),
											AimingSocket,
											FVector(0,0,0),
											FRotator(0,0,0),
											EAttachLocation::KeepRelativeOffset,
											true);

	// Sets timer for shoot end 
	GetWorldTimerManager().SetTimer(ShootingTimerHandle, this,
		&ACarnievilPrototypeCharacter::ShotEnd, shootIntervalTime, false);


	// Gets actor that was hit
	APawn* actor = Cast<APawn>(Hit.GetActor());
	if (actor){ // If hit something
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Gotcha"); // Debug log

		// Check whether hit actor is an enemy
		AAIBaronSamediPawn* enemy = Cast<AAIBaronSamediPawn>(actor);
		if (enemy){
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Hit Guy");
			
			// Spawn blood particles
			UGameplayStatics::SpawnEmitterAttached(BloodParticles,
										enemy->GetMesh(),
										"None",
										Hit.Location,
										FRotator(0, 0, 0),
										EAttachLocation::KeepWorldPosition,
										true);
			
			// Deals damage
			TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
			FDamageEvent DamageEvent(ValidDamageTypeClass);
			UGameplayStatics::ApplyPointDamage(enemy, damage, GetActorLocation(), Hit, 
												this->GetController(), this,
												ValidDamageTypeClass);

			// Changes target after enemy kill
			if (enemy->IsPendingKill())
				reticle->UntargetActor(); // Stops targetting enemy
		}

	}
	else {
		OnImplementDestroyable(Hit.GetActor());
	}

}


// Locks on to a certain pawn for shooting
void ACarnievilPrototypeCharacter::LockOn(APawn* Pawn)
{

	// Validate pawn
	if (!Pawn)
		return;
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Seen"); // Debug
	
	// Makes reticle target the pawn
	reticle->TargetActor(Pawn);

	// Saves camera field of view
	FoV = FollowCamera->FieldOfView;

}

// Tells whether a pawn is in visible range
bool ACarnievilPrototypeCharacter::IsInRange(APawn* Pawn)
{

	// Distance between pawn and character
	FVector distanceVector = Pawn->GetActorLocation() - GetActorLocation();
	float distance = 0;
	FVector dir = FVector::ZeroVector;
	distanceVector.ToDirectionAndLength(dir, distance);

	// Gets angle between pawn/camera and character
	float angle;
	if (!isFixed){
		// Not fixed, depends on camera
		angle = FVector::DotProduct(CameraBoom->GetComponentRotation().Vector(), dir); //GetActorRotation().Vector(), dir);
	}
	else{
		// Fixed, depends on character
		angle = FVector::DotProduct(GetActorRotation().Vector(), dir);
	}

	dir = dir.RotateAngleAxis(lockOnOffset, GetActorUpVector());

	if (distance > SightRadius || // if pawn is too far away
		angle < FMath::Cos(VisibilityAngle)){ // not in visibility angle
		return false;
	}

	// Raytrace to check if something is blocking its way
	FCollisionQueryParams TraceParams(TEXT("Trace"), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.AddIgnoredActor(this);
	TraceParams.AddIgnoredActor(Pawn);
	TraceParams.bTraceComplex = false;

	// Hit to get blocking point
	FHitResult Hit(ForceInit);
	FVector traceStart = GetActorLocation();
	traceStart.Z += 10; // Offset to start from head
	// Execute raytrace
	GetWorld()->LineTraceSingleByChannel(Hit, traceStart, Pawn->GetActorLocation(),
		ECC_Visibility, TraceParams);

	// Location after adjusting according to raycast hit
	FVector_NetQuantize LaserTargetLocation;

	// Check for block
	if (Hit.bBlockingHit)
	{
		return false; // Target behind something
	}

	// Pawn is in sight range if reached this point
	return true;
}


// Refreshes enemies VisiblePawns array with all enemies in line of sight
void ACarnievilPrototypeCharacter::GetVisibleEnemies()
{
	VisiblePawns.Empty(); // Empties array

	// Iterate through pawns to find list of enemies in sight
	for (FConstPawnIterator it = GetWorld()->GetPawnIterator(); it; it++){
		APawn* currentPawn = Cast<APawn>(*it);

		if (currentPawn && // Exists
			currentPawn != this && // Not this character
			IsInRange(currentPawn) && // Pawn is in visible range
			reticle->GetTarget() != currentPawn &&
			Cast<AAIBaronSamediPawn>(currentPawn)->m_health > 0){ // Alive
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Target On Sight"); // Debug
			VisiblePawns.Add(currentPawn); // Adds Pawn to array of visible pawns
		}
	}

}


// Indicates that player is locking on
void ACarnievilPrototypeCharacter::OnLockOnPressed()
{
	// If reloading, down, or dying, does not lock on
	if (isReloading || isDying || isDown)
		return;

	// Sets lock on to true
	isLockedOn = true;

	OnLockOnCamera();

	// Gets enemies in line of sight
	GetVisibleEnemies();

	// If there are pawns in range
	if (VisiblePawns.Num() > 0)
		LockOn(GetClosestTarget());
	else
		LockOn(this);
}


// Stops locking on
void ACarnievilPrototypeCharacter::OnLockOnReleased()
{
	// Sets lock on to true
	isLockedOn = false;

	// Stops targetting
	reticle->UntargetActor();

	// Resets offset for locking onto a certain direction
	lockOnOffset = 0;

	// Hide laser sight
	LaserBeamPSC->SetActive(false, false);
}


// Changes locked on target to the one on the left
void ACarnievilPrototypeCharacter::LockOnLeft()
{
	// If not locked on, does nothing
	if (!isLockedOn)
		return;

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Target Left"); // Debug

	lockOnOffset = 45; // Sets offset for locking on angle
	OnLockOnPressed(); // Locks on
}


// Changes locked on target to the one on the right
void ACarnievilPrototypeCharacter::LockOnRight()
{
	// If not locked on, does nothing
	if (!isLockedOn)
		return;

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Target Right"); // Debug

	lockOnOffset = -45;// Sets offset for locking on angle
	OnLockOnPressed(); // locks on
}

// Gets the target closest to the character
APawn* ACarnievilPrototypeCharacter::GetClosestTarget()
{
	// Sets closest pawn so far and the distance to it
	APawn* closestPawn = VisiblePawns[0];
	FVector distanceToCurrent = GetActorLocation() - closestPawn->GetActorLocation();

	for (int i = 1; i < VisiblePawns.Num(); i++){
		// Get distance vector to pawn
		FVector distance = GetActorLocation() - VisiblePawns[i]->GetActorLocation();

		// Compare distances to figure out closest one
		if (distance.Size() < distanceToCurrent.Size()){
			// Current pawn in iteration is now the closest
			closestPawn = VisiblePawns[i];
			distanceToCurrent = distance;
		}
	}

	return closestPawn;
}


// Makes character run faster
void ACarnievilPrototypeCharacter::OnSprint()
{
	if (!isDown && maySprint) // Can't run if down or running is disabled
		isSprinting = true;
}


// Character stops running
void ACarnievilPrototypeCharacter::OnSprintExit()
{
	isSprinting = false;
}

// Applies a certain amount of damage to the character
void ACarnievilPrototypeCharacter::ApplyCharDamage(int32 dmg)
{
	// If invulnerability is on or already dead, do nothing
	ACarnievilPrototypeGameMode* gm = Cast<ACarnievilPrototypeGameMode>(GetWorld()->GetAuthGameMode());
	if (gm->isInvulnerable || currentHealth <= 0)
		return;

	currentHealth -= dmg;

	// Dies if no health
	if (currentHealth <= 0){
		OnPlayerDeath(); // Event

		this->SetLifeSpan(deathDelayTime); // Kill character
		isDying = true; // character is dying
		isLockedOn = false;
	}

	OnPlayerDamage(); // Event

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "AAAAAAAAAAAAAAAAAAAAAY"); //Debug
}


// Override damage causing function from ACharacter
float ACarnievilPrototypeCharacter::TakeDamage(float Damage,
												struct FDamageEvent const& DamageEvent,
												class AController* EventInstigator,
												class AActor* DamageCauser)
{
	// Calls parent function
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, 
												EventInstigator, DamageCauser);
	// Calls local damaging function
	ApplyCharDamage((int)ActualDamage);

	if(DamageEvent.IsOfType(FPointDamageEvent::ClassID)){
		const FPointDamageEvent* pointDmg = (const FPointDamageEvent*)&DamageEvent;

		// Spawn blood particles
		UGameplayStatics::SpawnEmitterAttached(BloodParticles,
			this->GetMesh(),
			"None",
			pointDmg->HitInfo.Location,
			FRotator(0, 0, 0),
			EAttachLocation::SnapToTarget,
			true);
	}

	return ActualDamage;

}


// Sets laser sight for shooting
void ACarnievilPrototypeCharacter::SetLaserSight()
{

	// Start point for laser sight
	const FVector TraceStart = GetMesh()->GetSocketLocation(AimingSocket);
	// End point for laser sight
	FVector TraceEnd = TraceStart + GetMesh()->GetSocketRotation(AimingSocket).Vector() * AttackRange;
	//reticle->GetActorLocation() - TraceStart) * AttackRange;

	// Raytrace to check if something is blocking its way
	FCollisionQueryParams TraceParams(TEXT("Trace"), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.AddIgnoredActor(this);
	TraceParams.bTraceComplex = false;

	// Hit to get blocking point
	FHitResult Hit(ForceInit);
	// Execute raytrace
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
	
	// Location after adjusting according to raycast hit
	FVector_NetQuantize LaserTargetLocation;

	if (Hit.GetActor()){
		LaserHit = Hit.GetActor()->GetActorRotation();
	}
	else{
		LaserHit = FRotator::ZeroRotator;
	}
	

	// Check for block
	if (Hit.bBlockingHit)
	{
		LaserTargetLocation = Hit.Location; // Target blocking, so laser stops there
	}
	else
	{
		LaserTargetLocation = TraceEnd; // No target blocking, laser goes on
	}

	//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.0f); // Debug

	// Validate particle system
	if (LaserBeamFX)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "FX"); // Debug

		if (LaserBeamPSC)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "PSC"); // Debug

			// Activate Particle System Component and set its positions	
			LaserBeamPSC->SetActive(true, true);
			LaserBeamPSC->SetBeamSourcePoint(0, TraceStart, 0);
			LaserBeamPSC->SetBeamTargetPoint(0, LaserTargetLocation, 0);

			LaserPointPosition = LaserTargetLocation;
			LaserPointRotation = Hit.ImpactNormal.Rotation();
		}

	}
}


// Reloads ammo
void ACarnievilPrototypeCharacter::OnReload()
{

	// Gun is already full, character is locking on, or already reloading
	if (currentAmmo == ammoMagazineSize || isReloading)
		return;


	// Game mode for unlimited magazines variable
	ACarnievilPrototypeGameMode* gm = Cast<ACarnievilPrototypeGameMode>(GetWorld()->GetAuthGameMode());

	// Check if magazines available
	if (currentMagazines > 0 || gm->isUnlimitedMags){
		isReloading = true;

		// Sets timer for reload end 
		GetWorldTimerManager().SetTimer(ReloadTimerHandle, this,
			&ACarnievilPrototypeCharacter::ReloadEnd, reloadTime, false);
	}

}


/* Stops reloading */
void ACarnievilPrototypeCharacter::ReloadEnd()
{
	//Reload successful
	currentAmmo = ammoMagazineSize;
	currentMagazines--;

	isReloading = false;
}


/* Shooting ends and is enabled once more */
void ACarnievilPrototypeCharacter::ShotEnd()
{
	isShooting = false;
}


/** Character pushes enemy standing in front of it */
void ACarnievilPrototypeCharacter::OnPush()
{
	// Can't push
	if (isPushing || isReloading || isShooting)
		return;

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "PUSH"); // Debug

	// Character is now pushing
	isPushing = true;

	// Prepare for trace
	const FVector Start = GetActorLocation() + GetActorForwardVector() * 50;
	const FVector End = GetActorLocation() + GetActorForwardVector() * 100;
	const float Radius = 50.0f;
	FHitResult Hit;

	// Collision parameters
	FCollisionQueryParams TraceParams(FName(TEXT("Collision param")), true);
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.AddIgnoredActor(this);

	// Shows trace
	/*const FName TraceTag("PushTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	TraceParams.TraceTag = TraceTag;*/

	// Traces sphere
	GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat(), ECC_GameTraceChannel1,
		FCollisionShape::MakeSphere(Radius), TraceParams);

	// Gets actor that was hit
	APawn* actor = Cast<APawn>(Hit.GetActor());
	if (actor){ // If hit something
		// Check whether hit actor is an enemy
		AAIBaronSamediPawn* enemy = Cast<AAIBaronSamediPawn>(actor);

		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Hit something");

		if (enemy){
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Hit Guy");
			OnImplementPush(enemy);
		}
	}


	// Sets timer for push end 
	GetWorldTimerManager().SetTimer(PushingTimerHandle, this,
		&ACarnievilPrototypeCharacter::OnPushEnd, pushIntervalTime, false);

}

/** Character has already pushed an enemy standing in front of it */
void ACarnievilPrototypeCharacter::OnPushEnd()
{
	isPushing = false;
}

// Returns character's stress level based on actual stress and calculation 
float ACarnievilPrototypeCharacter::GetStressLevel()
{
	// Apply randomness to stress
	float stressLevel = currentStress + FMath::RandRange(-5, 5);

	// Checks to avoid negative values
	if (stressLevel < 0)
		stressLevel = 0;

	return stressLevel * 0.01; // Returns it as a percentage
}

// Applies a specific amount of stress on the character
void ACarnievilPrototypeCharacter::ApplyStress(float stress)
{
	// If stress is off, do nothing
	ACarnievilPrototypeGameMode* gm = Cast<ACarnievilPrototypeGameMode>(GetWorld()->GetAuthGameMode());
	if (gm->isSuperHigh)
		return;

	// Apply stress
	currentStress += stress;

	// Stress cannot be more than the maximum stress allowed
	if (currentStress > maxStress){
		currentStress = maxStress;
	}

	// Stress can't be less than 0
	if (currentStress < 0){
		currentStress = 0;
	}
}


// If character is too stressed, character may trip
void ACarnievilPrototypeCharacter::MayTrip()
{
	// If locked on, not sprinting, or already down, does not trip
	if (isLockedOn || !isSprinting || isDown){
		return;
	}

	isSprinting = false;
	isDown = true; // Character is now down
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Player Down"); // Debug

	// Sets timer for standing up 
	GetWorldTimerManager().SetTimer(StandUpTimerHandle, this,
		&ACarnievilPrototypeCharacter::StandUp, standUpTime, false);

}

// Character stands up after tripping
void ACarnievilPrototypeCharacter::StandUp()
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Player Up");
	isDown = false;
}

// Snaps camera to face wherever the player is facing
void ACarnievilPrototypeCharacter::CamSnap(){
	camSnapping = true;

	FRotator* targetCamRotation = new FRotator(-20, 0, 0);
	snapRotTarget = *targetCamRotation + GetActorRotation();

}

// Stops camera snap
void ACarnievilPrototypeCharacter::CamSnapReleased()
{
	camSnapping = false;
}


// Disables and stops sprinting
void ACarnievilPrototypeCharacter::SprintDeactivation()
{
	maySprint = false;
	OnSprintExit();
}

// Reenables sprinting
void ACarnievilPrototypeCharacter::SprintActivation()
{
	maySprint = true;
}


// Toggles reticle's following of locked on enemies
void ACarnievilPrototypeCharacter::OnLockOnFollowToggle()
{
	lockOnFollow = !lockOnFollow;
}

// Hit function for tripping
void ACarnievilPrototypeCharacter::OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	// Casts to verify it is an enemy
	AAIBaronSamediPawn* enemy = Cast<AAIBaronSamediPawn>(OtherActor);
	if (enemy){
		// Enemy is alive
		if (enemy->m_health > 0){
			// Character trips
			MayTrip();
		}
		
	}
}