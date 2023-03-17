// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldGoblinTerrain.generated.h"

UCLASS()
class WORLDGOBLIN_API AWorldGoblinTerrain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWorldGoblinTerrain();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void Regenerate();

public:

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Generator")
	TObjectPtr<UMaterialInterface> Generator;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 VolumeSize = 32;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ChunkSize = 1000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector3f Result = FVector3f(0.f);

protected:


};
