// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp.h"
#include "GameFramework/Character.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>

// Sets default values
APickUp::APickUp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MyMesh"));
	MyMesh->SetSimulatePhysics(true);
	RootComponent = MyMesh;

	bHolding = false;
	bGravity = true;
}

// Called when the game starts or when spawned
void APickUp::BeginPlay()
{
	Super::BeginPlay();
	
	MyCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	PlayerCamera = MyCharacter->FindComponentByClass<UCameraComponent>();

	TArray<USceneComponent*> Components;

	MyCharacter->GetComponents(Components);

	if (Components.Num() > 0) 
	{
		for (auto& Comp : Components) 
		{
			if (Comp->GetName() == "HoldingComponent") 
			{
				HoldingComp = Cast<USceneComponent>(Comp);

			}
		}
	}
}

// Called every frame
void APickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bHolding && HoldingComp) 
	{
		//SetActorLocationAndRotation(HoldingComp->GetComponentLocation(), HoldingComp->GetComponentRotation());
		SetActorLocation(HoldingComp->GetComponentLocation());
		
	}
}

void APickUp::RotateActor() 
{
	ControlRotation = GetWorld()->GetFirstPlayerController()->GetControlRotation();
	SetActorRotation(FQuat(ControlRotation));
}

void APickUp::PickUp() 
{
	bHolding = !bHolding;
	if (bHolding)
		GLog->Log("true");
	else
		GLog->Log("false");
	bGravity = !bGravity;
	MyMesh->SetEnableGravity(bGravity);
	MyMesh->SetSimulatePhysics(bHolding ? false : true); // If bHolding is true it's gonna be false and if it's false then it's false
	MyMesh->SetCollisionEnabled(bHolding ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);

	if (!bHolding) 
	{
		ForwadVector = PlayerCamera->GetForwardVector();
		MyMesh->AddForce(ForwadVector * 100000 * MyMesh->GetMass());
	}
}