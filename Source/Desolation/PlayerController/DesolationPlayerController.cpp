// Copyright Epic Games, Inc. All Rights Reserved.


#include "DesolationPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "CameraManager/DesolationCameraManager.h"

ADesolationPlayerController::ADesolationPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = ADesolationCameraManager::StaticClass();
}

void ADesolationPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}
