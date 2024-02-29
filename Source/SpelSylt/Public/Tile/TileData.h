// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TileData.generated.h"

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ETileExits : uint8 {
	None	= 0			UMETA(Hidden),
	North	= 1 << 0,
	East	= 1 << 1,
	South	= 1 << 2,
	West	= 1 << 3,
};
ENUM_CLASS_FLAGS(ETileExits);

USTRUCT(BlueprintType)
struct SPELSYLT_API FTileData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> Level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "/Script/Spelsylt.ETileExits"))
	int32 Exits = 0;
};