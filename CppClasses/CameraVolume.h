// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "FixedCamera.h"
#include "CameraVolume.generated.h"

UCLASS()
class CARNIEVILPROTOTYPE_API ACameraVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACameraVolume();

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

	// Fixed Camera associated with this volume
	UPROPERTY(EditAnywhere)
		AFixedCamera* camera;

private:
	// Collider
	UPROPERTY(EditAnywhere)
		UBoxComponent* BoxCollider;

	// Speed of camera blending when switching
	UPROPERTY(EditAnywhere)
		float BlendSpeed = 0.5f;
	
};
