#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"

#include "DCVolumeGenerator.generated.h"

struct WORLDGOBLINSHADERS_API FQuad
{
	FIntVector3 a;
	FIntVector3 b;
};

struct WORLDGOBLINSHADERS_API FDCVolumeGeneratorDispatchParams
{

	uint32 VolumeSize = 32;
	FVector3f Position = FVector3f(0.f);
	
	/* Generation */
	FMaterialRenderProxy* MaterialRenderProxy;
	FScene* Scene;
	float GameTime;
	uint32 Random;

	/* Triangulation */
	float VolumeScale = 100.f;
	float MaxCornerDistance = 100.f;
	float CenterBias = 0.01f;
	float ClampRange = 100.f;

	FDCVolumeGeneratorDispatchParams(AActor* SceneContext, int32 VolumeSize, UMaterialInterface* Generator, FVector3f Position);
};

UCLASS(BlueprintType)
class WORLDGOBLINSHADERS_API UDCVolumeResult : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	TArray<FVector4f> SDF;

	UPROPERTY(BlueprintReadOnly)
	TArray<FVector3f> Vertices;

	UPROPERTY(BlueprintReadOnly)
	TArray<FVector3f> Normals;

	UPROPERTY(BlueprintReadOnly)
	TArray<FIntVector> Triangles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 VertexCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 TriangleCount = 0;
};

// This is a public interface that we define so outside code can invoke our compute shader.
class WORLDGOBLINSHADERS_API FDCVolumeGeneratorInterface {
public:
	// Executes this shader on the render thread
	static void DispatchRenderThread(
		//class FRHICommandListImmediate* RHICmdList,
		FDCVolumeGeneratorDispatchParams Params,
		TFunction<void(bool Success, UDCVolumeResult* Output)> AsyncCallback
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void DispatchGameThread(
		FDCVolumeGeneratorDispatchParams Params,
		TFunction<void(bool Success, UDCVolumeResult* Output)> AsyncCallback
	);

	// Dispatches this shader. Can be called from any thread
	static void Dispatch(
		FDCVolumeGeneratorDispatchParams Params,
		TFunction<void(bool Success, UDCVolumeResult* Output)> AsyncCallback
	);
};