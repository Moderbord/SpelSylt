// Copyright Epic Games, Inc. All Rights Reserved.
// Adapted from the VirtualHeightfieldMesh plugin

#pragma once

#include "CoreMinimal.h"
#include "PrimitiveSceneProxy.h"

namespace TestTriangleShaderMesh
{
	/** Buffers filled by GPU culling. */
	struct FDrawInstanceBuffers
	{
		/* Culled instance buffer. */
		FBufferRHIRef InstanceBuffer;
		FUnorderedAccessViewRHIRef InstanceBufferUAV;
		FShaderResourceViewRHIRef InstanceBufferSRV;

		/* IndirectArgs buffer for final DrawInstancedIndirect. */
		FBufferRHIRef IndirectArgsBuffer;
		FUnorderedAccessViewRHIRef IndirectArgsBufferUAV;
	};
}

class FTestTriangleShaderSceneProxy final : public FPrimitiveSceneProxy
{
public:
	FTestTriangleShaderSceneProxy(class UTestTriangleShaderComponent* InComponent);

protected:
	//~ Begin FPrimitiveSceneProxy Interface
	virtual SIZE_T GetTypeHash() const override;
	virtual uint32 GetMemoryFootprint() const override;
	virtual void CreateRenderThreadResources() override;
	virtual void DestroyRenderThreadResources() override;
	virtual void OnTransformChanged() override;
	// virtual bool HasSubprimitiveOcclusionQueries() const override;
	// virtual const TArray<FBoxSphereBounds>* GetOcclusionQueries(const FSceneView* View) const override;
	// virtual void AcceptOcclusionResults(const FSceneView* View, TArray<bool>* Results, int32 ResultsStart, int32 NumResults) override;
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;
	//~ End FPrimitiveSceneProxy Interface

private:
	void BuildOcclusionVolumes(TArrayView<FVector2D> const& InMinMaxData, FIntPoint const& InMinMaxSize, TArrayView<int32> const& InMinMaxMips, int32 InNumLods);

public:
	bool bHiddenInEditor;

	class FMaterialRenderProxy* Material;
	FMaterialRelevance MaterialRelevance;

	bool bCallbackRegistered;

	class FTestTriangleShaderVertexFactory* VertexFactory;
};


//  Notes: Looks like GetMeshShaderMap is returning nullptr during the DepthPass