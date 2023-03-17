// Copyright Epic Games, Inc. All Rights Reserved.

#include "CauldronGenWorld/Public/TestTriangleShader/TestTriangleShaderActor.h"
#include "CauldronGenWorld/Public/TestTriangleShader/TestTriangleShaderComponent.h"

ATestTriangleShader::ATestTriangleShader(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootComponent = TestTriangleShaderComponent = CreateDefaultSubobject<UTestTriangleShaderComponent>(TEXT("TestTriangleShaderComponent"));
}
