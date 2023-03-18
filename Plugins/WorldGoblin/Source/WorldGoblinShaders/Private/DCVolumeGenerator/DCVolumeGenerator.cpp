#include "DCVolumeGenerator.h"
#include "WorldGoblinShaders/Public/DCVolumeGenerator/DCVolumeGenerator.h"
#include "PixelShaderUtils.h"
#include "RenderCore/Public/RenderGraphUtils.h"
#include "RenderingThread.h" //new
#include "MeshPassProcessor.inl"
#include "StaticMeshResources.h"
#include "DynamicMeshBuilder.h"
#include "RenderGraphResources.h"
#include "GlobalShader.h"
#include "UnifiedBuffer.h"
#include "CanvasTypes.h"
#include "MaterialShader.h"

DECLARE_STATS_GROUP(TEXT("DCVolumeGenerator"), STATGROUP_DCVolumeGenerator, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("DCVolumeGenerator Execute"), STAT_DCVolumeGenerator_Execute, STATGROUP_DCVolumeGenerator);

// This class carries our parameter declarations and acts as the bridge between cpp and HLSL.
class WORLDGOBLINSHADERS_API FDCVolumeGenerator : public FMeshMaterialShader
{
public:
	
	DECLARE_SHADER_TYPE(FDCVolumeGenerator, MeshMaterial);
	SHADER_USE_PARAMETER_STRUCT_WITH_LEGACY_BASE(FDCVolumeGenerator, FMeshMaterialShader)
	
	
	class FDCVolumeGenerator_Perm_TEST : SHADER_PERMUTATION_INT("TEST", 1);
	using FPermutationDomain = TShaderPermutationDomain<
		FDCVolumeGenerator_Perm_TEST
	>;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		/*
		* Here's where you define one or more of the input parameters for your shader.
		* Some examples:
		*/
		// SHADER_PARAMETER(uint32, MyUint32) // On the shader side: uint32 MyUint32;
		// SHADER_PARAMETER(FVector3f, MyVector) // On the shader side: float3 MyVector;

		// SHADER_PARAMETER_TEXTURE(Texture2D, MyTexture) // On the shader side: Texture2D<float4> MyTexture; (float4 should be whatever you expect each pixel in the texture to be, in this case float4(R,G,B,A) for 4 channels)
		// SHADER_PARAMETER_SAMPLER(SamplerState, MyTextureSampler) // On the shader side: SamplerState MySampler; // CPP side: TStaticSamplerState<ESamplerFilter::SF_Bilinear>::GetRHI();

		// SHADER_PARAMETER_ARRAY(float, MyFloatArray, [3]) // On the shader side: float MyFloatArray[3];

		// SHADER_PARAMETER_UAV(RWTexture2D<FVector4f>, MyTextureUAV) // On the shader side: RWTexture2D<float4> MyTextureUAV;
		// SHADER_PARAMETER_UAV(RWStructuredBuffer<FMyCustomStruct>, MyCustomStructs) // On the shader side: RWStructuredBuffer<FMyCustomStruct> MyCustomStructs;
		// SHADER_PARAMETER_UAV(RWBuffer<FMyCustomStruct>, MyCustomStructs) // On the shader side: RWBuffer<FMyCustomStruct> MyCustomStructs;

		// SHADER_PARAMETER_SRV(StructuredBuffer<FMyCustomStruct>, MyCustomStructs) // On the shader side: StructuredBuffer<FMyCustomStruct> MyCustomStructs;
		// SHADER_PARAMETER_SRV(Buffer<FMyCustomStruct>, MyCustomStructs) // On the shader side: Buffer<FMyCustomStruct> MyCustomStructs;
		// SHADER_PARAMETER_SRV(Texture2D<FVector4f>, MyReadOnlyTexture) // On the shader side: Texture2D<float4> MyReadOnlyTexture;

		// SHADER_PARAMETER_STRUCT_REF(FMyCustomStruct, MyCustomStruct)

		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)

		SHADER_PARAMETER(uint32, VolumeSize)
		SHADER_PARAMETER(float, VolumeScale)
		SHADER_PARAMETER(FVector3f, Position)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float>, Density)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<FVector4f>, Normals)
		

	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters)
	{
		const FPermutationDomain PermutationVector(Parameters.PermutationId);
		
		const bool bIsCompatible =
			Parameters.MaterialParameters.MaterialDomain == MD_Surface
			&& Parameters.MaterialParameters.BlendMode == BLEND_Opaque
			&& Parameters.MaterialParameters.ShadingModels == MSM_DefaultLit
			&& Parameters.MaterialParameters.bIsUsedWithVirtualHeightfieldMesh;

		return bIsCompatible;
		return true;
	}

	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		const FPermutationDomain PermutationVector(Parameters.PermutationId);

		OutEnvironment.SetDefine(TEXT("THREADS_X"), NUM_THREADS_DCVolumeGenerator_X);
		OutEnvironment.SetDefine(TEXT("THREADS_Y"), NUM_THREADS_DCVolumeGenerator_Y);
		OutEnvironment.SetDefine(TEXT("THREADS_Z"), NUM_THREADS_DCVolumeGenerator_Z);

	}
private:
};

// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_MATERIAL_SHADER_TYPE(, FDCVolumeGenerator, TEXT("/WorldGoblinShadersShaders/DCVolumeGenerator/DCVolumeGenerator.usf"), TEXT("DCVolumeGenerator"), SF_Compute);




class WORLDGOBLINSHADERS_API FDCVertexGenerator : public FGlobalShader
{
public:

	DECLARE_GLOBAL_SHADER(FDCVertexGenerator);
	SHADER_USE_PARAMETER_STRUCT(FDCVertexGenerator, FGlobalShader);

		class FDCVertexGenerator_Perm_TEST : SHADER_PERMUTATION_INT("TEST", 1);
	using FPermutationDomain = TShaderPermutationDomain<
		FDCVertexGenerator_Perm_TEST
	>;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(uint32, VolumeSize)
		SHADER_PARAMETER(float, VolumeScale)
		SHADER_PARAMETER(float, MaxCornerDistance)
		SHADER_PARAMETER(float, CenterBias)
		SHADER_PARAMETER(float, ClampRange)
		SHADER_PARAMETER(FVector3f, Position)
		SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<float>, Density)
		SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<FVector4f>, Normals)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<int>, Indices)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float3>, Vertices)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float3>, OutNormals)
	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		const FPermutationDomain PermutationVector(Parameters.PermutationId);

		return true;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		const FPermutationDomain PermutationVector(Parameters.PermutationId);

		OutEnvironment.SetDefine(TEXT("THREADS_X"), NUM_THREADS_DCVertexGenerator_X);
		OutEnvironment.SetDefine(TEXT("THREADS_Y"), NUM_THREADS_DCVertexGenerator_Y);
		OutEnvironment.SetDefine(TEXT("THREADS_Z"), NUM_THREADS_DCVertexGenerator_Z);
	}
private:
};

// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_MATERIAL_SHADER_TYPE(, FDCVertexGenerator, TEXT("/WorldGoblinShadersShaders/DCVolumeGenerator/DCVertexGenerator.usf"), TEXT("DCVertexGenerator"), SF_Compute);




class WORLDGOBLINSHADERS_API FDCTriangleGenerator : public FGlobalShader
{
public:

	DECLARE_GLOBAL_SHADER(FDCTriangleGenerator);
	SHADER_USE_PARAMETER_STRUCT(FDCTriangleGenerator, FGlobalShader);

	class FDCTriangleGenerator_Perm_TEST : SHADER_PERMUTATION_INT("TEST", 1);
	using FPermutationDomain = TShaderPermutationDomain<
		FDCTriangleGenerator_Perm_TEST
	>;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(uint32, VolumeSize)
		SHADER_PARAMETER(FVector3f, Position)
		SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<float>, Density)
	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		const FPermutationDomain PermutationVector(Parameters.PermutationId);

		return true;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		const FPermutationDomain PermutationVector(Parameters.PermutationId);

		OutEnvironment.SetDefine(TEXT("THREADS_X"), NUM_THREADS_DCTriangleGenerator_X);
		OutEnvironment.SetDefine(TEXT("THREADS_Y"), NUM_THREADS_DCTriangleGenerator_Y);
		OutEnvironment.SetDefine(TEXT("THREADS_Z"), NUM_THREADS_DCTriangleGenerator_Z);
	}
private:
};

// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_MATERIAL_SHADER_TYPE(, FDCTriangleGenerator, TEXT("/WorldGoblinShadersShaders/DCVolumeGenerator/DCTriangleGenerator.usf"), TEXT("DCTriangleGenerator"), SF_Compute);




void FDCVolumeGeneratorInterface::DispatchRenderThread(FDCVolumeGeneratorDispatchParams Params, TFunction<void(bool Success, UDCVolumeResult* Result)> AsyncCallback)
{
	FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();

	FRDGBuilder GraphBuilder(RHICmdList);
	{
		SCOPE_CYCLE_COUNTER(STAT_DCVolumeGenerator_Execute);
		DECLARE_GPU_STAT(DCVolumeGenerator)
		RDG_EVENT_SCOPE(GraphBuilder, "DCVolumeGenerator");
		RDG_GPU_STAT_SCOPE(GraphBuilder, DCVolumeGenerator);
		
		const int32 VolumeSize = Params.VolumeSize;
		const int32 NumVoxels = VolumeSize * VolumeSize * VolumeSize;
		const FScene* LocalScene = Params.Scene;
		const FMaterialRenderProxy* MaterialRenderProxy = nullptr;
		const FMaterial* MaterialResource = &Params.MaterialRenderProxy->GetMaterialWithFallback(GMaxRHIFeatureLevel, MaterialRenderProxy);
		MaterialRenderProxy = MaterialRenderProxy ? MaterialRenderProxy : Params.MaterialRenderProxy;

		typename FDCVolumeGenerator::FPermutationDomain VolumePermutationVector;
		typename FDCVertexGenerator::FPermutationDomain VertexPermutationVector;
		//typename FDCTriangleGenerator::FPermutationDomain TrianglePermutationVector;
		
		TShaderRef<FDCVolumeGenerator> ComputeShader = MaterialResource->GetShader<FDCVolumeGenerator>(&FLocalVertexFactory::StaticType, VolumePermutationVector, false);
		TShaderMapRef<FDCVertexGenerator> VertexGenerator(GetGlobalShaderMap(GMaxRHIFeatureLevel), VertexPermutationVector);
		//TShaderMapRef<FDCTriangleGenerator> TriangleGenerator(GetGlobalShaderMap(GMaxRHIFeatureLevel), TrianglePermutationVector);

		bool isGeneratorValid = ComputeShader.IsValid();
		bool isVertexGenValid = VertexGenerator.IsValid();
		bool isTriangleGenValid = true; // TriangleGenerator.IsValid();

		if (isGeneratorValid && isVertexGenValid && isTriangleGenValid) 
		{

			FDCVolumeGenerator::FParameters* VolumePassParameters = GraphBuilder.AllocParameters<FDCVolumeGenerator::FParameters>();
			FDCVertexGenerator::FParameters* VertexPassParameters = GraphBuilder.AllocParameters<FDCVertexGenerator::FParameters>();
			//FDCTriangleGenerator::FParameters* TrianglePassParameters = GraphBuilder.AllocParameters<FDCTriangleGenerator::FParameters>();

			// Create a buffer for normals
			FRDGBufferRef NormalsBuffer = GraphBuilder.CreateBuffer(
				FRDGBufferDesc::CreateBufferDesc(sizeof(FVector4f), NumVoxels),
				TEXT("NormalsBuffer"));

			auto NormalsBufferUAV = GraphBuilder.CreateUAV(FRDGBufferUAVDesc(NormalsBuffer, PF_A32B32G32R32F));
			auto NormalsBufferSRV = GraphBuilder.CreateSRV(FRDGBufferSRVDesc(NormalsBuffer, PF_A32B32G32R32F));

			// Create a buffer for density values
			FRDGBufferRef DensityBuffer = GraphBuilder.CreateBuffer(
				FRDGBufferDesc::CreateBufferDesc(sizeof(float), NumVoxels),
				TEXT("DensityBuffer"));

			auto DensityBufferUAV = GraphBuilder.CreateUAV(FRDGBufferUAVDesc(DensityBuffer, PF_R32_SINT));
			auto DensityBufferSRV = GraphBuilder.CreateSRV(FRDGBufferSRVDesc(DensityBuffer, PF_R32_SINT));

			// Create a buffer for vertices
			FRDGBufferRef VertexBuffer = GraphBuilder.CreateBuffer(
				FRDGBufferDesc::CreateBufferDesc(sizeof(FVector3f), NumVoxels),
				TEXT("VertexBuffer"));

			auto VertexBufferUAV = GraphBuilder.CreateUAV(FRDGBufferUAVDesc(VertexBuffer, PF_R32G32B32_SINT));

			// Create a buffer for indices
			FRDGBufferRef IndexBuffer = GraphBuilder.CreateBuffer(
				FRDGBufferDesc::CreateBufferDesc(sizeof(int), NumVoxels),
				TEXT("IndexBuffer"));

			auto IndexBufferUAV = GraphBuilder.CreateUAV(FRDGBufferUAVDesc(IndexBuffer, PF_R32_SINT));

			// Create a buffer for output normals
			FRDGBufferRef OutNormalsBuffer = GraphBuilder.CreateBuffer(
				FRDGBufferDesc::CreateBufferDesc(sizeof(FVector4f), NumVoxels),
				TEXT("NormalsBuffer"));

			auto OutNormalsBufferUAV = GraphBuilder.CreateUAV(FRDGBufferUAVDesc(OutNormalsBuffer, PF_A32B32G32R32F));
			
			// Create a view uniform buffer for the generator material and fill it with whatever is necessary
			FViewUniformShaderParameters ViewUniformShaderParameters;

			ViewUniformShaderParameters.GameTime = Params.GameTime;
			ViewUniformShaderParameters.Random = Params.Random;
			ViewUniformShaderParameters.SVPositionToTranslatedWorld = FMatrix44f::Identity;

			auto ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(ViewUniformShaderParameters, UniformBuffer_SingleFrame);

			
			VolumePassParameters->View = ViewUniformBuffer;
			VolumePassParameters->VolumeSize = Params.VolumeSize;
			VolumePassParameters->VolumeScale = Params.VolumeScale;
			VolumePassParameters->Position = Params.Position;
			VolumePassParameters->Normals = NormalsBufferUAV;
			VolumePassParameters->Density = DensityBufferUAV;

			VertexPassParameters->VolumeSize = Params.VolumeSize;
			VertexPassParameters->VolumeScale = Params.VolumeScale;
			VertexPassParameters->MaxCornerDistance = Params.MaxCornerDistance;
			VertexPassParameters->CenterBias = Params.CenterBias;
			VertexPassParameters->ClampRange = Params.ClampRange;
			VertexPassParameters->Position = Params.Position;
			VertexPassParameters->Normals = NormalsBufferSRV;
			VertexPassParameters->Density = DensityBufferSRV;
			VertexPassParameters->Vertices = VertexBufferUAV;
			VertexPassParameters->Indices = IndexBufferUAV;
			VertexPassParameters->OutNormals = OutNormalsBufferUAV;

			/*TrianglePassParameters->VolumeSize = Params.VolumeSize;
			TrianglePassParameters->Density = DensityBufferUAV;
			TrianglePassParameters->Indices = IndexBufferUAV;
			TrianglePassParameters->Quads = QuadBufferUAV;*/


			auto GroupCount = FIntVector(
				VolumeSize / NUM_THREADS_DCVolumeGenerator_X,
				VolumeSize / NUM_THREADS_DCVolumeGenerator_Y,
				VolumeSize / NUM_THREADS_DCVolumeGenerator_Z);

			/* Volume generation pass */
			GraphBuilder.AddPass(
				RDG_EVENT_NAME("ExecuteDCVolumeGenerator"),
				VolumePassParameters,
				ERDGPassFlags::AsyncCompute,
				[&VolumePassParameters, ComputeShader, MaterialRenderProxy, MaterialResource, LocalScene, GroupCount](FRHIComputeCommandList& RHICmdList)
			{
				
				FMeshPassProcessorRenderState DrawRenderState;

				MaterialRenderProxy->UpdateUniformExpressionCacheIfNeeded(LocalScene->GetFeatureLevel());

				FMeshMaterialShaderElementData ShaderElementData;

				FMeshProcessorShaders PassShaders;
				PassShaders.ComputeShader = ComputeShader;

				FMeshDrawShaderBindings ShaderBindings;
				ShaderBindings.Initialize(PassShaders);

				int32 DataOffset = 0;
				FMeshDrawSingleShaderBindings SingleShaderBindings = ShaderBindings.GetSingleShaderBindings(SF_Compute, DataOffset);
				ComputeShader->GetShaderBindings(LocalScene, LocalScene->GetFeatureLevel(), nullptr, *MaterialRenderProxy, *MaterialResource, DrawRenderState, ShaderElementData, SingleShaderBindings);

				ShaderBindings.Finalize(&PassShaders);

				FRHIComputeShader* ComputeShaderRHI = ComputeShader.GetComputeShader();
				SetComputePipelineState(RHICmdList, ComputeShaderRHI);
				ShaderBindings.SetOnCommandList(RHICmdList, ComputeShaderRHI);
				SetShaderParameters(RHICmdList, ComputeShader, ComputeShaderRHI, *VolumePassParameters);
				RHICmdList.DispatchComputeShader(GroupCount.X, GroupCount.Y, GroupCount.Z);
				
			});

			/*GraphBuilder.AddPass(
				RDG_EVENT_NAME("DC Vertex Generation"),
				VertexPassParameters,
				ERDGPassFlags::AsyncCompute,
				[&VertexPassParameters, VertexGenerator, GroupCount](FRHIComputeCommandList& RHICmdList)
				{
					FComputeShaderUtils::Dispatch(RHICmdList, VertexGenerator, *VertexPassParameters, GroupCount);
				});*/
			
			FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("DC Vertex Generation"), VertexGenerator, VertexPassParameters, GroupCount);



















			FRHIGPUBufferReadback* DensityReadback = new FRHIGPUBufferReadback(TEXT("DCVolumeDensityExtraction"));
			AddEnqueueCopyPass(GraphBuilder, DensityReadback, DensityBuffer, 0u); // Try removing 0 here?

			FRHIGPUBufferReadback* NormalsReadback = new FRHIGPUBufferReadback(TEXT("DCVolumeNormalsExtraction"));
			AddEnqueueCopyPass(GraphBuilder, NormalsReadback, NormalsBuffer, 0u); // Try removing 0 here?

			auto RunnerFunc = [DensityReadback, NormalsReadback, GroupCount, NumVoxels, AsyncCallback](auto&& RunnerFunc) -> void {
				if (DensityReadback->IsReady() && NormalsReadback->IsReady()) {
					

					UDCVolumeResult* Res = NewObject<UDCVolumeResult>();

					uint32 density_data_size = NumVoxels * sizeof(float);
					float* density_buffer = (float*)DensityReadback->Lock(density_data_size);
					Res->Densities.Init(0.f, NumVoxels);
					FMemory::Memcpy(Res->Densities.GetData(), density_buffer, density_data_size);
					DensityReadback->Unlock();

					uint32 normals_data_size = NumVoxels * sizeof(FVector4f);
					FVector4f* normals_buffer = (FVector4f*)NormalsReadback->Lock(normals_data_size);
					Res->Normals.Init(FVector4f(0.f), NumVoxels);
					FMemory::Memcpy(Res->Normals.GetData(), normals_buffer, normals_data_size);
					NormalsReadback->Unlock();

					AsyncTask(ENamedThreads::GameThread, [AsyncCallback, Res]() {
						AsyncCallback(true, Res);
					});

					delete DensityReadback;
					delete NormalsReadback;

				} else {
					AsyncTask(ENamedThreads::ActualRenderingThread, [RunnerFunc]() {
						RunnerFunc(RunnerFunc);
					});
				}
			};

			AsyncTask(ENamedThreads::ActualRenderingThread, [RunnerFunc]() {
				RunnerFunc(RunnerFunc);
			});
			
		} else {
			#if WITH_EDITOR
				GEngine->AddOnScreenDebugMessage((uint64)42145125184, 6.f, FColor::Red, FString(TEXT("The compute shader has a problem.")));
			#endif

			// We exit here as we don't want to crash the game if the shader is not found or has an error.
			AsyncCallback(false, nullptr);
		}
	}

	GraphBuilder.Execute();
}

void FDCVolumeGeneratorInterface::DispatchGameThread(FDCVolumeGeneratorDispatchParams Params, TFunction<void(bool Success, UDCVolumeResult* Result)> AsyncCallback)
{
	ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
		[Params, AsyncCallback](FRHICommandListImmediate& RHICmdList)
		{
			DispatchRenderThread(Params, AsyncCallback);
		});
}

void FDCVolumeGeneratorInterface::Dispatch(FDCVolumeGeneratorDispatchParams Params, TFunction<void(bool Success, UDCVolumeResult* Result)> AsyncCallback)
{
	if (IsInRenderingThread()) 
	{
		DispatchRenderThread(Params, AsyncCallback);
	}
	else
	{
		DispatchGameThread(Params, AsyncCallback);
	}
}

FDCVolumeGeneratorDispatchParams::FDCVolumeGeneratorDispatchParams(AActor* SceneContext, int32 VolumeSize, UMaterialInterface* Generator, FVector3f Position)
{
	this->VolumeSize = VolumeSize;
	this->Position = Position;
	this->MaterialRenderProxy = Generator->GetRenderProxy();
	this->Scene = SceneContext->GetRootComponent()->GetScene()->GetRenderScene();
	this->Random = FMath::Rand();
	this->GameTime = SceneContext->GetWorld()->GetTimeSeconds();
}
