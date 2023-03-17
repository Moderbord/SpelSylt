#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Renderer/Private/ScenePrivate.h"

#include "SimpleMaterialTest.generated.h"

struct CAULDRONGENWORLD_API FSimpleMaterialTestDispatchParams
{
	int X;
	int Y;
	int Z;

	
	FVector3f Position;
	
	// Set to the desired material uses when executing our compute shader
	FMaterialRenderProxy* MaterialRenderProxy;
	// Scene reference used to create a uniform buffer for rendering the material graph
	FScene* Scene;
	float GameTime;
	uint32 Random;

	FSimpleMaterialTestDispatchParams(int x, int y, int z)
		: X(x)
		, Y(y)
		, Z(z)
	{
	}
};

// This is a public interface that we define so outside code can invoke our compute shader.
class CAULDRONGENWORLD_API FSimpleMaterialTestInterface {
public:
	// Executes this shader on the render thread
	static void DispatchRenderThread(
		FRHICommandListImmediate& RHICmdList,
		FSimpleMaterialTestDispatchParams Params,
		TFunction<void(FVector3f OutputColor)> AsyncCallback
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void DispatchGameThread(
		FSimpleMaterialTestDispatchParams Params,
		TFunction<void(FVector3f OutputVal)> AsyncCallback
	)
	{
		ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
		[Params, AsyncCallback](FRHICommandListImmediate& RHICmdList)
		{
			DispatchRenderThread(RHICmdList, Params, AsyncCallback);
		});
	}

	// Dispatches this shader. Can be called from any thread
	static void Dispatch(
		FSimpleMaterialTestDispatchParams Params,
		TFunction<void(FVector3f OutputColor)> AsyncCallback
	)
	{
		if (IsInRenderingThread()) {
			DispatchRenderThread(GetImmediateCommandList_ForRenderCommand(), Params, AsyncCallback);
		}else{
			DispatchGameThread(Params, AsyncCallback);
		}
	}
};



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSimpleMaterialTestLibrary_AsyncExecutionCompleted, const FVector3f, Value);


UCLASS() // Change the _API to match your project
class CAULDRONGENWORLD_API USimpleMaterialTestLibrary_AsyncExecution : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	
	// Execute the actual load
	virtual void Activate() override {
		// Create a dispatch parameters struct and set the position passed into our material call
		FSimpleMaterialTestDispatchParams Params(1, 1, 1);
		Params.Position = Position;
		Params.MaterialRenderProxy = MaterialRenderProxy;
		Params.Scene = Scene;
		Params.Random = Random;
		Params.GameTime = GameTime;

		// Dispatch the compute shader and wait until it completes
		FSimpleMaterialTestInterface::Dispatch(Params, [this](FVector3f OutputColor) {
			this->Completed.Broadcast(OutputColor);
		});
	}
	
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", Category = "ComputeShader", WorldContext = "WorldContextObject"))
	static USimpleMaterialTestLibrary_AsyncExecution* ExecuteBaseMaterialComputeShader(UObject* WorldContextObject, AActor* SceneContext, UMaterialInterface* Material, FVector3f Position) {
		check(IsValid(SceneContext));
		USimpleMaterialTestLibrary_AsyncExecution* Action = NewObject<USimpleMaterialTestLibrary_AsyncExecution>();
		Action->Position = Position;
		Action->MaterialRenderProxy = Material->GetRenderProxy();
		Action->Scene = SceneContext->GetRootComponent()->GetScene()->GetRenderScene();
		Action->Random = FMath::Rand();
		Action->GameTime = SceneContext->GetWorld()->GetTimeSeconds();
		Action->RegisterWithGameInstance(WorldContextObject);
		return Action;
	}

	UPROPERTY(BlueprintAssignable)
	FOnSimpleMaterialTestLibrary_AsyncExecutionCompleted Completed;

	
	FVector3f Position;
	FMaterialRenderProxy* MaterialRenderProxy;
	float GameTime;
	uint32 Random;
	FScene* Scene;
	
};