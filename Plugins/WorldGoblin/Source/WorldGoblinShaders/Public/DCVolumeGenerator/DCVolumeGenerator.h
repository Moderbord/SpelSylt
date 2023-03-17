#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"
//#include "Kismet/BlueprintAsyncActionBase.h"
//#include "Engine/TextureRenderTarget2D.h"
//#include "Renderer/Private/ScenePrivate.h"

//#include "DCVolumeGenerator.generated.h"

struct WORLDGOBLINSHADERS_API FDCVolumeGeneratorDispatchParams
{

	int32 VolumeSize;
	
	FVector3f Position;
	
	// Set to the desired material uses when executing our compute shader
	FMaterialRenderProxy* MaterialRenderProxy;
	// Scene reference used to create a uniform buffer for rendering the material graph
	FScene* Scene;
	float GameTime;
	uint32 Random;

	FDCVolumeGeneratorDispatchParams(AActor* SceneContext, int32 VolumeSize, UMaterialInterface* Generator, FVector3f Position);
};

// This is a public interface that we define so outside code can invoke our compute shader.
class WORLDGOBLINSHADERS_API FDCVolumeGeneratorInterface {
public:
	// Executes this shader on the render thread
	static void DispatchRenderThread(
		//class FRHICommandListImmediate* RHICmdList,
		FDCVolumeGeneratorDispatchParams Params,
		TFunction<void(FVector3f OutputColor)> AsyncCallback
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void DispatchGameThread(
		FDCVolumeGeneratorDispatchParams Params,
		TFunction<void(FVector3f OutputVal)> AsyncCallback
	);

	// Dispatches this shader. Can be called from any thread
	static void Dispatch(
		FDCVolumeGeneratorDispatchParams Params,
		TFunction<void(FVector3f OutputColor)> AsyncCallback
	);
};


//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDCVolumeGenerator_AsyncExecutionCompleted, const FVector3f, Value);
//
//
//UCLASS() // Change the _API to match your project
//class WORLDGOBLINSHADERS_API UDCVolumeGenerator_AsyncExecution : public UBlueprintAsyncActionBase
//{
//	GENERATED_BODY()
//
//public:
//
//	// Execute the actual load
//	virtual void Activate() override {
//		// Create a dispatch parameters struct and set the position passed into our material call
//		FDCVolumeGeneratorDispatchParams Params(1, 1, 1);
//		Params.Position = Position;
//		Params.MaterialRenderProxy = MaterialRenderProxy;
//		Params.Scene = Scene;
//		Params.Random = Random;
//		Params.GameTime = GameTime;
//
//		// Dispatch the compute shader and wait until it completes
//		FDCVolumeGeneratorInterface::Dispatch(Params, [this](FVector3f OutputColor) {
//			this->Completed.Broadcast(OutputColor);
//			});
//	}
//
//
//	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", Category = "ComputeShader", WorldContext = "WorldContextObject"))
//	static UDCVolumeGenerator_AsyncExecution* ExecuteDualContouringShader(UObject* WorldContextObject, AActor* SceneContext, UMaterialInterface* Material, FVector3f Position) 
//	{
//		check(IsValid(SceneContext));
//		UDCVolumeGenerator_AsyncExecution* Action = NewObject<UDCVolumeGenerator_AsyncExecution>();
//		Action->Position = Position;
//		Action->MaterialRenderProxy = Material->GetRenderProxy();
//		Action->Scene = SceneContext->GetRootComponent()->GetScene()->GetRenderScene();
//		Action->Random = FMath::Rand();
//		Action->GameTime = SceneContext->GetWorld()->GetTimeSeconds();
//		Action->RegisterWithGameInstance(WorldContextObject);
//		return Action;
//	}
//
//	UPROPERTY(BlueprintAssignable)
//	FOnDCVolumeGenerator_AsyncExecutionCompleted Completed;
//
//	FVector3f Position;
//	FMaterialRenderProxy* MaterialRenderProxy;
//	float GameTime;
//	uint32 Random;
//	FScene* Scene;
//
//};