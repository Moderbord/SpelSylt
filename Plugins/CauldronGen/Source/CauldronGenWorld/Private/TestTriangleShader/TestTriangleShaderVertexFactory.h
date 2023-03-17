// Copyright Epic Games, Inc. All Rights Reserved.
// Adapted from the VirtualHeightfieldMesh plugin

#pragma once

#include "CoreMinimal.h"
#include "Containers/DynamicRHIResourceArray.h"
#include "RenderResource.h"
#include "RHI.h"
#include "SceneManagement.h"
#include "UniformBuffer.h"
#include "VertexFactory.h"

/**
 * Uniform buffer to hold parameters specific to this vertex factory. Only set up once.
 */
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FTestTriangleShaderParameters, )
	// SHADER_PARAMETER_TEXTURE(Texture2D<uint4>, PageTableTexture)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

typedef TUniformBufferRef<FTestTriangleShaderParameters> FTestTriangleShaderBufferRef;

/**
 * Per frame UserData to pass to the vertex shader.
 */
struct FTestTriangleShaderUserData : public FOneFrameResource
{
	FRHIShaderResourceView* InstanceBufferSRV;
	FVector3f LodViewOrigin;
};

/*
* Index buffer to provide incides for the mesh we're rending.
*/
class FTestTriangleShaderIndexBuffer : public FIndexBuffer
{
public:

	FTestTriangleShaderIndexBuffer()
	{}

	virtual void InitRHI() override;

	int32 GetIndexCount() const { return NumIndices; }

private:
	int32 NumIndices = 0;
};

class FTestTriangleShaderVertexFactory : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FTestTriangleShader);

public:
	FTestTriangleShaderVertexFactory(ERHIFeatureLevel::Type InFeatureLevel, const FTestTriangleShaderParameters& InParams);

	~FTestTriangleShaderVertexFactory();

	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;

	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters);
	static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);
	static void ValidateCompiledResult(const FVertexFactoryType* Type, EShaderPlatform Platform, const FShaderParameterMap& ParameterMap, TArray<FString>& OutErrors);

	FIndexBuffer const* GetIndexBuffer() const { return IndexBuffer; }

private:
	FTestTriangleShaderParameters Params;
	FTestTriangleShaderBufferRef UniformBuffer;
	FTestTriangleShaderIndexBuffer* IndexBuffer = nullptr;

	// Shader parameters is the data passed to our vertex shader
	friend class FTestTriangleShaderShaderParameters;
};
