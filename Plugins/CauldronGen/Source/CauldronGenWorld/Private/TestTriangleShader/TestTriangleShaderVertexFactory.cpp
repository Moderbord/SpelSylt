// Copyright Epic Games, Inc. All Rights Reserved.
// Adapted from the VirtualHeightfieldMesh plugin

#include "TestTriangleShaderVertexFactory.h"

#include "Engine/Engine.h"
#include "EngineGlobals.h"
#include "Materials/Material.h"
#include "MeshMaterialShader.h"
#include "RHIStaticStates.h"
#include "ShaderParameters.h"
#include "ShaderParameterUtils.h"

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FTestTriangleShaderParameters, "TestTriangleShaderParams");

namespace FTestTriangleShaderUtil
{
	template< typename T >
	FBufferRHIRef CreateIndexBuffer()
	{
		TResourceArray<T, INDEXBUFFER_ALIGNMENT> Indices;

		// Allocate room for indices
		Indices.Reserve(3);
		
		// Top left
		// CCW triangle winding order
		Indices.Add(1);
		Indices.Add(0);
		Indices.Add(2);

		const uint32 Size = Indices.GetResourceDataSize();
		const uint32 Stride = sizeof(T);

		// Create index buffer. Fill buffer with initial data upon creation
		FRHIResourceCreateInfo CreateInfo(TEXT("TestTriangleShaderIndexBuffer"), &Indices);
		return RHICreateIndexBuffer(Stride, Size, BUF_Static, CreateInfo);
	}
}

void FTestTriangleShaderIndexBuffer::InitRHI()
{
	IndexBufferRHI = FTestTriangleShaderUtil::CreateIndexBuffer<uint16>();
}


/**
 * Shader parameters for vertex factory.
 */
class FTestTriangleShaderShaderParameters : public FVertexFactoryShaderParameters
{
	DECLARE_TYPE_LAYOUT(FTestTriangleShaderShaderParameters, NonVirtual);

public:
	void Bind(const FShaderParameterMap& ParameterMap)
	{
		// TODO
		InstanceBufferParameter.Bind(ParameterMap, TEXT("InstanceBuffer"));
		LodViewOriginParameter.Bind(ParameterMap, TEXT("LodViewOrigin"));
		// LodDistancesParameter.Bind(ParameterMap, TEXT("LodDistances"));
	}

	void GetElementShaderBindings(
		const class FSceneInterface* Scene,
		const class FSceneView* View,
		const class FMeshMaterialShader* Shader,
		const EVertexInputStreamType InputStreamType,
		ERHIFeatureLevel::Type FeatureLevel,
		const class FVertexFactory* InVertexFactory,
		const struct FMeshBatchElement& BatchElement,
		class FMeshDrawSingleShaderBindings& ShaderBindings,
		FVertexInputStreamArray& VertexStreams) const
	{
		FTestTriangleShaderVertexFactory* VertexFactory = (FTestTriangleShaderVertexFactory*)InVertexFactory;
		ShaderBindings.Add(Shader->GetUniformBufferParameter<FTestTriangleShaderParameters>(), VertexFactory->UniformBuffer);

		FTestTriangleShaderUserData* UserData = (FTestTriangleShaderUserData*)BatchElement.UserData;
		// TODO
		ShaderBindings.Add(InstanceBufferParameter, UserData->InstanceBufferSRV);
		ShaderBindings.Add(LodViewOriginParameter, UserData->LodViewOrigin);
		// ShaderBindings.Add(LodDistancesParameter, UserData->LodDistances);
	}

protected:
	// TODO
	LAYOUT_FIELD(FShaderResourceParameter, InstanceBufferParameter);
	LAYOUT_FIELD(FShaderParameter, LodViewOriginParameter);
	// LAYOUT_FIELD(FShaderParameter, LodDistancesParameter);
};

IMPLEMENT_TYPE_LAYOUT(FTestTriangleShaderShaderParameters);

IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FTestTriangleShaderVertexFactory, SF_Vertex, FTestTriangleShaderShaderParameters);
IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FTestTriangleShaderVertexFactory, SF_Pixel, FTestTriangleShaderShaderParameters);


FTestTriangleShaderVertexFactory::FTestTriangleShaderVertexFactory(ERHIFeatureLevel::Type InFeatureLevel, const FTestTriangleShaderParameters& InParams)
	: FVertexFactory(InFeatureLevel)
	, Params(InParams)
{
	IndexBuffer = new FTestTriangleShaderIndexBuffer();
}

FTestTriangleShaderVertexFactory::~FTestTriangleShaderVertexFactory()
{
	delete IndexBuffer;
}

void FTestTriangleShaderVertexFactory::InitRHI()
{
	UniformBuffer = FTestTriangleShaderBufferRef::CreateUniformBufferImmediate(Params, UniformBuffer_MultiFrame);

	IndexBuffer->InitResource();

	FVertexStream NullVertexStream;
	NullVertexStream.VertexBuffer = nullptr;
	NullVertexStream.Stride = 0;
	NullVertexStream.Offset = 0;
	NullVertexStream.VertexStreamUsage = EVertexStreamUsage::ManualFetch;

	check(Streams.Num() == 0);
	Streams.Add(NullVertexStream);

	FVertexDeclarationElementList Elements;
	
	InitDeclaration(Elements);
}

void FTestTriangleShaderVertexFactory::ReleaseRHI()
{
	UniformBuffer.SafeRelease();

	if (IndexBuffer)
	{
		IndexBuffer->ReleaseResource();
	}

	FVertexFactory::ReleaseRHI();
}

bool FTestTriangleShaderVertexFactory::ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters)
{
	//todo[vhm]: Fallback path for mobile.
	if (!IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5))
	{
		return false;
	}
	// TODO
	return (Parameters.MaterialParameters.MaterialDomain == MD_Surface && Parameters.MaterialParameters.bIsUsedWithVirtualHeightfieldMesh) || Parameters.MaterialParameters.bIsSpecialEngineMaterial;
}

void FTestTriangleShaderVertexFactory::ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	// TODO
	// OutEnvironment.SetDefine(TEXT("VF_VIRTUAL_HEIGHFIELD_MESH"), 1);
}

void FTestTriangleShaderVertexFactory::ValidateCompiledResult(const FVertexFactoryType* Type, EShaderPlatform Platform, const FShaderParameterMap& ParameterMap, TArray<FString>& OutErrors)
{
}

// TODO
IMPLEMENT_VERTEX_FACTORY_TYPE(FTestTriangleShaderVertexFactory, "/CauldronGenWorldShaders/TestTriangleShader/TestTriangleShaderVertexFactory.ush",
	  EVertexFactoryFlags::UsedWithMaterials
	| EVertexFactoryFlags::SupportsDynamicLighting
	| EVertexFactoryFlags::SupportsPrimitiveIdStream
);
