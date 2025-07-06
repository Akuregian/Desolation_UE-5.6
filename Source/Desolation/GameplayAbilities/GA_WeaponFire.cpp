// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_WeaponFire.h"
#include "Abilities/GameplayAbility.h"
#include "Character/ShooterCharacter.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UGA_WeaponFire::UGA_WeaponFire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	StartFiringTag = FGameplayTag::RequestGameplayTag(FName("Survival.Ability.Weapon.StartFiring"));
	EndFiringTag = FGameplayTag::RequestGameplayTag(FName("Survival.Ability.Weapon.StopFiring"));

	// Fire tag
	AbilityTags.AddTag(StartFiringTag);
}

void UGA_WeaponFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// Start Firing if Shooter is valid
	if (AShooterCharacter* Shooter = Cast<AShooterCharacter>(GetActorInfo().AvatarActor))
	{
		Shooter->DoStartFiring();
	}

	// Wait for the StopFiring gameplay event to end the ability
	UAbilityTask_WaitGameplayEvent* WaitStop =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EndFiringTag, nullptr, /*OnlyOnce=*/true);

	WaitStop->EventReceived.AddDynamic(this, &UGA_WeaponFire::UGA_WeaponFire::StopFiringEventReceived);
	WaitStop->ReadyForActivation();
}

void UGA_WeaponFire::FireWeapon()
{
	if (AShooterCharacter* Shooter = Cast<AShooterCharacter>(GetActorInfo().AvatarActor))
	{
		Shooter->DoStartFiring();
	}
}

void UGA_WeaponFire::StopFiringEventReceived(FGameplayEventData Payload)
{
	// 1) Stop firing
	if (AShooterCharacter* Shooter = Cast<AShooterCharacter>(GetActorInfo().AvatarActor))
	{
		Shooter->DoStopFiring();
	}

	// 2) End the ability so it can be pressed again
	EndAbility(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActorInfo(),
		GetCurrentActivationInfo(),
		/*bReplicateEndAbility=*/true,
		/*bWasCancelled=*/false
	);
}