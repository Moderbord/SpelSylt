// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGoblinTerrain.h"
#include "DCVolumeGenerator/DCVolumeGenerator.h"

// Sets default values
AWorldGoblinTerrain::AWorldGoblinTerrain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AWorldGoblinTerrain::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWorldGoblinTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWorldGoblinTerrain::Regenerate()
{
	FVector3f location = FVector3f(GetActorLocation());

	FDCVolumeGeneratorDispatchParams Params(this, 32, Generator, location);

	Params.VolumeScale = VolumeScale;
	Params.MaxCornerDistance = MaxCornerDistance;
	Params.CenterBias = CenterBias;
	Params.ClampRange = ClampRange;

	FDCVolumeGeneratorInterface::Dispatch(Params, 
		[this](bool Success, UDCVolumeResult* Result) 
		{
			if (Success)
			{
				Volume = Result;
				OnRegenerateFinished();
			}
		});
}

