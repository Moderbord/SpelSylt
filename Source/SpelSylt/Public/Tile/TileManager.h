// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TileData.h"
#include "TileManager.generated.h"

UCLASS()
class SPELSYLT_API ATileManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATileManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure)
	bool TileMatches(FIntPoint Tile, UPARAM(meta = (Bitmask, BitmaskEnum = ETileExits)) int32 Bitmask) const;

	UFUNCTION(BlueprintPure)
	bool TileHasExit(FIntPoint Tile, ETileExits Exit) const;

	UFUNCTION(BlueprintCallable)
	int32 GetRequiredExits(FIntPoint Pos) const;

	UFUNCTION(BlueprintCallable)
	int32 GetForbiddenExits(FIntPoint Pos) const;

	UFUNCTION(BlueprintCallable)
	void GenerateAt(FIntPoint Pos);

	UFUNCTION(BlueprintCallable)
	void CullAt(FIntPoint Pos, int32 Range = 1);

	UFUNCTION(BlueprintCallable)
	void KeepAt(FIntPoint Pos, int32 Range = 1);

	UFUNCTION(BlueprintCallable)
	void PerformGeneratePass();

	UFUNCTION(BlueprintCallable)
	void PerformCullingPass();

	UFUNCTION(BlueprintCallable)
	void PlaceTile(FIntPoint Pos, int32 Rot, FTileData Data);

	UFUNCTION(BlueprintCallable)
	void RemoveTile(FIntPoint Pos);

	UFUNCTION(BlueprintCallable)
	bool TileExists(FIntPoint Tile) const;

	UFUNCTION(BlueprintCallable)
	FIntPoint ActorToPoint(const AActor* Actor) const;
	
	static void Rotate(int32& Bitmask);
	static bool CheckTile(const FTileData* Tile, int32 Bitmask);
	static bool CheckMask(int32 Mask1, int32 Mask2);
	static int32 GetTileType(const FTileData* Tile);

	FTileData* GetRandomCompatibleTile(int32 Exits, int32 Forbids, int32& OutConfig, bool Force=false);


public:

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TMap<FIntPoint, class ULevelStreamingDynamic*> TileMap;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TMap<FIntPoint, int32> TileExitMap;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TObjectPtr<UDataTable> TileData;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	FDataTableRowHandle StartTile;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TArray<AActor*> TrackedActors;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TSet<FIntPoint> CullCandidates;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	float TileSize;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<USceneComponent> Root;


protected:

};
