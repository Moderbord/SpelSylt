#include "DualContouring.h"
#include "CauldronGenWorld/Public/DualContouring/DualContouring.h"
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

DECLARE_STATS_GROUP(TEXT("DualContouring"), STATGROUP_DualContouring, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("DualContouring Execute"), STAT_DualContouring_Execute, STATGROUP_DualContouring);

// This class carries our parameter declarations and acts as the bridge between cpp and HLSL.
class CAULDRONGENWORLD_API FDualContouring : public FGlobalShader
{
public:
	
	DECLARE_GLOBAL_SHADER(FDualContouring);
	SHADER_USE_PARAMETER_STRUCT(FDualContouring, FGlobalShader);
	
	
	class FDualContouring_Perm_TEST : SHADER_PERMUTATION_INT("TEST", 1);
	using FPermutationDomain = TShaderPermutationDomain<
		FDualContouring_Perm_TEST
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

		SHADER_PARAMETER(int, Smegma)
		SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<int>, Input)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<int>, Vertices)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<int>, Triangles)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<int>, Output)
		

	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		const FPermutationDomain PermutationVector(Parameters.PermutationId);
		
		return true;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		const FPermutationDomain PermutationVector(Parameters.PermutationId);

		/*
		* Here you define constants that can be used statically in the shader code.
		* Example:
		*/
		// OutEnvironment.SetDefine(TEXT("MY_CUSTOM_CONST"), TEXT("1"));

		/*
		* These defines are used in the thread count section of our shader
		*/
		OutEnvironment.SetDefine(TEXT("THREADS_X"), NUM_THREADS_DualContouring_X);
		OutEnvironment.SetDefine(TEXT("THREADS_Y"), NUM_THREADS_DualContouring_Y);
		OutEnvironment.SetDefine(TEXT("THREADS_Z"), NUM_THREADS_DualContouring_Z);

		// This shader must support typed UAV load and we are testing if it is supported at runtime using RHIIsTypedUAVLoadSupported
		//OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);

		// FForwardLightingParameters::ModifyCompilationEnvironment(Parameters.Platform, OutEnvironment);
	}
private:
};

// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FDualContouring, "/CauldronGenWorldShaders/DualContouring/DualContouring.usf", "DualContouring", SF_Compute);

void FDualContouringInterface::DispatchRenderThread(FRHICommandListImmediate& RHICmdList, FDualContouringDispatchParams Params, TFunction<void(bool SuccessVal)> AsyncCallback) {
	FRDGBuilder GraphBuilder(RHICmdList);

	{
		SCOPE_CYCLE_COUNTER(STAT_DualContouring_Execute);
		DECLARE_GPU_STAT(DualContouring)
		RDG_EVENT_SCOPE(GraphBuilder, "DualContouring");
		RDG_GPU_STAT_SCOPE(GraphBuilder, DualContouring);
		
		typename FDualContouring::FPermutationDomain PermutationVector;
		
		// Add any static permutation options here
		// PermutationVector.Set<FDualContouring::FMyPermutationName>(12345);

		TShaderMapRef<FDualContouring> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel), PermutationVector);
		

		bool bIsShaderValid = ComputeShader.IsValid();

		if (bIsShaderValid) {
			FDualContouring::FParameters* PassParameters = GraphBuilder.AllocParameters<FDualContouring::FParameters>();

			
			const void* RawData = (void*)Params.Input;
			int NumInputs = 2;
			int InputSize = sizeof(int);
			FRDGBufferRef InputBuffer = CreateUploadBuffer(GraphBuilder, TEXT("InputBuffer"), InputSize, NumInputs, RawData, InputSize * NumInputs);

			PassParameters->Smegma = Params.Smegma;
			PassParameters->Input = GraphBuilder.CreateSRV(FRDGBufferSRVDesc(InputBuffer, PF_R32_SINT));

			FRDGBufferRef OutputBuffer = GraphBuilder.CreateBuffer(
				FRDGBufferDesc::CreateBufferDesc(sizeof(int32), 8 * 8),
				TEXT("OutputBuffer"));

			PassParameters->Output = GraphBuilder.CreateUAV(FRDGBufferUAVDesc(OutputBuffer, PF_R32_SINT));

			FRDGBufferRef VerticesBuffer = GraphBuilder.CreateBuffer(
				FRDGBufferDesc::CreateBufferDesc(sizeof(int32), 8 * 8),
				TEXT("VerticesBuffer"));

			PassParameters->Vertices = GraphBuilder.CreateUAV(FRDGBufferUAVDesc(VerticesBuffer, PF_R32_SINT));

			FRDGBufferRef TrianglesBuffer = GraphBuilder.CreateBuffer(
				FRDGBufferDesc::CreateBufferDesc(sizeof(int32), 8 * 8),
				TEXT("TrianglesBuffer"));

			PassParameters->Triangles = GraphBuilder.CreateUAV(FRDGBufferUAVDesc(TrianglesBuffer, PF_R32_SINT));

			auto GroupCount = FComputeShaderUtils::GetGroupCount(FIntVector(Params.X, Params.Y, Params.Z), FComputeShaderUtils::kGolden2DGroupSize);
			GraphBuilder.AddPass(
				RDG_EVENT_NAME("ExecuteDualContouring"),
				PassParameters,
				ERDGPassFlags::AsyncCompute,
				[&PassParameters, ComputeShader, GroupCount](FRHIComputeCommandList& RHICmdList)
			{
				FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *PassParameters, GroupCount);
			});

			
			FRHIGPUBufferReadback* GPUBufferReadback = new FRHIGPUBufferReadback(TEXT("ExecuteDualContouringOutput"));
			AddEnqueueCopyPass(GraphBuilder, GPUBufferReadback, OutputBuffer, 0u);

			auto RunnerFunc = [GPUBufferReadback, AsyncCallback](auto&& RunnerFunc) -> void {
				if (GPUBufferReadback->IsReady()) {
					
					int32* Buffer = (int32*)GPUBufferReadback->Lock(8 * 8);
					bool OutVal = Buffer[0] != 0;
					
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
			
		}
	}

	GraphBuilder.Execute();
}