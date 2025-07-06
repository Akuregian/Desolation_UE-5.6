// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterCharacter.h"
#include "ShooterWeapon/ShooterWeapon.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "AbilitySystemComponent.h"

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

AShooterCharacter::AShooterCharacter(const FObjectInitializer& FObjectInitializer)
	: Super(FObjectInitializer)
{
	// create the noise emitter component
	PawnNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Pawn Noise Emitter"));

	// configure movement
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);

	// Create and Replicate ASC
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// TODO: Create (only) and add Attributes to protected variable in header..
	
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// base class handles move, aim and jump inputs
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (InputBindings)
		{
			for (const auto& Binding : InputBindings->AbilityInputActions)
			{
				EnhancedInputComponent->BindAction(
					Binding.InputAction,
					ETriggerEvent::Started,
					this,
					&AShooterCharacter::OnAbilityActivated,
					Binding.InputTag
				);

				EnhancedInputComponent->BindAction(
					Binding.InputAction,
					ETriggerEvent::Completed,
					this,
					&AShooterCharacter::OnAbilityEnded,
					Binding.InputTag
				);
			}
		}
	}
}

void AShooterCharacter::OnAbilityActivated(FGameplayTag InputTag)
{
	
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,FString::Printf(TEXT("ShooterCharacter Ability Activated")));
	// Activate the WeaponFire Gameplay Ability
	AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(InputTag));
}

void AShooterCharacter::OnAbilityEnded(FGameplayTag InputTag)
{
	// Broadcast explicit "StopFire" tag 
	const FGameplayTag StopFireTag =
		FGameplayTag::RequestGameplayTag(TEXT("Survival.Ability.Weapon.StopFiring"));
	FGameplayEventData EventData;
	EventData.EventTag = StopFireTag;
	AbilitySystemComponent->HandleGameplayEvent(StopFireTag, &EventData);
}

float AShooterCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// ignore if already dead
	if (CurrentHP <= 0.0f)
	{
		return 0.0f;
	}

	// Reduce HP
	CurrentHP -= Damage;

	// Have we depleted HP?
	if (CurrentHP <= 0.0f)
	{
		// deactivate the weapon
		if (IsValid(CurrentWeapon))
		{
			CurrentWeapon->DeactivateWeapon();
		}

		// reset the bullet counter UI
		OnBulletCountUpdated.Broadcast(0, 0);

		// destroy this character
		Destroy();
	}

	return Damage;
}

void AShooterCharacter::DoStartFiring()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,FString::Printf(TEXT("Firing")));
	// fire the current weapon
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFiring();
	}
}

void AShooterCharacter::DoStopFiring()
{
	// stop firing the current weapon
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFiring();
	}
}

void AShooterCharacter::DoSwitchWeapon()
{
	// ensure we have at least two weapons two switch between
	if (OwnedWeapons.Num() > 1)
	{
		// deactivate the old weapon
		CurrentWeapon->DeactivateWeapon();

		// find the index of the current weapon in the owned list
		int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);

		// is this the last weapon?
		if (WeaponIndex == OwnedWeapons.Num() - 1)
		{
			// loop back to the beginning of the array
			WeaponIndex = 0;
		}
		else {
			// select the next weapon index
			++WeaponIndex;
		}

		// set the new weapon as current
		CurrentWeapon = OwnedWeapons[WeaponIndex];

		// activate the new weapon
		CurrentWeapon->ActivateWeapon();
	}
}

void AShooterCharacter::AttachWeaponMeshes(AShooterWeapon* Weapon)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// attach the weapon actor
	Weapon->AttachToActor(this, AttachmentRule);

	// attach the weapon meshes
	Weapon->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	Weapon->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, FirstPersonWeaponSocket);
	
}

void AShooterCharacter::PlayFiringMontage(UAnimMontage* Montage)
{
	
}

void AShooterCharacter::AddWeaponRecoil(float Recoil)
{
	// apply the recoil as pitch input
	AddControllerPitchInput(Recoil);
}

void AShooterCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	OnBulletCountUpdated.Broadcast(MagazineSize, CurrentAmmo);
}

FVector AShooterCharacter::GetWeaponTargetLocation()
{
	// trace ahead from the camera viewpoint
	FHitResult OutHit;

	const FVector Start = GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector End = Start + (GetFirstPersonCameraComponent()->GetForwardVector() * MaxAimDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

	// return either the impact point or the trace end
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterCharacter::AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass)
{
	// do we already own this weapon?
	AShooterWeapon* OwnedWeapon = FindWeaponOfType(WeaponClass);

	if (!OwnedWeapon)
	{
		// spawn the new weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

		AShooterWeapon* AddedWeapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);

		if (AddedWeapon)
		{
			// add the weapon to the owned list
			OwnedWeapons.Add(AddedWeapon);

			// if we have an existing weapon, deactivate it
			if (CurrentWeapon)
			{
				CurrentWeapon->DeactivateWeapon();
			}

			// switch to the new weapon
			CurrentWeapon = AddedWeapon;
			CurrentWeapon->ActivateWeapon();
		}
	}
}

void AShooterCharacter::OnWeaponActivated(AShooterWeapon* Weapon)
{
	// update the bullet counter
	OnBulletCountUpdated.Broadcast(Weapon->GetMagazineSize(), Weapon->GetBulletCount());

	// set the character mesh AnimInstances
	GetFirstPersonMesh()->SetAnimInstanceClass(Weapon->GetFirstPersonAnimInstanceClass());
	GetMesh()->SetAnimInstanceClass(Weapon->GetThirdPersonAnimInstanceClass());

	// Grant the Fire Weapon Ability for this weapon
	if (AbilitySystemComponent && Weapon->GetWeaponFireAbility())
	{
		// 1. Create a spec with INDEX_NONE for InputID
		FGameplayAbilitySpec Spec(
			Weapon->GetWeaponFireAbility(),  // TSubclassOf<UGameplayAbility>
			/*Level=*/ 1,
			/*InputID=*/ 1,
			/*SourceObject=*/ Weapon
		);
		
		// 3. Give it to the ASC
		AbilitySystemComponent->GiveAbility(Spec);
	}
}

void AShooterCharacter::OnWeaponDeactivated(AShooterWeapon* Weapon)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ClearAllAbilities();
	}
}

void AShooterCharacter::OnSemiWeaponRefire()
{
	// unused
}

AShooterWeapon* AShooterCharacter::FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const
{
	// check each owned weapon
	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (Weapon->IsA(WeaponClass))
		{
			return Weapon;
		}
	}

	// weapon not found
	return nullptr;

}

void AShooterCharacter::DebugPrintTags()
{
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DebugPrintTags] No ASC found."));
		return;
	}

	// 1. Gather owned tags
	FGameplayTagContainer OwnedTags;
	AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);

	// 2. Convert to a single-line string
	//    e.g. "Survival.Ability.Weapon.StartFiring,Status.Poisoned"
	FString TagsString = OwnedTags.ToStringSimple();

	// 3. On-screen debug: use a constant message key so we don't spam multiple lines
	static const int32 DebugMsgKey = 1337;
	const float DisplayTime = 1.5f;              // seconds
	const FColor DisplayColor = FColor::Green;   // whatever you like

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			DebugMsgKey,
			DisplayTime,
			DisplayColor,
			FString::Printf(TEXT("ActiveTags: %s"), *TagsString)
		);
	}
}
