#include "EpicUnrealMCPBridge.h"
#include "MCPServerRunnable.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "HAL/RunnableThread.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Camera/CameraActor.h"
#include "JsonObjectConverter.h"
#include "GameFramework/Actor.h"
#include "Engine/Selection.h"
#include "Kismet/GameplayStatics.h"
#include "Async/Async.h"
#include "Editor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Commands/EpicUnrealMCPEditorCommands.h"

// Default settings
#define MCP_SERVER_HOST "127.0.0.1"
#define MCP_SERVER_PORT 55557

// Static singleton instance
TUniquePtr<FEpicUnrealMCPBridge> FEpicUnrealMCPBridge::Instance;

FEpicUnrealMCPBridge::FEpicUnrealMCPBridge()
	: bIsRunning(false)
	, ListenerSocket(nullptr)
	, ConnectionSocket(nullptr)
	, ServerThread(nullptr)
	, Port(MCP_SERVER_PORT)
{
	EditorCommands = MakeShared<FEpicUnrealMCPEditorCommands>();
}

FEpicUnrealMCPBridge::~FEpicUnrealMCPBridge()
{
	StopServer();
	EditorCommands.Reset();
}

// Singleton access
FEpicUnrealMCPBridge& FEpicUnrealMCPBridge::Get()
{
	check(Instance.IsValid());
	return *Instance;
}

void FEpicUnrealMCPBridge::Initialize()
{
	if (!Instance.IsValid())
	{
		UE_LOG(LogTemp, Display, TEXT("FEpicUnrealMCPBridge: Initializing singleton"));
		Instance = MakeUnique<FEpicUnrealMCPBridge>();
		FIPv4Address::Parse(MCP_SERVER_HOST, Instance->ServerAddress);
		Instance->StartServer();
	}
}

void FEpicUnrealMCPBridge::Shutdown()
{
	if (Instance.IsValid())
	{
		UE_LOG(LogTemp, Display, TEXT("FEpicUnrealMCPBridge: Shutting down singleton"));
		Instance->StopServer();
		Instance.Reset();
	}
}

// FTickableEditorObject interface
void FEpicUnrealMCPBridge::Tick(float DeltaTime)
{
	// The actual work is done in the server thread
	// This tick can be used for periodic maintenance if needed
}

TStatId FEpicUnrealMCPBridge::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FEpicUnrealMCPBridge, STATGROUP_Tickables);
}

// Start the MCP server
void FEpicUnrealMCPBridge::StartServer()
{
	if (bIsRunning)
	{
		UE_LOG(LogTemp, Warning, TEXT("FEpicUnrealMCPBridge: Server is already running"));
		return;
	}

	// Create socket subsystem
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("FEpicUnrealMCPBridge: Failed to get socket subsystem"));
		return;
	}

	// Create listener socket
	TSharedPtr<FSocket> NewListenerSocket = MakeShareable(SocketSubsystem->CreateSocket(NAME_Stream, TEXT("UnrealMCPListener"), false));
	if (!NewListenerSocket.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("FEpicUnrealMCPBridge: Failed to create listener socket"));
		return;
	}

	// Allow address reuse for quick restarts
	NewListenerSocket->SetReuseAddr(true);
	NewListenerSocket->SetNonBlocking(true);

	// Bind to address
	FIPv4Endpoint Endpoint(ServerAddress, Port);
	if (!NewListenerSocket->Bind(*Endpoint.ToInternetAddr()))
	{
		UE_LOG(LogTemp, Error, TEXT("FEpicUnrealMCPBridge: Failed to bind listener socket to %s:%d"), *ServerAddress.ToString(), Port);
		return;
	}

	// Start listening
	if (!NewListenerSocket->Listen(5))
	{
		UE_LOG(LogTemp, Error, TEXT("FEpicUnrealMCPBridge: Failed to start listening"));
		return;
	}

	ListenerSocket = NewListenerSocket;
	bIsRunning = true;
	UE_LOG(LogTemp, Display, TEXT("FEpicUnrealMCPBridge: Server started on %s:%d"), *ServerAddress.ToString(), Port);

	// Start server thread
	ServerThread = FRunnableThread::Create(
		new FMCPServerRunnable(this, ListenerSocket),
		TEXT("UnrealMCPServerThread"),
		0, TPri_Normal
	);

	if (!ServerThread)
	{
		UE_LOG(LogTemp, Error, TEXT("FEpicUnrealMCPBridge: Failed to create server thread"));
		StopServer();
		return;
	}
}

// Stop the MCP server
void FEpicUnrealMCPBridge::StopServer()
{
	if (!bIsRunning)
	{
		return;
	}

	bIsRunning = false;

	// Clean up thread
	if (ServerThread)
	{
		ServerThread->Kill(true);
		delete ServerThread;
		ServerThread = nullptr;
	}

	// Close sockets
	if (ConnectionSocket.IsValid())
	{
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket.Get());
		ConnectionSocket.Reset();
	}

	if (ListenerSocket.IsValid())
	{
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenerSocket.Get());
		ListenerSocket.Reset();
	}

	UE_LOG(LogTemp, Display, TEXT("FEpicUnrealMCPBridge: Server stopped"));
}

// Execute a command received from a client
FString FEpicUnrealMCPBridge::ExecuteCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
	UE_LOG(LogTemp, Display, TEXT("FEpicUnrealMCPBridge: Executing command: %s"), *CommandType);

	// Create a promise to wait for the result
	TPromise<FString> Promise;
	TFuture<FString> Future = Promise.GetFuture();

	// Queue execution on Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, CommandType, Params, Promise = MoveTemp(Promise)]() mutable
	{
		TSharedPtr<FJsonObject> ResponseJson = MakeShareable(new FJsonObject);

		try
		{
			TSharedPtr<FJsonObject> ResultJson;

			if (CommandType == TEXT("ping"))
			{
				ResultJson = MakeShareable(new FJsonObject);
				ResultJson->SetStringField(TEXT("message"), TEXT("pong"));
			}
			// All editor commands (existing + new)
			else if (CommandType == TEXT("get_actors_in_level") ||
					 CommandType == TEXT("find_actors_by_name") ||
					 CommandType == TEXT("spawn_actor") ||
					 CommandType == TEXT("delete_actor") ||
					 CommandType == TEXT("set_actor_transform") ||
					 CommandType == TEXT("get_unreal_engine_path") ||
					 CommandType == TEXT("get_unreal_project_path") ||
					 CommandType == TEXT("editor_console_command") ||
					 CommandType == TEXT("editor_project_info") ||
					 CommandType == TEXT("editor_get_map_info") ||
					 CommandType == TEXT("editor_search_assets") ||
					 CommandType == TEXT("editor_validate_assets") ||
					 CommandType == TEXT("editor_take_screenshot") ||
					 CommandType == TEXT("editor_move_camera"))
			{
				ResultJson = EditorCommands->HandleCommand(CommandType, Params);
			}
			else
			{
				ResponseJson->SetStringField(TEXT("status"), TEXT("error"));
				ResponseJson->SetStringField(TEXT("error"), FString::Printf(TEXT("Unknown command: %s"), *CommandType));

				FString ResultString;
				TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
				FJsonSerializer::Serialize(ResponseJson.ToSharedRef(), Writer);
				Promise.SetValue(ResultString);
				return;
			}

			// Check if the result contains an error
			bool bSuccess = true;
			FString ErrorMessage;

			if (ResultJson.IsValid() && ResultJson->HasField(TEXT("success")))
			{
				bSuccess = ResultJson->GetBoolField(TEXT("success"));
				if (!bSuccess && ResultJson->HasField(TEXT("error")))
				{
					ErrorMessage = ResultJson->GetStringField(TEXT("error"));
				}
			}

			if (bSuccess)
			{
				// Set success status and include the result
				ResponseJson->SetStringField(TEXT("status"), TEXT("success"));
				ResponseJson->SetObjectField(TEXT("result"), ResultJson);
			}
			else
			{
				// Set error status and include the error message
				ResponseJson->SetStringField(TEXT("status"), TEXT("error"));
				ResponseJson->SetStringField(TEXT("error"), ErrorMessage);
			}
		}
		catch (const std::exception& e)
		{
			ResponseJson->SetStringField(TEXT("status"), TEXT("error"));
			ResponseJson->SetStringField(TEXT("error"), UTF8_TO_TCHAR(e.what()));
		}

		FString ResultString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
		FJsonSerializer::Serialize(ResponseJson.ToSharedRef(), Writer);
		Promise.SetValue(ResultString);
	});

	return Future.Get();
}
