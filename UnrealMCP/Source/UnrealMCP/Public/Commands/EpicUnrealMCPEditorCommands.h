#pragma once

#include "CoreMinimal.h"
#include "Json.h"

// Forward declarations for Widget Blueprint support
class UWidgetBlueprint;
class UWidget;

/**
 * Handler class for Editor-related MCP commands
 * Handles viewport control, actor manipulation, level management,
 * and various editor utility functions.
 *
 * UE4.27 compatible version.
 */
class UNREALMCP_API FEpicUnrealMCPEditorCommands
{
public:
	FEpicUnrealMCPEditorCommands();

	// Handle editor commands
	TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
	// Actor manipulation commands
	TSharedPtr<FJsonObject> HandleGetActorsInLevel(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleFindActorsByName(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSpawnActor(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleDeleteActor(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSetActorTransform(const TSharedPtr<FJsonObject>& Params);

	// New tools for UE4.27
	TSharedPtr<FJsonObject> HandleGetUnrealEnginePath(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleGetUnrealProjectPath(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleEditorConsoleCommand(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleEditorProjectInfo(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleEditorGetMapInfo(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleEditorSearchAssets(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleEditorValidateAssets(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleEditorTakeScreenshot(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleEditorMoveCamera(const TSharedPtr<FJsonObject>& Params);

	// ============================================================================
	// Widget Blueprint Commands
	// ============================================================================

	// CREATE Operations
	TSharedPtr<FJsonObject> HandleCreateWidgetBlueprint(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleAddWidgetToBlueprint(const TSharedPtr<FJsonObject>& Params);

	// READ Operations
	TSharedPtr<FJsonObject> HandleListWidgetBlueprints(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleGetWidgetHierarchy(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleGetWidgetProperties(const TSharedPtr<FJsonObject>& Params);

	// UPDATE Operations
	TSharedPtr<FJsonObject> HandleSetWidgetProperties(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleRenameWidget(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleReparentWidget(const TSharedPtr<FJsonObject>& Params);

	// DELETE Operations
	TSharedPtr<FJsonObject> HandleRemoveWidgetFromBlueprint(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleDeleteWidgetBlueprint(const TSharedPtr<FJsonObject>& Params);

	// RUNTIME Operations
	TSharedPtr<FJsonObject> HandleShowWidget(const TSharedPtr<FJsonObject>& Params);

	// Widget Blueprint Helpers
	UWidgetBlueprint* LoadWidgetBlueprint(const FString& AssetPath);
	UWidget* FindWidgetByName(UWidgetBlueprint* WidgetBP, const FString& WidgetName);
	TSharedPtr<FJsonObject> WidgetToJson(UWidget* Widget, bool bRecursive = false);

	// Helper to create error response
	TSharedPtr<FJsonObject> CreateErrorResponse(const FString& ErrorMessage);

	// Helper to get vector from JSON
	FVector GetVectorFromJson(const TSharedPtr<FJsonObject>& Params, const FString& FieldName);

	// Helper to get rotator from JSON
	FRotator GetRotatorFromJson(const TSharedPtr<FJsonObject>& Params, const FString& FieldName);

	// Helper to convert actor to JSON
	TSharedPtr<FJsonValue> ActorToJson(AActor* Actor);
	TSharedPtr<FJsonObject> ActorToJsonObject(AActor* Actor, bool bIncludeSuccess = false);
};
