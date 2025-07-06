// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TimerManager.h"
#include "GA_WeaponFire.generated.h"

/**
 * 
 */
UCLASS()
class DESOLATION_API UGA_WeaponFire : public UGameplayAbility
{
	GENERATED_BODY()

public:
	
	UGA_WeaponFire();

	// -------- UGameplayAbility override functions -------
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	// ---------------------------------------------------
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameplayTag")
	FGameplayTag StartFiringTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameplayTag")
	FGameplayTag EndFiringTag;
	
	/** Called by our timer to fire repeatedly */
	void FireWeapon();
	
	/** Called when we receive the StopFiring gameplay event */
	UFUNCTION()
	void StopFiringEventReceived(FGameplayEventData Payload);

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	
};
