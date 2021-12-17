// Fill out your copyright notice in the Description page of Project Settings.


#include "Health.h"
#include "GameFramework/GameModeBase.h"


// Sets default values for this component's properties
UHealth::UHealth()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	DefaultHealth = 100;
	Health = DefaultHealth;
}


// Called when the game starts
void UHealth::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();

	if (Owner) 
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealth::TakeDamage);
	}
	
}

void UHealth::TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0) 
	{
		return;
	}

	if (Health <= 0)
	{
		DamagedActor->AActor::Destroy(false, true);
		GetWorld()->GetAuthGameMode()->RestartPlayer(GetWorld()->GetFirstPlayerController());
	}
	
	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

}





