// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestTriangleShaderActor.generated.h"

UCLASS(hidecategories = (Cooking, Input, LOD), MinimalAPI)
class ATestTriangleShader : public AActor
{
	GENERATED_UCLASS_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UTestTriangleShaderComponent* TestTriangleShaderComponent;

protected:
};
