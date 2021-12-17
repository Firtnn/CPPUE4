// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet.h"

// Sets default values
ABullet::ABullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    ProjectileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMeshComponent"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh>Mesh(TEXT("StaticMesh'/Game/StarterContent/Props/MaterialSphere.MaterialSphere'"));
    if (Mesh.Succeeded())
    {
        ProjectileMeshComponent->SetStaticMesh(Mesh.Object);
        ProjectileMeshComponent->SetRelativeLocation(FVector(50.f, 0.f, 0.f));
        ProjectileMeshComponent->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
    }

    RootComponent = ProjectileMeshComponent;

    
    
       // Use this component to drive this projectile's movement.
       ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
       ProjectileMovementComponent->SetUpdatedComponent(RootComponent);
       ProjectileMovementComponent->InitialSpeed = 3000.0f;
       ProjectileMovementComponent->MaxSpeed = 3000.0f;
       ProjectileMovementComponent->bRotationFollowsVelocity = true;
       ProjectileMovementComponent->bShouldBounce = false;
       ProjectileMovementComponent->Bounciness = 0.3f;
       ProjectileMovementComponent->ProjectileGravityScale = 0.5f;
    
}

// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();
	ProjectileMeshComponent->OnComponentHit.AddDynamic(this, &ABullet::OnHit);

}

// Called every frame
void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) 
{
    ADecalActor* Decal = GetWorld()->SpawnActor<ADecalActor>(Hit.Location, FRotator());
    Decal->SetLifeSpan(2.5f);
    Decal->GetDecal()->DecalSize = FVector(22.f, 64.f, 64.f);

    Destroy();
}
