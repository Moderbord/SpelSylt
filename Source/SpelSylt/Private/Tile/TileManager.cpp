// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile/TileManager.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/LevelStreamingDynamic.h"

// Sets default values
ATileManager::ATileManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

}

// Called when the game starts or when spawned
void ATileManager::BeginPlay()
{
	Super::BeginPlay();

	UGameplayStatics::GetAllActorsWithTag(GetWorld(), "Anchor", TrackedActors);

	FTileData* row = StartTile.GetRow<FTileData>("Failed to get StartTile!");
	if (row)
		PlaceTile(FIntPoint(0), 0, *row);
}

// Called every frame
void ATileManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	PerformGeneratePass();
	PerformCullingPass();

}

bool ATileManager::TileMatches(FIntPoint Tile, int32 Bitmask) const
{
	const int32* exits = TileExitMap.Find(Tile);

	return exits ? (bool)((Bitmask & *exits) == Bitmask) : false;
}

bool ATileManager::TileHasExit(FIntPoint Tile, ETileExits Exit) const
{
	const int32* exits = TileExitMap.Find(Tile);

	return exits ? (bool)(*exits & (int32)Exit) : false;
}

int32 ATileManager::GetRequiredExits(FIntPoint Pos) const
{
	//const FIntPoint directions[] = { {0,-1}, {1,0}, {0,1}, {-1, 0} };
	const FIntPoint directions[] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
	const ETileExits cardinals[] = { ETileExits::North, ETileExits::East, ETileExits::South, ETileExits::West };
	const ETileExits opposites[] = { ETileExits::South, ETileExits::West, ETileExits::North, ETileExits::East };

	int32 exits = 0;

	for (int32 i = 0; i < 4; i++)
	{
		FIntPoint point = Pos + directions[i];
		ETileExits exit = opposites[i];

		if (TileHasExit(point, exit))
			exits |= (int32)cardinals[i];
	}

	return exits;
}

int32 ATileManager::GetForbiddenExits(FIntPoint Pos) const
{
	const FIntPoint directions[] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
	const ETileExits cardinals[] = { ETileExits::North, ETileExits::East, ETileExits::South, ETileExits::West };
	const ETileExits opposites[] = { ETileExits::South, ETileExits::West, ETileExits::North, ETileExits::East };

	int32 forbids = 0;

	for (int32 i = 0; i < 4; i++)
	{
		FIntPoint point = Pos + directions[i];
		ETileExits exit = opposites[i];

		if (!TileExists(point))
			continue;

		if (!TileHasExit(point, exit))
			forbids |= (int32)cardinals[i];
	}

	return forbids;
}

void ATileManager::GenerateAt(FIntPoint Pos)
{
	const FIntPoint order[] =
	{
		{0, 0},
		{1, 0}, {0, 1}, {-1, 0}, {0, -1},
		{-1, -1}, {1, -1}, {-1, 1}, {1, 1}
	};

	for (auto dir : order)
	{
		FIntPoint pos = Pos + dir;

		if (TileMap.Contains(pos))
			continue;

		int32 exits = GetRequiredExits(pos);
		int32 forbids = GetForbiddenExits(pos);

		int32 rot = 0;
		FTileData* data = GetRandomCompatibleTile(exits, forbids, rot, pos == FIntPoint(0));
		
		UE_LOG(LogTemp, Log, TEXT("Tile at (%i, %i): Exits = %i (%i forbidden), Rot = %i"), pos.X, pos.Y, exits, forbids, rot);

		if (data)
			PlaceTile(pos, rot, *data);
	}
}

void ATileManager::CullAt(FIntPoint Pos, int32 Range)
{
	for (auto it = TileMap.CreateConstIterator(); it; ++it)
	{
		FIntPoint pos = it.Key();
		int32 dist = (Pos - pos).Size();
		
		if (dist > Range)
			CullCandidates.Add(pos);
	}
}

void ATileManager::KeepAt(FIntPoint Pos, int32 Range)
{
	for (auto it = TileMap.CreateConstIterator(); it; ++it)
	{
		FIntPoint pos = it.Key();
		int32 dist = (Pos - pos).Size();

		if (dist <= Range)
			CullCandidates.Remove(pos);
	}
}

void ATileManager::PerformGeneratePass()
{
	for (auto& actor : TrackedActors)
	{
		FIntPoint point = ActorToPoint(actor);
		GenerateAt(point);
	}
		
}

void ATileManager::PerformCullingPass()
{
	CullCandidates.Empty();

	for (int32 i = TrackedActors.Num() - 1; i >= 0; i--)
	{
		AActor* actor = TrackedActors[i];

		if (!IsValid(actor))
			TrackedActors.RemoveAt(i);
	}

	for (auto& actor : TrackedActors)
	{
		FIntPoint point = ActorToPoint(actor);
		CullAt(point);
	}

	for (auto& actor : TrackedActors)
	{
		FIntPoint point = ActorToPoint(actor);
		KeepAt(point);
	}

	for (auto point : CullCandidates)
		RemoveTile(point);
}

void ATileManager::PlaceTile(FIntPoint Pos, int32 Rot, FTileData Data)
{
	const FVector Trans = FVector(Pos, 0.f) * TileSize;
	const FRotator Rotation = FRotator(0.f, (float)Rot * -90.f, 0.f);
	const FTransform Transform = FTransform(Rotation, Trans);

	bool success = false;
	ULevelStreamingDynamic* level = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(this, Data.Level, Transform, success);
	
	// A little hacky -- but if the tile has been rotated, rotate the exit mask as well.
	int32 mask = Data.Exits;
	for (int32 i = 0; i < Rot; i++)
		Rotate(mask);

	if (success)
	{
		TileMap.Add(Pos, level);
		TileExitMap.Add(Pos, mask);
	}
}

void ATileManager::RemoveTile(FIntPoint Pos)
{
	ULevelStreamingDynamic** level = TileMap.Find(Pos);
	if (level)
	{
		(*level)->SetIsRequestingUnloadAndRemoval(true);
		TileMap.Remove(Pos);
		TileExitMap.Remove(Pos);
	}
}

bool ATileManager::TileExists(FIntPoint Tile) const
{
	return TileMap.Contains(Tile);
}

FIntPoint ATileManager::ActorToPoint(const AActor* Actor) const
{
	FVector loc = Actor->GetActorLocation() / TileSize;
	return FIntPoint(FMath::RoundToInt(loc.X), FMath::RoundToInt(loc.Y));
}

void ATileManager::Rotate(int32& Bitmask)
{
	int32 temp = (Bitmask >> 1) | ((Bitmask << 3) & 15);
	Bitmask = temp;
}

bool ATileManager::CheckTile(const FTileData* Tile, int32 Bitmask)
{
	return (bool)((Bitmask & Tile->Exits) == Bitmask);
}

bool ATileManager::CheckMask(int32 Mask1, int32 Mask2)
{
	return (bool)((Mask2 & Mask1) == Mask2);
}

int32 ATileManager::GetTileType(const FTileData* Tile)
{
	return 0;
}

FTileData* ATileManager::GetRandomCompatibleTile(int32 Exits, int32 Forbids, int32& OutConfig, bool Force)
{
	const int maxTries = 100;

	if (!TileData)
		return nullptr;

	TArray<FTileData*> out;
	TileData->GetAllRows<FTileData>("Error in TileData lookup!", out);

	if (out.Num() == 0)
		return nullptr;

	// Crash prevention
	for (int32 i = 0; i < maxTries; i++)
	{
		FTileData* test = out[FMath::RandRange(0, out.Num() - 1)];
		int32 tileExits = test->Exits;

		// Rotation
		for (int32 r = 0; r < 4; r++)
		{
			OutConfig = r;

			if (Force)
				return test;

			if (Exits == 0)
			{
				if (tileExits == 0)
					return test;

				goto end_test; // The bitwise test will return true for all 0-masks, so we skip these.
			}

			if (Forbids != 0 && CheckMask(tileExits, Forbids))
			{
				goto end_test;
			}

			if (CheckMask(tileExits, Exits))
			{
				return test;
			}
			
			end_test:

			Rotate(tileExits);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Tile selector timed out! Exits = %i, Forbids = %i, Rot = %i"), Exits, Forbids, OutConfig);

	return nullptr;
}

