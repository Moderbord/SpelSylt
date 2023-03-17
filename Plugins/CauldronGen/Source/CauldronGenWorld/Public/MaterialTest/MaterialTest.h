#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Renderer/Private/ScenePrivate.h"

#include "MaterialTest.generated.h"

struct CAULDRONGENWORLD_API FMaterialTestDispatchParams
{
	int X;
	int Y;
	int Z;

	
	FRenderTarget* RenderTarget;
	
	// Set to the desired material uses when executing our compute shader
	FMaterialRenderProxy* MaterialRenderProxy;
	// Scene reference used to create a uniform buffer for rendering the material graph
	FScene* Scene;
	float GameTime;
	uint32 Random;

	FMaterialTestDispatchParams(int x, int y, int z)
		: X(x)
		, Y(y)
		, Z(z)
	{
	}
};

// This is a public interface that we define so outside code can invoke our compute shader.
class CAULDRONGENWORLD_API FMaterialTestInterface {
public:
	// Executes this shader on the render thread
	static void DispatchRenderThread(
		FRHICommandListImmediate& RHICmdList,
		FMaterialTestDispatchParams Params
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void DispatchGameThread(
		FMaterialTestDispatchParams Params
	)
	{
		ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
		[Params](FRHICommandListImmediate& RHICmdList)
		{
			DispatchRenderThread(RHICmdList, Params);
		});
	}

	// Dispatches this shader. Can be called from any thread
	static void Dispatch(
		FMaterialTestDispatchParams Params
	)
	{
		if (IsInRenderingThread()) {
			DispatchRenderThread(GetImmediateCommandList_ForRenderCommand(), Params);
		}else{
			DispatchGameThread(Params);
		}
	}
};

// This is a static blueprint library that can be used to invoke our compute shader from blueprints.
UCLASS()
class CAULDRONGENWORLD_API UMaterialTestLibrary : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	static void ExecuteMaterialRTComputeShader(AActor* SceneContext, UMaterialInterface* Material, UTextureRenderTarget2D* RT)
	{
		// Create a dispatch parameters struct and fill it the input array with our args
		FMaterialTestDispatchParams Params(RT->SizeX, RT->SizeY, 1);
		Params.RenderTarget = RT->GameThread_GetRenderTargetResource();
		Params.MaterialRenderProxy = Material->GetRenderProxy();
		Params.Scene = SceneContext->GetRootComponent()->GetScene()->GetRenderScene();
		Params.Random = FMath::Rand();
		Params.GameTime = SceneContext->GetWorld()->GetTimeSeconds();

		FMaterialTestInterface::Dispatch(Params);
	}
};
