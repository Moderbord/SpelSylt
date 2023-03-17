#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/TextureRenderTarget2D.h"

#include "DualContouring.generated.h"

struct CAULDRONGENWORLD_API FDualContouringDispatchParams
{
	int X;
	int Y;
	int Z;


	int Smegma;
	FVector3f Vertices[8 * 8];
	int Triangles[8 * 8];
	int Input[2];
	int Output[8 * 8];
	
	

	FDualContouringDispatchParams(int x, int y, int z)
		: X(x)
		, Y(y)
		, Z(z)
	{
	}
};

// This is a public interface that we define so outside code can invoke our compute shader.
class CAULDRONGENWORLD_API FDualContouringInterface {
public:
	// Executes this shader on the render thread
	static void DispatchRenderThread(
		FRHICommandListImmediate& RHICmdList,
		FDualContouringDispatchParams Params,
		TFunction<void(bool SuccessVal)> AsyncCallback
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void DispatchGameThread(
		FDualContouringDispatchParams Params,
		TFunction<void(bool SuccessVal)> AsyncCallback
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
		FDualContouringDispatchParams Params,
		TFunction<void(bool SuccessVal)> AsyncCallback
	)
	{
		if (IsInRenderingThread()) 
		{
			DispatchRenderThread(GetImmediateCommandList_ForRenderCommand(), Params, AsyncCallback);
		} 
		else 
		{
			DispatchGameThread(Params, AsyncCallback);
		}
	}
};



DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDualContouringLibrary_AsyncExecutionCompleted, const bool, Success, const TArray<FVector3f>&, Vertices, const TArray<int>&, Triangles);


UCLASS() // Change the _API to match your project
class CAULDRONGENWORLD_API UDualContouringLibrary_AsyncExecution : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	
	// Execute the actual load
	virtual void Activate() override {
		// Create a dispatch parameters struct and fill it the input array with our args
		FDualContouringDispatchParams Params(1, 1, 1);
		Params.Smegma = Smegma;

		// Dispatch the compute shader and wait until it completes
		FDualContouringInterface::Dispatch(Params, [this](bool SuccessVal) {
			this->Completed.Broadcast(SuccessVal, Vertices, Triangles);
		});
	}
	
	
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", Category = "ComputeShader", WorldContext = "WorldContextObject"))
	static UDualContouringLibrary_AsyncExecution* ExecuteBaseComputeShader(UObject* WorldContextObject, int Smegma) {
		UDualContouringLibrary_AsyncExecution* Action = NewObject<UDualContouringLibrary_AsyncExecution>();
		Action->Smegma = Smegma;
		Action->RegisterWithGameInstance(WorldContextObject);

		return Action;
	}

	UPROPERTY(BlueprintAssignable)
	FOnDualContouringLibrary_AsyncExecutionCompleted Completed;

	int Smegma;

	UPROPERTY(BlueprintReadOnly)
	TArray<FVector3f> Vertices;

	UPROPERTY(BlueprintReadOnly)
	TArray<int> Triangles;
	
};