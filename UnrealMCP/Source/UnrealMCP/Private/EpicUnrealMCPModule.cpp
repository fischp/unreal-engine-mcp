#include "EpicUnrealMCPModule.h"
#include "EpicUnrealMCPBridge.h"
#include "Modules/ModuleManager.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "FEpicUnrealMCPModule"

void FEpicUnrealMCPModule::StartupModule()
{
	UE_LOG(LogTemp, Display, TEXT("Epic Unreal MCP Module has started"));

	// Initialize the MCP Bridge singleton
	// This starts the TCP server that listens for MCP commands
	FEpicUnrealMCPBridge::Initialize();
}

void FEpicUnrealMCPModule::ShutdownModule()
{
	UE_LOG(LogTemp, Display, TEXT("Epic Unreal MCP Module shutting down"));

	// Shutdown the MCP Bridge singleton
	// This stops the TCP server and cleans up resources
	FEpicUnrealMCPBridge::Shutdown();

	UE_LOG(LogTemp, Display, TEXT("Epic Unreal MCP Module has shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FEpicUnrealMCPModule, UnrealMCP)
