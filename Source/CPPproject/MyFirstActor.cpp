// Fill out your copyright notice in the Description page of Project Settings.


#include "MyFirstActor.h"
#include <CPPproject/CPPprojectCharacter.h>

// Sets default values
AMyFirstActor::AMyFirstActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene")); // On cree le scene component en definissant son type et create defaut return un pointeur
	RootComponent = SceneComponent; //RootComponent définie la position de l'objet et c'est un pointeur aussi

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(SceneComponent);

	

	

	LetterCount = 10;
}

// Called when the game starts or when spawned
void AMyFirstActor::BeginPlay()
{
	Super::BeginPlay();
	MeshComponent->OnComponentHit.AddDynamic(this, &AMyFirstActor::OnHit);

}

// Called every frame
void AMyFirstActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//GLog->Log("tick");

}

void AMyFirstActor::PrintLetterCount() 
{
	GLog->Log("LetterCount = " + FString::FromInt(LetterCount));
}

void AMyFirstActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	ACPPprojectCharacter* Character = Cast<ACPPprojectCharacter>(OtherActor);
	if (Character == nullptr)
		return;

	Character->LaunchCharacter(FVector(0, 0, 1000), false, true);
}

