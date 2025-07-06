// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputAction.h"
#include "GameplayTagContainer.h"
#include "AbilityInputActionBinding.generated.h"

/**
 * Struct mapping a UInputAction to a GameplayTag for ability activation.
 */
USTRUCT(BlueprintType)
struct FAbilityInputBinding
{
	GENERATED_BODY()

	/**
	 * The input action that triggers the ability (Enhanced Input Action).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* InputAction;

	/**
	 * The gameplay tag corresponding to the ability input (e.g. "Ability.Weapon.Fire").
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	FGameplayTag InputTag;
};

UCLASS(BlueprintType)
class DESOLATION_API UAbilityInputActionBinding : public UDataAsset
{
	GENERATED_BODY()

public:
	
	// Array of InputAction-to-GameplayTag bindings for abilities. 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TArray<FAbilityInputBinding> AbilityInputActions;
};
