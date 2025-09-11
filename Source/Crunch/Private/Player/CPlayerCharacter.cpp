// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputcomponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFrameWork/CharacterMovementcomponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

ACPlayerCharacter::ACPlayerCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("Camera Boom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation = true;

	ViewCam = CreateDefaultSubobject<UCameraComponent>("View Cam");
	ViewCam->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
}

void ACPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	APlayerController* OwningPlayerController = GetController<APlayerController>();

	if (OwningPlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = OwningPlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (InputSubsystem)
		{
			InputSubsystem->RemoveMappingContext(GameplayInputMappingContext);
			InputSubsystem->AddMappingContext(GameplayInputMappingContext, 0);
		}
	}
}

void ACPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComp)
	{
		EnhancedInputComp->BindAction(JumpInputAction, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Jump);
		EnhancedInputComp->BindAction(LookInputAction, ETriggerEvent::Triggered, this, &ACPlayerCharacter::HandleLookInputAction);
		EnhancedInputComp->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &ACPlayerCharacter::HandleMoveInputAction);
	}

}

void ACPlayerCharacter::HandleLookInputAction(const FInputActionValue& InputAction)
{
	FVector2D InputVal = InputAction.Get<FVector2D>();

	AddControllerYawInput(InputVal.X);
	AddControllerPitchInput(-InputVal.Y);
}

void ACPlayerCharacter::HandleMoveInputAction(const FInputActionValue& InputAction)
{
	FVector2D InputValue = InputAction.Get<FVector2D>();

	InputValue.Normalize();

	AddMovementInput(GetMovefwdDir() * InputValue.Y + GetLookRightDir() * InputValue.X);
}

FVector ACPlayerCharacter::GetLookRightDir() const
{
	return ViewCam->GetRightVector();
}

FVector ACPlayerCharacter::GetLookfwdDir() const
{
	return ViewCam->GetForwardVector();
}

FVector ACPlayerCharacter::GetMovefwdDir() const
{
	return FVector::CrossProduct(GetLookRightDir(), FVector::UpVector);
}




