// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorldGoblinShaders/Public/VolumeDensityOutputMaterialExpression.h"

#include "Materials/MaterialExpressionCustom.h"

#include "MaterialCompiler.h"

// Used by the LOCTEXT() macro. See: https://docs.unrealengine.com/5.0/en-US/text-localization-in-unreal-engine/
#define LOCTEXT_NAMESPACE "MaterialExpression"

UMaterialExpressionVolumeDensityOutput::UMaterialExpressionVolumeDensityOutput(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		FText NAME_Output;
		// This is used for placing the expression in the correct category
		// You can reference multiple categories here, see: https://github.com/EpicGames/UnrealEngine/blob/ue5-main/Engine/Source/Runtime/Engine/Private/Materials/MaterialExpressions.cpp#L18670
		FConstructorStatics()
			: NAME_Output(LOCTEXT( "Output", "Output" )) // These can be anything. Like: NAME_Math(LOCTEXT( "Math", "Math" ))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

#if WITH_EDITORONLY_DATA
	MenuCategories.Add(ConstructorStatics.NAME_Output);
#endif

#if WITH_EDITORONLY_DATA
	Outputs.Reset(); // Remove the default output pin
#endif
}

#if WITH_EDITOR
int32 UMaterialExpressionVolumeDensityOutput::Compile( FMaterialCompiler* Compiler, int32 OutputIndex)
{
	int32 CodeInput = INDEX_NONE;

	// Generates function names GetVolumetricAdvanceMaterialOutput{index} used in BasePixelShader.usf.
	if (OutputIndex == 0)
	{
		// If MyInput1 is connected, use it, otherwise return 0
		CodeInput = Density.IsConnected() ? Density.Compile(Compiler) : Compiler->Constant(0);
	}

	return Compiler->CustomOutput(this, OutputIndex, CodeInput);
}

void UMaterialExpressionVolumeDensityOutput::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add(TEXT("WorldGoblin Density Output Node"));
}
#endif // WITH_EDITOR

int32 UMaterialExpressionVolumeDensityOutput::GetNumOutputs() const
{
	return 2;
}

FString UMaterialExpressionVolumeDensityOutput::GetFunctionName() const
{
	// This is the function name used when getting the output values of this node in a shader (vertex factory, compute shader, etc.)
	// Example usage: GetVolumetricAdvanceMaterialOutput0(MaterialParameters) // Grabs the first output value
	return TEXT("WorldGoblinDensityOutput");
}

FString UMaterialExpressionVolumeDensityOutput::GetDisplayName() const
{
	return TEXT("WorldGoblin Density Output Node");
}

#undef LOCTEXT_NAMESPACE