// Copyright Epic Games, Inc. All Rights Reserved.

#include "CauldronGenWorld.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "RHI.h"
#include "GlobalShader.h"
#include "RHICommandList.h"
#include "RenderGraphBuilder.h"
#include "RenderTargetPool.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FCauldronGenWorld"

void FCauldronGenWorld::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("CauldronGen"))->GetBaseDir(), TEXT("Shaders/CauldronGenWorld/Private"));
	AddShaderSourceDirectoryMapping(TEXT("/CauldronGenWorldShaders"), PluginShaderDir);
}

void FCauldronGenWorld::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCauldronGenWorld, CauldronGen)