#include "SimpleMaterialTest.h"
#include "CauldronGenWorld/Public/SimpleMaterialTest/SimpleMaterialTest.h"
#include "PixelShaderUtils.h"
#include "RenderCore/Public/RenderGraphUtils.h"
#include "MeshPassProcessor.inl"
#include "StaticMeshResources.h"
#include "DynamicMeshBuilder.h"
#include "RenderGraphResources.h"
#include "GlobalShader.h"
#include "UnifiedBuffer.h"
#include "CanvasTypes.h"
#include "MaterialShader.h"

DECLARE_STATS_GROUP(TEXT("SimpleMaterialTest"), STATGROUP_SimpleMaterialTest, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("SimpleMaterialTest Execute"), STAT_SimpleMaterialTest_Execute, STATGROUP_SimpleMaterialTest);

// This class carries our parameter declarations and acts as the bridge between cpp and HLSL.
class CAULDRONGENWORLD_API FSimpleMaterialTest : public FMeshMaterialShader
{
public:
	
	DECLARE_SHADER_TYPE(FSimpleMaterialTest, MeshMaterial);
	SHADER_USE_PARAMETER_STRUCT_WITH_LEGACY_BASE(FSimpleMaterialTest, FMeshMaterialShader)
	
	
	class FSimpleMaterialTest_Perm_TEST : SHADER_PERMUTATION_INT("TEST", 1);
	using FPermutationDomain = TShaderPermutationDomain<
		FSimpleMaterialTest_Perm_TEST
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
		SHADER_PARAMETER(FVector3f, Position)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float4>, OutputColor)
		

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

		/*
		* Here you define constants that can be used statically in the shader code.
		* Example:
		*/
		// OutEnvironment.SetDefine(TEXT("MY_CUSTOM_CONST"), TEXT("1"));

		/*
		* These defines are used in the thread count section of our shader
		*/
		OutEnvironment.SetDefine(TEXT("THREADS_X"), NUM_THREADS_SimpleMaterialTest_X);
		OutEnvironment.SetDefine(TEXT("THREADS_Y"), NUM_THREADS_SimpleMaterialTest_Y);
		OutEnvironment.SetDefine(TEXT("THREADS_Z"), NUM_THREADS_SimpleMaterialTest_Z);

		// This shader must support typed UAV load and we are testing if it is supported at runtime using RHIIsTypedUAVLoadSupported
		//OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);

		// FForwardLightingParameters::ModifyCompilationEnvironment(Parameters.Platform, OutEnvironment);
	}
private:
};

// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_MATERIAL_SHADER_TYPE(, FSimpleMaterialTest, TEXT("/CauldronGenWorldShaders/SimpleMaterialTest/SimpleMaterialTest.usf"), TEXT("SimpleMaterialTest"), SF_Compute);

void FSimpleMaterialTestInterface::DispatchRenderThread(FRHICommandListImmediate& RHICmdList, FSimpleMaterialTestDispatchParams Params, TFunction<void(FVector3f OutputColor)> AsyncCallback) {
	FRDGBuilder GraphBuilder(RHICmdList);

	{
		SCOPE_CYCLE_COUNTER(STAT_SimpleMaterialTest_Execute);
		DECLARE_GPU_STAT(SimpleMaterialTest)
		RDG_EVENT_SCOPE(GraphBuilder, "SimpleMaterialTest");
		RDG_GPU_STAT_SCOPE(GraphBuilder, SimpleMaterialTest);
		
		const FScene* LocalScene = Params.Scene;
		const FMaterialRenderProxy* MaterialRenderProxy = nullptr;
		const FMaterial* MaterialResource = &Params.MaterialRenderProxy->GetMaterialWithFallback(GMaxRHIFeatureLevel, MaterialRenderProxy);
		MaterialRenderProxy = MaterialRenderProxy ? MaterialRenderProxy : Params.MaterialRenderProxy;

		typename FSimpleMaterialTest::FPermutationDomain PermutationVector;

		// Add any static permutation options here
		// PermutationVector.Set<FSimpleMaterialTest::FMyPermutationName>(12345);
		
		TShaderRef<FSimpleMaterialTest> ComputeShader = MaterialResource->GetShader<FSimpleMaterialTest>(&FLocalVertexFactory::StaticType, PermutationVector, false);
		

		bool bIsShaderValid = ComputeShader.IsValid();

		if (bIsShaderValid) {
			FSimpleMaterialTest::FParameters* PassParameters = GraphBuilder.AllocParameters<FSimpleMaterialTest::FParameters>();

			
			FRDGBufferRef OutputBuffer = GraphBuilder.CreateBuffer(
				FRDGBufferDesc::CreateBufferDesc(sizeof(FVector4f), 1),
				TEXT("OutputBuffer"));

			PassParameters->OutputColor = GraphBuilder.CreateUAV(FRDGBufferUAVDesc(OutputBuffer, PF_A32B32G32R32F));
			PassParameters->Position = Params.Position;

			FViewUniformShaderParameters ViewUniformShaderParameters;

			ViewUniformShaderParameters.GameTime = Params.GameTime;
			ViewUniformShaderParameters.Random = Params.Random;
			ViewUniformShaderParameters.SVPositionToTranslatedWorld = FMatrix44f::Identity;

			auto ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(ViewUniformShaderParameters, UniformBuffer_SingleFrame);
			PassParameters->View = ViewUniformBuffer;

			auto GroupCount = FComputeShaderUtils::GetGroupCount(FIntVector(Params.X, Params.Y, Params.Z), FComputeShaderUtils::kGolden2DGroupSize);
			GraphBuilder.AddPass(
				RDG_EVENT_NAME("ExecuteSimpleMaterialTest"),
				PassParameters,
				ERDGPassFlags::AsyncCompute,
				[&PassParameters, ComputeShader, MaterialRenderProxy, MaterialResource, LocalScene, GroupCount](FRHIComputeCommandList& RHICmdList)
			{
				
				FMeshPassProcessorRenderState DrawRenderState;
				
				MaterialRenderProxy->UpdateUniformExpressionCacheIfNeeded(LocalScene->GetFeatureLevel());
				
				/*FViewUniformShaderParameters ViewUniformShaderParameters;

				ViewUniformShaderParameters.GameTime = GameTime;
				ViewUniformShaderParameters.Random = Random;*/

				/*auto ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(ViewUniformShaderParameters, UniformBuffer_SingleFrame);
				DrawRenderState.SetViewUniformBuffer(ViewUniformBuffer);*/


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
				//RHICmdList.SetComputeShader(ComputeShaderRHI);
				SetComputePipelineState(RHICmdList, ComputeShaderRHI);
				ShaderBindings.SetOnCommandList(RHICmdList, ComputeShaderRHI);
				SetShaderParameters(RHICmdList, ComputeShader, ComputeShaderRHI, *PassParameters);
				RHICmdList.DispatchComputeShader(GroupCount.X, GroupCount.Y, GroupCount.Z);
				
			});

			
			FRHIGPUBufferReadback* GPUBufferReadback = new FRHIGPUBufferReadback(TEXT("ExecuteSimpleMaterialTestOutput"));
			AddEnqueueCopyPass(GraphBuilder, GPUBufferReadback, OutputBuffer, 0u);

			auto RunnerFunc = [GPUBufferReadback, AsyncCallback](auto&& RunnerFunc) -> void {
				if (GPUBufferReadback->IsReady()) {
					
					FVector4f* Buffer = (FVector4f*)GPUBufferReadback->Lock(1);
					FVector3f OutVal = FVector3f(Buffer[0].X, Buffer[0].Y, Buffer[0].Z);
					
					GPUBufferReadback->Unlock();

					AsyncTask(ENamedThreads::GameThread, [AsyncCallback, OutVal]() {
						AsyncCallback(OutVal);
					});

					delete GPUBufferReadback;
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
			AsyncCallback(FVector3f(0.f));
		}
	}

	GraphBuilder.Execute();
}