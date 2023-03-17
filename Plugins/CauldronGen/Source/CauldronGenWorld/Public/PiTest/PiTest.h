#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/TextureRenderTarget2D.h"

#include "PiTest.generated.h"

struct CAULDRONGENWORLD_API FPiTestDispatchParams
{
	int X;
	int Y;
	int Z;

	
	float Seed;
	
	

	FPiTestDispatchParams(int x, int y, int z)
		: X(x)
		, Y(y)
		, Z(z)
	{
	}
};

// This is a public interface that we define so outside code can invoke our compute shader.
class CAULDRONGENWORLD_API FPiTestInterface {
public:
	// Executes this shader on the render thread
	static void DispatchRenderThread(
		FRHICommandListImmediate& RHICmdList,
		FPiTestDispatchParams Params,
		TFunction<void(int TotalInCircle)> AsyncCallback
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void DispatchGameThread(
		FPiTestDispatchParams Params,
		TFunction<void(int OutputVal)> AsyncCallback
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
		FPiTestDispatchParams Params,
		TFunction<void(int TotalInCircle)> AsyncCallback
	)
	{
		if (IsInRenderingThread()) {
			DispatchRenderThread(GetImmediateCommandList_ForRenderCommand(), Params, AsyncCallback);
		}else{
			DispatchGameThread(Params, AsyncCallback);
		}
	}
};



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPiTestLibrary_AsyncExecutionCompleted, const double, Value);


UCLASS() // Change the _API to match your project
class CAULDRONGENWORLD_API UPiTestLibrary_AsyncExecution : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	
	// Execute the actual load
	virtual void Activate() override {
		// Create a dispatch parameters struct and set our desired seed
		FPiTestDispatchParams Params(TotalSamples, 1, 1);
		Params.Seed = Seed;

		// Dispatch the compute shader and wait until it completes
		FPiTestInterface::Dispatch(Params, [this](int TotalInCircle) {
			// TotalInCircle is set to the result of the compute shader
			// Divide by the total number of samples to get the ratio of samples in the circle
			// We're multiplying by 4 because the simulation is done in quarter-circles
			double FinalPI = ((double) TotalInCircle / (double) TotalSamples);

			Completed.Broadcast(FinalPI);
		});
	}
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", Category = "ComputeShader", WorldContext = "WorldContextObject"))
	static UPiTestLibrary_AsyncExecution* ExecutePIComputeShader(UObject* WorldContextObject, int TotalSamples, float Seed) {
		UPiTestLibrary_AsyncExecution* Action = NewObject<UPiTestLibrary_AsyncExecution>();
		Action->TotalSamples = TotalSamples;
		Action->Seed = Seed;
		Action->RegisterWithGameInstance(WorldContextObject);

		return Action;
	}
	

	UPROPERTY(BlueprintAssignable)
	FOnPiTestLibrary_AsyncExecutionCompleted Completed;

	
	float Seed;
	int TotalSamples;
	
};