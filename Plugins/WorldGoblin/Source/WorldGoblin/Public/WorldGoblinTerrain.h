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

	UFUNCTION(BlueprintImplementableEvent)
	void OnRegenerateFinished();

public:

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Generator")
	TObjectPtr<UMaterialInterface> Generator;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 VolumeSize = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VolumeScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxCornerDistance = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CenterBias = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ClampRange = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NormalSampleStep = 0.0001f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector3f Result = FVector3f(0.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UDCVolumeResult* Volume;

protected:


};
