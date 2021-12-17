// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/World.h"
#include "CPPprojectCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"


//////////////////////////////////////////////////////////////////////////
// ACPPprojectCharacter

ACPPprojectCharacter::ACPPprojectCharacter()
{
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
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	HoldingComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HoldingComponent"));
	HoldingComponent->SetRelativeLocation(FVector(100.0f, 0.0f, 0.0f));
	//HoldingComponent->SetupAttachement();

	CurrentItem = NULL;
	bCanMove = true;
	bInspecting = false;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACPPprojectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &ACPPprojectCharacter::SpawnObject);

	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ACPPprojectCharacter::OnStartRun);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &ACPPprojectCharacter::OnStopRun);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACPPprojectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACPPprojectCharacter::MoveRight);

	PlayerInputComponent->BindAction("Action", IE_Pressed, this, &ACPPprojectCharacter::OnAction);

	PlayerInputComponent->BindAction("Inspect", IE_Pressed, this, &ACPPprojectCharacter::OnInspect);
	PlayerInputComponent->BindAction("Inspect", IE_Released, this, &ACPPprojectCharacter::OnInspectReleased);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ACPPprojectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ACPPprojectCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ACPPprojectCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ACPPprojectCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ACPPprojectCharacter::OnResetVR);
}


void ACPPprojectCharacter::BeginPlay()
{
	Super::BeginPlay(); 

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	//PitchMax = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMax;
	//PitchMin = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMin;
}

void ACPPprojectCharacter::Tick(float  DeltaTime) 
{
	Super::Tick(DeltaTime);

	Start = HoldingComponent->GetComponentLocation();
	ForwardVector = CameraBoom->GetForwardVector();
	End = ((ForwardVector * 200.0f) + Start);

	DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 1, 0, 1);

	if (!bHoldingItem)
	{
		//if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, DefaultComponentQueryParams, DefaultResponseParams)) // to check if we hit something
		//{
		//	GLog->Log("CaTouche");
		//	if (Hit.GetActor()->GetClass()->IsChildOf(APickUp::StaticClass())) // to check if the hit is a pickupClass
		//	{
		//		CurrentItem = Cast<APickUp>(Hit.GetActor());
		//	}
		//}	

		if (GetWorld()->UWorld::LineTraceSingleByObjectType(Hit, Start, End, DefaultComponentQueryParams, DefaultResponseParams)) // to check if we hit something
		{
			if (Hit.GetActor()->GetClass()->IsChildOf(APickUp::StaticClass())) // to check if the hit is a pickupClass
			{
				//GLog->Log("OH OUI");
				CurrentItem = Cast<APickUp>(Hit.GetActor());
	
			}
		}
	}
	else 
	{
		CurrentItem = NULL;
	}

	if (bInspecting)
	{
		if (bHoldingItem)
		{
			//JSP COMMENT FAIRE AVEC UN THIRDPERSON Je marque quand meme ce qu'il fait dans le tuto en commentaire
			//FirstPersonCameraComponent->SetFieldOfView(FMath::Lerp(FirstPersonCameraComponent->FieldOfView, 90.0f, 0.1f))
			//HoldingComponent->SetRelativeLocation(FVector(0.0f, 50.0f, 50.0f));
			CurrentItem->RotateActor();
		}
	}
	else 
	{
		if (bHoldingItem) 
		{
			HoldingComponent->SetRelativeLocation(FVector(100.0f, 0.0f, 0.0f));
		}
	}
}

void ACPPprojectCharacter::OnStartRun()
{
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void ACPPprojectCharacter::OnStopRun()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ACPPprojectCharacter::SpawnObject()
{
	
	FVector Loc = HoldingComponent->GetComponentLocation();
	FRotator Rot = GetActorRotation();
	FActorSpawnParameters SpawnParams;

	//GetController()->GetPlayerViewPoint(Loc, Rot);

	AActor* SpawnedActorRef = GetWorld()->SpawnActor<AActor>(ActorToSpawn, Loc, Rot, SpawnParams);


}

void ACPPprojectCharacter::OnResetVR()
{
	// If CPPproject is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in CPPproject.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ACPPprojectCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ACPPprojectCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void ACPPprojectCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACPPprojectCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACPPprojectCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ACPPprojectCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ACPPprojectCharacter::OnAction() 
{
	if (CurrentItem && !bInspecting) 
	{
		ToggleItemPickup();
	}
}

void ACPPprojectCharacter::OnInspect()
{
	if (bHoldingItem) 
	{
		LastRotation = GetControlRotation();
	}
	else 
	{
		bInspecting = true;
	}
}

void ACPPprojectCharacter::OnInspectReleased() 
{
	if (bInspecting && bHoldingItem) 
	{
		GetController()->SetControlRotation(LastRotation);
		//GetWorld()->GetFirstPlayerController()->PlayerCamera
	}
	else 
	{
		bInspecting = false;
	}
}

void ACPPprojectCharacter::ToggleItemPickup()
{
	if (CurrentItem) 
	{
		bHoldingItem = !bHoldingItem;
		CurrentItem->PickUp();


		if (!bHoldingItem) 
		{
			CurrentItem = NULL;
		}
	}
}