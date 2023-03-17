// Copyright Epic Games, Inc. All Rights Reserved.
// Adapted from the VirtualHeightfieldMesh plugin

#include "CauldronGenWorld/Public/TestTriangleShader/TestTriangleShaderComponent.h"

#include "Engine/World.h"
#include "TestTriangleShaderSceneProxy.h"

UTestTriangleShaderComponent::UTestTriangleShaderComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CastShadow = true;
	bCastContactShadow = false;
	bUseAsOccluder = true;
	bAffectDynamicIndirectLighting = false;
	bAffectDistanceFieldLighting = false;
	bNeverDistanceCull = true;
#if WITH_EDITORONLY_DATA
	bEnableAutoLODGeneration = false;
#endif
	Mobility = EComponentMobility::Static;
}

void UTestTriangleShaderComponent::OnRegister()
{
	Super::OnRegister();
}

void UTestTriangleShaderComponent::OnUnregister()
{
	Super::OnUnregister();
}

void UTestTriangleShaderComponent::ApplyWorldOffset(const FVector& InOffset, bool bWorldShift)
{
	Super::ApplyWorldOffset(InOffset, bWorldShift);
	MarkRenderStateDirty();
}

bool UTestTriangleShaderComponent::IsVisible() const
{
	return Super::IsVisible();
}

FBoxSphereBounds UTestTriangleShaderComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	return FBoxSphereBounds(FBox(FVector(0.f, 0.f, 0.f), FVector(10000.f))).TransformBy(LocalToWorld);
}

FPrimitiveSceneProxy* UTestTriangleShaderComponent::CreateSceneProxy()
{
	return new FTestTriangleShaderSceneProxy(this);
}

void UTestTriangleShaderComponent::SetMaterial(int32 InElementIndex, UMaterialInterface* InMaterial)
{
	if (InElementIndex == 0 && Material != InMaterial)
	{
		Material = InMaterial;
		MarkRenderStateDirty();
	}
}

void UTestTriangleShaderComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	if (Material != nullptr)
	{
		OutMaterials.Add(Material);
	}
}