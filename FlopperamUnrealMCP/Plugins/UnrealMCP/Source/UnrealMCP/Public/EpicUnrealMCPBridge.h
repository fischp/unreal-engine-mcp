#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Http.h"
#include "Json.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Commands/EpicUnrealMCPEditorCommands.h"

class FMCPServerRunnable;

/**
 * MCP Bridge using FTickableEditorObject pattern for UE4.27 compatibility.
 * Handles communication between external tools and the Unreal Editor
 * through a TCP socket connection. Commands are received as JSON and
 * routed to appropriate command handlers.
 *
 * This replaces the UE5+ UEditorSubsystem pattern with a singleton
 * that implements FTickableEditorObject for editor tick support.
 */
class UNREALMCP_API FEpicUnrealMCPBridge : public FTickableEditorObject
{
public:
	FEpicUnrealMCPBridge();
	virtual ~FEpicUnrealMCPBridge();

	// Singleton access
	static FEpicUnrealMCPBridge& Get();
	static void Initialize();
	static void Shutdown();
	static bool IsInitialized() { return Instance.IsValid(); }

	// FTickableEditorObject interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return bIsRunning; }
	virtual TStatId GetStatId() const override;

	// Server functions
	void StartServer();
	void StopServer();
	bool IsRunning() const { return bIsRunning; }

	// Command execution
	FString ExecuteCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

	// PIE (Play in Editor) callbacks
	void OnBeginPIE(bool bIsSimulating);
	void OnEndPIE(bool bIsSimulating);

private:
	// Singleton instance
	static TUniquePtr<FEpicUnrealMCPBridge> Instance;

	// Server state
	bool bIsRunning;
	TSharedPtr<FSocket> ListenerSocket;
	TSharedPtr<FSocket> ConnectionSocket;
	FRunnableThread* ServerThread;

	// Server configuration
	FIPv4Address ServerAddress;
	uint16 Port;

	// Command handler instance
	TSharedPtr<FEpicUnrealMCPEditorCommands> EditorCommands;
};
