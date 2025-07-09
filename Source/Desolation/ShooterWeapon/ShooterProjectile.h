// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "ShooterProjectile.generated.h"

class UGameplayEffect;
class USphereComponent;
class UProjectileMovementComponent;
class ACharacter;

/**
 *  Simple projectile class for a first person shooter game
 */
UCLASS(abstract)
class DESOLATION_API AShooterProjectile : public AActor
{
	GENERATED_BODY()

	/** Handles movement for the projectile */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

protected:

	/** Loudness of the AI perception noise done by this projectile on hit */
	UPROPERTY(EditAnywhere, Category="Noise")
	float NoiseLoudness = 3.0f;

	/** Range of the AI perception noise done by this projectile on hit */
	UPROPERTY(EditAnywhere, Category="Noise")
	float NoiseRange = 3000.0f;

	/** Tag of the AI perception noise done by this projectile on hit */
	UPROPERTY(EditAnywhere, Category="Noise")
	FName NoiseTag = FName("Projectile");

	/** Physics force to apply on hit */
	UPROPERTY(EditAnywhere, Category="Hit")
	float PhysicsForce = 100.0f;

	/** Damage to apply on hit */
	UPROPERTY(EditAnywhere, Category="Hit")
	float HitDamage = 25.0f;

	/** Type of damage to apply. Can be used to represent specific types of damage such as fire, explosion, etc. */
	UPROPERTY(EditAnywhere, Category="Hit")
	TSubclassOf<UDamageType> HitDamageType;

	UPROPERTY(EditDefaultsOnly, Category="Hit")
	TSubclassOf<UGameplayEffect> BulletDamageGE;

	UPROPERTY(EditDefaultsOnly, Category="Hit")
	FGameplayTag DamageTag;
	
	/** If true, this projectile has already hit another surface */
	bool bHit = false;

public:	

	/** Constructor */
	AShooterProjectile();
	
	/** Provides collision detection for the projectile */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* CollisionComponent;

protected:
	
	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Handles collision */
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

protected:

	/** Apply damage to a hit character */
	UFUNCTION(BlueprintCallable, Category="Projectile")
	virtual void DamageCharacter(ACharacter* HitCharacter, const FHitResult& Hit);

	/** Passes control to Blueprint to implement any effects on hit */
	UFUNCTION(BlueprintImplementableEvent, Category="Projectile", meta=(DisplayName = "On Projectile Hit"))
	void BP_OnProjectileHit(const FHitResult& Hit);
};
