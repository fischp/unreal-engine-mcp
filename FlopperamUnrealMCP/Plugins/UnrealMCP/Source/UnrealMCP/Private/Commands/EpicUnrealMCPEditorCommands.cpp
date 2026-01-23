#include "Commands/EpicUnrealMCPEditorCommands.h"
#include "Editor.h"
#include "UObject/UnrealType.h"
#include "UObject/PropertyPortFlags.h"
#include "UObject/TextProperty.h"
#include "LevelEditorViewport.h"
#include "EditorViewportClient.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "Engine/GameViewportClient.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/App.h"
#include "Misc/EngineVersion.h"
#include "GameFramework/Actor.h"
#include "Engine/Selection.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Camera/CameraActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/LevelStreaming.h"
#include "AssetRegistryModule.h"
#include "IAssetRegistry.h"
#include "ProjectDescriptor.h"

// UMG/Widget Blueprint includes
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Components/PanelWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/ScrollBox.h"
#include "WidgetBlueprintFactory.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "ObjectTools.h"
#include "Engine/DataTable.h"

FEpicUnrealMCPEditorCommands::FEpicUnrealMCPEditorCommands()
{
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
	// Actor manipulation commands
	if (CommandType == TEXT("get_actors_in_level"))
	{
		return HandleGetActorsInLevel(Params);
	}
	else if (CommandType == TEXT("find_actors_by_name"))
	{
		return HandleFindActorsByName(Params);
	}
	else if (CommandType == TEXT("spawn_actor"))
	{
		return HandleSpawnActor(Params);
	}
	else if (CommandType == TEXT("delete_actor"))
	{
		return HandleDeleteActor(Params);
	}
	else if (CommandType == TEXT("set_actor_transform"))
	{
		return HandleSetActorTransform(Params);
	}
	// New tools
	else if (CommandType == TEXT("get_unreal_engine_path"))
	{
		return HandleGetUnrealEnginePath(Params);
	}
	else if (CommandType == TEXT("get_unreal_project_path"))
	{
		return HandleGetUnrealProjectPath(Params);
	}
	else if (CommandType == TEXT("editor_console_command"))
	{
		return HandleEditorConsoleCommand(Params);
	}
	else if (CommandType == TEXT("editor_project_info"))
	{
		return HandleEditorProjectInfo(Params);
	}
	else if (CommandType == TEXT("editor_get_map_info"))
	{
		return HandleEditorGetMapInfo(Params);
	}
	else if (CommandType == TEXT("editor_search_assets"))
	{
		return HandleEditorSearchAssets(Params);
	}
	else if (CommandType == TEXT("editor_validate_assets"))
	{
		return HandleEditorValidateAssets(Params);
	}
	else if (CommandType == TEXT("editor_take_screenshot"))
	{
		return HandleEditorTakeScreenshot(Params);
	}
	else if (CommandType == TEXT("editor_move_camera"))
	{
		return HandleEditorMoveCamera(Params);
	}
	// Widget Blueprint commands - CREATE
	else if (CommandType == TEXT("create_widget_blueprint"))
	{
		return HandleCreateWidgetBlueprint(Params);
	}
	else if (CommandType == TEXT("add_widget_to_blueprint"))
	{
		return HandleAddWidgetToBlueprint(Params);
	}
	// Widget Blueprint commands - READ
	else if (CommandType == TEXT("list_widget_blueprints"))
	{
		return HandleListWidgetBlueprints(Params);
	}
	else if (CommandType == TEXT("get_widget_hierarchy"))
	{
		return HandleGetWidgetHierarchy(Params);
	}
	else if (CommandType == TEXT("get_widget_properties"))
	{
		return HandleGetWidgetProperties(Params);
	}
	// Widget Blueprint commands - UPDATE
	else if (CommandType == TEXT("set_widget_properties"))
	{
		return HandleSetWidgetProperties(Params);
	}
	else if (CommandType == TEXT("rename_widget"))
	{
		return HandleRenameWidget(Params);
	}
	else if (CommandType == TEXT("reparent_widget"))
	{
		return HandleReparentWidget(Params);
	}
	// Widget Blueprint commands - DELETE
	else if (CommandType == TEXT("remove_widget_from_blueprint"))
	{
		return HandleRemoveWidgetFromBlueprint(Params);
	}
	else if (CommandType == TEXT("delete_widget_blueprint"))
	{
		return HandleDeleteWidgetBlueprint(Params);
	}
	// Widget Blueprint commands - RUNTIME
	else if (CommandType == TEXT("show_widget"))
	{
		return HandleShowWidget(Params);
	}
	// Actor Property commands
	else if (CommandType == TEXT("get_actor_property"))
	{
		return HandleGetActorProperty(Params);
	}
	else if (CommandType == TEXT("set_actor_property"))
	{
		return HandleSetActorProperty(Params);
	}
	// Blueprint Actor commands
	else if (CommandType == TEXT("spawn_blueprint_actor"))
	{
		return HandleSpawnBlueprintActor(Params);
	}
	else if (CommandType == TEXT("copy_actor"))
	{
		return HandleCopyActor(Params);
	}
	// Asset Property commands
	else if (CommandType == TEXT("get_asset_property"))
	{
		return HandleGetAssetProperty(Params);
	}
	else if (CommandType == TEXT("set_asset_property"))
	{
		return HandleSetAssetProperty(Params);
	}
	// Blueprint Default Property commands
	else if (CommandType == TEXT("get_blueprint_default_property"))
	{
		return HandleGetBlueprintDefaultProperty(Params);
	}
	else if (CommandType == TEXT("set_blueprint_default_property"))
	{
		return HandleSetBlueprintDefaultProperty(Params);
	}
	// Data Table commands
	else if (CommandType == TEXT("list_data_table_rows"))
	{
		return HandleListDataTableRows(Params);
	}
	else if (CommandType == TEXT("get_data_table_row"))
	{
		return HandleGetDataTableRow(Params);
	}
	else if (CommandType == TEXT("set_data_table_row_field"))
	{
		return HandleSetDataTableRowField(Params);
	}
	else if (CommandType == TEXT("add_data_table_row"))
	{
		return HandleAddDataTableRow(Params);
	}
	else if (CommandType == TEXT("delete_data_table_row"))
	{
		return HandleDeleteDataTableRow(Params);
	}

	return CreateErrorResponse(FString::Printf(TEXT("Unknown editor command: %s"), *CommandType));
}

// Helper functions
TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::CreateErrorResponse(const FString& ErrorMessage)
{
	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), false);
	Result->SetStringField(TEXT("error"), ErrorMessage);
	return Result;
}

FVector FEpicUnrealMCPEditorCommands::GetVectorFromJson(const TSharedPtr<FJsonObject>& Params, const FString& FieldName)
{
	FVector Result(0.0f, 0.0f, 0.0f);

	if (Params->HasField(FieldName))
	{
		const TArray<TSharedPtr<FJsonValue>>* ArrayValue;
		if (Params->TryGetArrayField(FieldName, ArrayValue) && ArrayValue->Num() >= 3)
		{
			Result.X = (*ArrayValue)[0]->AsNumber();
			Result.Y = (*ArrayValue)[1]->AsNumber();
			Result.Z = (*ArrayValue)[2]->AsNumber();
		}
	}

	return Result;
}

FRotator FEpicUnrealMCPEditorCommands::GetRotatorFromJson(const TSharedPtr<FJsonObject>& Params, const FString& FieldName)
{
	FRotator Result(0.0f, 0.0f, 0.0f);

	if (Params->HasField(FieldName))
	{
		const TArray<TSharedPtr<FJsonValue>>* ArrayValue;
		if (Params->TryGetArrayField(FieldName, ArrayValue) && ArrayValue->Num() >= 3)
		{
			Result.Pitch = (*ArrayValue)[0]->AsNumber();
			Result.Yaw = (*ArrayValue)[1]->AsNumber();
			Result.Roll = (*ArrayValue)[2]->AsNumber();
		}
	}

	return Result;
}

TSharedPtr<FJsonValue> FEpicUnrealMCPEditorCommands::ActorToJson(AActor* Actor)
{
	TSharedPtr<FJsonObject> ActorObj = MakeShared<FJsonObject>();

	ActorObj->SetStringField(TEXT("name"), Actor->GetName());
	ActorObj->SetStringField(TEXT("class"), Actor->GetClass()->GetName());

	FVector Location = Actor->GetActorLocation();
	TArray<TSharedPtr<FJsonValue>> LocationArray;
	LocationArray.Add(MakeShared<FJsonValueNumber>(Location.X));
	LocationArray.Add(MakeShared<FJsonValueNumber>(Location.Y));
	LocationArray.Add(MakeShared<FJsonValueNumber>(Location.Z));
	ActorObj->SetArrayField(TEXT("location"), LocationArray);

	FRotator Rotation = Actor->GetActorRotation();
	TArray<TSharedPtr<FJsonValue>> RotationArray;
	RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Pitch));
	RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Yaw));
	RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Roll));
	ActorObj->SetArrayField(TEXT("rotation"), RotationArray);

	FVector Scale = Actor->GetActorScale3D();
	TArray<TSharedPtr<FJsonValue>> ScaleArray;
	ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.X));
	ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.Y));
	ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.Z));
	ActorObj->SetArrayField(TEXT("scale"), ScaleArray);

	return MakeShared<FJsonValueObject>(ActorObj);
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::ActorToJsonObject(AActor* Actor, bool bIncludeSuccess)
{
	TSharedPtr<FJsonObject> ActorObj = MakeShared<FJsonObject>();

	if (bIncludeSuccess)
	{
		ActorObj->SetBoolField(TEXT("success"), true);
	}

	ActorObj->SetStringField(TEXT("name"), Actor->GetName());
	ActorObj->SetStringField(TEXT("class"), Actor->GetClass()->GetName());

	FVector Location = Actor->GetActorLocation();
	TArray<TSharedPtr<FJsonValue>> LocationArray;
	LocationArray.Add(MakeShared<FJsonValueNumber>(Location.X));
	LocationArray.Add(MakeShared<FJsonValueNumber>(Location.Y));
	LocationArray.Add(MakeShared<FJsonValueNumber>(Location.Z));
	ActorObj->SetArrayField(TEXT("location"), LocationArray);

	FRotator Rotation = Actor->GetActorRotation();
	TArray<TSharedPtr<FJsonValue>> RotationArray;
	RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Pitch));
	RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Yaw));
	RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Roll));
	ActorObj->SetArrayField(TEXT("rotation"), RotationArray);

	FVector Scale = Actor->GetActorScale3D();
	TArray<TSharedPtr<FJsonValue>> ScaleArray;
	ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.X));
	ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.Y));
	ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.Z));
	ActorObj->SetArrayField(TEXT("scale"), ScaleArray);

	return ActorObj;
}

// ============================================================================
// Existing Actor Commands
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleGetActorsInLevel(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		return CreateErrorResponse(TEXT("No editor world available"));
	}

	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

	TArray<TSharedPtr<FJsonValue>> ActorArray;
	for (AActor* Actor : AllActors)
	{
		if (Actor)
		{
			ActorArray.Add(ActorToJson(Actor));
		}
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetArrayField(TEXT("actors"), ActorArray);
	ResultObj->SetNumberField(TEXT("count"), ActorArray.Num());

	return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleFindActorsByName(const TSharedPtr<FJsonObject>& Params)
{
	FString Pattern;
	if (!Params->TryGetStringField(TEXT("pattern"), Pattern))
	{
		return CreateErrorResponse(TEXT("Missing 'pattern' parameter"));
	}

	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		return CreateErrorResponse(TEXT("No editor world available"));
	}

	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

	TArray<TSharedPtr<FJsonValue>> MatchingActors;
	for (AActor* Actor : AllActors)
	{
		if (Actor && Actor->GetName().Contains(Pattern))
		{
			MatchingActors.Add(ActorToJson(Actor));
		}
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetArrayField(TEXT("actors"), MatchingActors);
	ResultObj->SetNumberField(TEXT("count"), MatchingActors.Num());

	return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleSpawnActor(const TSharedPtr<FJsonObject>& Params)
{
	// Get required parameters
	FString ActorType;
	if (!Params->TryGetStringField(TEXT("type"), ActorType))
	{
		return CreateErrorResponse(TEXT("Missing 'type' parameter"));
	}

	FString ActorName;
	if (!Params->TryGetStringField(TEXT("name"), ActorName))
	{
		return CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	// Get optional transform parameters
	FVector Location = GetVectorFromJson(Params, TEXT("location"));
	FRotator Rotation = GetRotatorFromJson(Params, TEXT("rotation"));
	FVector Scale = FVector(1.0f, 1.0f, 1.0f);
	if (Params->HasField(TEXT("scale")))
	{
		Scale = GetVectorFromJson(Params, TEXT("scale"));
	}

	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		return CreateErrorResponse(TEXT("Failed to get editor world"));
	}

	// Check if an actor with this name already exists
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
	for (AActor* Actor : AllActors)
	{
		if (Actor && Actor->GetName() == ActorName)
		{
			return CreateErrorResponse(FString::Printf(TEXT("Actor with name '%s' already exists"), *ActorName));
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = *ActorName;

	AActor* NewActor = nullptr;

	if (ActorType == TEXT("StaticMeshActor"))
	{
		AStaticMeshActor* NewMeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
		if (NewMeshActor)
		{
			// Check for an optional static_mesh parameter to assign a mesh
			FString MeshPath;
			if (Params->TryGetStringField(TEXT("static_mesh"), MeshPath))
			{
				UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
				if (Mesh)
				{
					NewMeshActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Could not find static mesh at path: %s"), *MeshPath);
				}
			}
		}
		NewActor = NewMeshActor;
	}
	else if (ActorType == TEXT("PointLight"))
	{
		NewActor = World->SpawnActor<APointLight>(APointLight::StaticClass(), Location, Rotation, SpawnParams);
	}
	else if (ActorType == TEXT("SpotLight"))
	{
		NewActor = World->SpawnActor<ASpotLight>(ASpotLight::StaticClass(), Location, Rotation, SpawnParams);
	}
	else if (ActorType == TEXT("DirectionalLight"))
	{
		NewActor = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), Location, Rotation, SpawnParams);
	}
	else if (ActorType == TEXT("CameraActor"))
	{
		NewActor = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), Location, Rotation, SpawnParams);
	}
	else
	{
		return CreateErrorResponse(FString::Printf(TEXT("Unknown actor type: %s"), *ActorType));
	}

	if (NewActor)
	{
		// Set scale
		FTransform Transform = NewActor->GetTransform();
		Transform.SetScale3D(Scale);
		NewActor->SetActorTransform(Transform);

		return ActorToJsonObject(NewActor, true);
	}

	return CreateErrorResponse(TEXT("Failed to create actor"));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleDeleteActor(const TSharedPtr<FJsonObject>& Params)
{
	FString ActorName;
	if (!Params->TryGetStringField(TEXT("name"), ActorName))
	{
		return CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		return CreateErrorResponse(TEXT("No editor world available"));
	}

	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

	for (AActor* Actor : AllActors)
	{
		if (Actor && Actor->GetName() == ActorName)
		{
			// Store actor info before deletion for the response
			TSharedPtr<FJsonObject> ActorInfo = ActorToJsonObject(Actor);

			// Delete the actor
			Actor->Destroy();

			TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
			ResultObj->SetBoolField(TEXT("success"), true);
			ResultObj->SetObjectField(TEXT("deleted_actor"), ActorInfo);
			return ResultObj;
		}
	}

	return CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleSetActorTransform(const TSharedPtr<FJsonObject>& Params)
{
	FString ActorName;
	if (!Params->TryGetStringField(TEXT("name"), ActorName))
	{
		return CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		return CreateErrorResponse(TEXT("No editor world available"));
	}

	// Find the actor
	AActor* TargetActor = nullptr;
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

	for (AActor* Actor : AllActors)
	{
		if (Actor && Actor->GetName() == ActorName)
		{
			TargetActor = Actor;
			break;
		}
	}

	if (!TargetActor)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
	}

	// Get transform parameters
	FTransform NewTransform = TargetActor->GetTransform();

	if (Params->HasField(TEXT("location")))
	{
		NewTransform.SetLocation(GetVectorFromJson(Params, TEXT("location")));
	}
	if (Params->HasField(TEXT("rotation")))
	{
		NewTransform.SetRotation(FQuat(GetRotatorFromJson(Params, TEXT("rotation"))));
	}
	if (Params->HasField(TEXT("scale")))
	{
		NewTransform.SetScale3D(GetVectorFromJson(Params, TEXT("scale")));
	}

	// Set the new transform
	TargetActor->SetActorTransform(NewTransform);

	// Return updated actor info
	return ActorToJsonObject(TargetActor, true);
}

// ============================================================================
// New Tools for UE4.27
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleGetUnrealEnginePath(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();

	FString EnginePath = FPaths::EngineDir();
	FString EngineVersion = FEngineVersion::Current().ToString();

	Result->SetStringField(TEXT("engine_path"), EnginePath);
	Result->SetStringField(TEXT("engine_version"), EngineVersion);
	Result->SetBoolField(TEXT("success"), true);

	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleGetUnrealProjectPath(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();

	FString ProjectDir = FPaths::ProjectDir();
	FString ProjectName = FApp::GetProjectName();
	FString ProjectFile = FPaths::GetProjectFilePath();

	Result->SetStringField(TEXT("project_path"), ProjectDir);
	Result->SetStringField(TEXT("project_name"), ProjectName);
	Result->SetStringField(TEXT("project_file"), ProjectFile);
	Result->SetBoolField(TEXT("success"), true);

	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleEditorConsoleCommand(const TSharedPtr<FJsonObject>& Params)
{
	FString Command;
	if (!Params->TryGetStringField(TEXT("command"), Command))
	{
		return CreateErrorResponse(TEXT("Missing 'command' parameter"));
	}

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();

	if (GEngine && GEditor)
	{
		UWorld* World = GEditor->GetEditorWorldContext().World();
		if (World)
		{
			GEngine->Exec(World, *Command);
			Result->SetStringField(TEXT("command"), Command);
			Result->SetBoolField(TEXT("success"), true);
		}
		else
		{
			return CreateErrorResponse(TEXT("No world available"));
		}
	}
	else
	{
		return CreateErrorResponse(TEXT("Engine or Editor not available"));
	}

	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleEditorProjectInfo(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();

	Result->SetStringField(TEXT("project_name"), FApp::GetProjectName());
	Result->SetStringField(TEXT("engine_version"), FEngineVersion::Current().ToString());
	Result->SetStringField(TEXT("project_dir"), FPaths::ProjectDir());
	Result->SetStringField(TEXT("project_file"), FPaths::GetProjectFilePath());

	// Try to get project descriptor info
	FString ProjectFilePath = FPaths::GetProjectFilePath();
	if (!ProjectFilePath.IsEmpty())
	{
		FProjectDescriptor ProjectDescriptor;
		FText OutError;
		if (ProjectDescriptor.Load(ProjectFilePath, OutError))
		{
			Result->SetStringField(TEXT("description"), ProjectDescriptor.Description);
			Result->SetStringField(TEXT("category"), ProjectDescriptor.Category);
		}
	}

	Result->SetBoolField(TEXT("success"), true);

	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleEditorGetMapInfo(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();

	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		return CreateErrorResponse(TEXT("No world available"));
	}

	Result->SetStringField(TEXT("level_name"), World->GetMapName());
	Result->SetStringField(TEXT("level_path"), World->GetOutermost()->GetName());

	// Get streaming levels
	TArray<TSharedPtr<FJsonValue>> SubLevels;
	const TArray<ULevelStreaming*>& StreamingLevels = World->GetStreamingLevels();
	for (ULevelStreaming* StreamingLevel : StreamingLevels)
	{
		if (StreamingLevel)
		{
			TSharedPtr<FJsonObject> LevelObj = MakeShared<FJsonObject>();
			LevelObj->SetStringField(TEXT("name"), StreamingLevel->GetWorldAssetPackageName());
			LevelObj->SetBoolField(TEXT("is_loaded"), StreamingLevel->IsLevelLoaded());
			LevelObj->SetBoolField(TEXT("is_visible"), StreamingLevel->IsLevelVisible());
			SubLevels.Add(MakeShared<FJsonValueObject>(LevelObj));
		}
	}
	Result->SetArrayField(TEXT("streaming_levels"), SubLevels);

	// Get actor count
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
	Result->SetNumberField(TEXT("actor_count"), AllActors.Num());

	Result->SetBoolField(TEXT("success"), true);

	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleEditorSearchAssets(const TSharedPtr<FJsonObject>& Params)
{
	FString SearchPattern;
	if (!Params->TryGetStringField(TEXT("pattern"), SearchPattern))
	{
		return CreateErrorResponse(TEXT("Missing 'pattern' parameter"));
	}

	FString ClassFilter;
	Params->TryGetStringField(TEXT("class_filter"), ClassFilter);

	int32 MaxResults = 100;
	Params->TryGetNumberField(TEXT("max_results"), MaxResults);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AssetDataList;

	// Build filter
	FARFilter Filter;
	if (!ClassFilter.IsEmpty())
	{
		Filter.ClassNames.Add(*ClassFilter);
	}
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;

	AssetRegistry.GetAllAssets(AssetDataList);

	TArray<TSharedPtr<FJsonValue>> MatchingAssets;
	int32 Count = 0;

	for (const FAssetData& AssetData : AssetDataList)
	{
		if (Count >= MaxResults)
		{
			break;
		}

		FString AssetName = AssetData.AssetName.ToString();
		FString AssetPath = AssetData.ObjectPath.ToString();

		// Check if asset matches the search pattern
		if (AssetName.Contains(SearchPattern) || AssetPath.Contains(SearchPattern))
		{
			// Apply class filter if specified
			if (!ClassFilter.IsEmpty() && !AssetData.AssetClass.ToString().Contains(ClassFilter))
			{
				continue;
			}

			TSharedPtr<FJsonObject> AssetObj = MakeShared<FJsonObject>();
			AssetObj->SetStringField(TEXT("name"), AssetName);
			AssetObj->SetStringField(TEXT("path"), AssetPath);
			AssetObj->SetStringField(TEXT("class"), AssetData.AssetClass.ToString());
			AssetObj->SetStringField(TEXT("package"), AssetData.PackageName.ToString());

			MatchingAssets.Add(MakeShared<FJsonValueObject>(AssetObj));
			Count++;
		}
	}

	Result->SetArrayField(TEXT("assets"), MatchingAssets);
	Result->SetNumberField(TEXT("count"), MatchingAssets.Num());
	Result->SetBoolField(TEXT("success"), true);

	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleEditorValidateAssets(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath;
	Params->TryGetStringField(TEXT("asset_path"), AssetPath);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AssetDataList;
	if (AssetPath.IsEmpty())
	{
		// Validate all assets (limited to 1000 for performance)
		AssetRegistry.GetAllAssets(AssetDataList);
		if (AssetDataList.Num() > 1000)
		{
			AssetDataList.SetNum(1000);
		}
	}
	else
	{
		AssetRegistry.GetAssetsByPath(*AssetPath, AssetDataList, true);
	}

	TArray<TSharedPtr<FJsonValue>> ValidAssets;
	TArray<TSharedPtr<FJsonValue>> InvalidAssets;

	for (const FAssetData& AssetData : AssetDataList)
	{
		TSharedPtr<FJsonObject> AssetObj = MakeShared<FJsonObject>();
		AssetObj->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		AssetObj->SetStringField(TEXT("path"), AssetData.ObjectPath.ToString());
		AssetObj->SetStringField(TEXT("class"), AssetData.AssetClass.ToString());

		if (AssetData.IsValid())
		{
			ValidAssets.Add(MakeShared<FJsonValueObject>(AssetObj));
		}
		else
		{
			InvalidAssets.Add(MakeShared<FJsonValueObject>(AssetObj));
		}
	}

	Result->SetArrayField(TEXT("valid_assets"), ValidAssets);
	Result->SetArrayField(TEXT("invalid_assets"), InvalidAssets);
	Result->SetNumberField(TEXT("valid_count"), ValidAssets.Num());
	Result->SetNumberField(TEXT("invalid_count"), InvalidAssets.Num());
	Result->SetBoolField(TEXT("success"), true);

	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleEditorTakeScreenshot(const TSharedPtr<FJsonObject>& Params)
{
	FString Filename;
	if (!Params->TryGetStringField(TEXT("filename"), Filename))
	{
		Filename = FString::Printf(TEXT("Screenshot_%s"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	}

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();

	FString ScreenshotDir = FPaths::ScreenShotDir();
	FString ScreenshotPath = ScreenshotDir / Filename + TEXT(".png");

	// Get the active viewport
	if (GEditor && GEditor->GetActiveViewport())
	{
		FViewport* Viewport = GEditor->GetActiveViewport();

		// Take screenshot
		TArray<FColor> Bitmap;
		if (Viewport->ReadPixels(Bitmap))
		{
			int32 Width = Viewport->GetSizeXY().X;
			int32 Height = Viewport->GetSizeXY().Y;

			// Ensure screenshot directory exists
			IFileManager::Get().MakeDirectory(*ScreenshotDir, true);

			// Save the screenshot
			TArray<uint8> CompressedBitmap;
			FImageUtils::CompressImageArray(Width, Height, Bitmap, CompressedBitmap);

			if (FFileHelper::SaveArrayToFile(CompressedBitmap, *ScreenshotPath))
			{
				Result->SetStringField(TEXT("screenshot_path"), ScreenshotPath);
				Result->SetNumberField(TEXT("width"), Width);
				Result->SetNumberField(TEXT("height"), Height);
				Result->SetBoolField(TEXT("success"), true);
			}
			else
			{
				return CreateErrorResponse(TEXT("Failed to save screenshot file"));
			}
		}
		else
		{
			return CreateErrorResponse(TEXT("Failed to read viewport pixels"));
		}
	}
	else
	{
		return CreateErrorResponse(TEXT("No active viewport"));
	}

	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleEditorMoveCamera(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();

	if (!GEditor || !GEditor->GetActiveViewport())
	{
		return CreateErrorResponse(TEXT("No active viewport"));
	}

	FEditorViewportClient* ViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
	if (!ViewportClient)
	{
		return CreateErrorResponse(TEXT("No viewport client"));
	}

	FVector NewLocation = ViewportClient->GetViewLocation();
	FRotator NewRotation = ViewportClient->GetViewRotation();

	if (Params->HasField(TEXT("location")))
	{
		NewLocation = GetVectorFromJson(Params, TEXT("location"));
	}
	if (Params->HasField(TEXT("rotation")))
	{
		NewRotation = GetRotatorFromJson(Params, TEXT("rotation"));
	}

	ViewportClient->SetViewLocation(NewLocation);
	ViewportClient->SetViewRotation(NewRotation);

	// Return new camera position
	TArray<TSharedPtr<FJsonValue>> LocationArray;
	LocationArray.Add(MakeShared<FJsonValueNumber>(NewLocation.X));
	LocationArray.Add(MakeShared<FJsonValueNumber>(NewLocation.Y));
	LocationArray.Add(MakeShared<FJsonValueNumber>(NewLocation.Z));
	Result->SetArrayField(TEXT("location"), LocationArray);

	TArray<TSharedPtr<FJsonValue>> RotationArray;
	RotationArray.Add(MakeShared<FJsonValueNumber>(NewRotation.Pitch));
	RotationArray.Add(MakeShared<FJsonValueNumber>(NewRotation.Yaw));
	RotationArray.Add(MakeShared<FJsonValueNumber>(NewRotation.Roll));
	Result->SetArrayField(TEXT("rotation"), RotationArray);

	Result->SetBoolField(TEXT("success"), true);

	return Result;
}

// ============================================================================
// Widget Blueprint Helper Methods
// ============================================================================

UWidgetBlueprint* FEpicUnrealMCPEditorCommands::LoadWidgetBlueprint(const FString& AssetPath)
{
	return LoadObject<UWidgetBlueprint>(nullptr, *AssetPath);
}

UWidget* FEpicUnrealMCPEditorCommands::FindWidgetByName(UWidgetBlueprint* WidgetBP, const FString& WidgetName)
{
	if (!WidgetBP || !WidgetBP->WidgetTree)
	{
		return nullptr;
	}

	// Search through all widgets in the tree
	TArray<UWidget*> AllWidgets;
	WidgetBP->WidgetTree->GetAllWidgets(AllWidgets);

	for (UWidget* Widget : AllWidgets)
	{
		if (Widget && Widget->GetName() == WidgetName)
		{
			return Widget;
		}
	}

	return nullptr;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::WidgetToJson(UWidget* Widget, bool bRecursive)
{
	if (!Widget)
	{
		return nullptr;
	}

	TSharedPtr<FJsonObject> WidgetObj = MakeShared<FJsonObject>();
	WidgetObj->SetStringField(TEXT("name"), Widget->GetName());
	WidgetObj->SetStringField(TEXT("class"), Widget->GetClass()->GetName());
	WidgetObj->SetBoolField(TEXT("is_visible"), Widget->IsVisible());

	// Add slot information if available
	if (Widget->Slot)
	{
		TSharedPtr<FJsonObject> SlotObj = MakeShared<FJsonObject>();
		SlotObj->SetStringField(TEXT("slot_class"), Widget->Slot->GetClass()->GetName());

		// Handle CanvasPanelSlot properties
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
		{
			FVector2D Position = CanvasSlot->GetPosition();
			FVector2D Size = CanvasSlot->GetSize();
			FAnchors Anchors = CanvasSlot->GetAnchors();

			TArray<TSharedPtr<FJsonValue>> PosArray;
			PosArray.Add(MakeShared<FJsonValueNumber>(Position.X));
			PosArray.Add(MakeShared<FJsonValueNumber>(Position.Y));
			SlotObj->SetArrayField(TEXT("position"), PosArray);

			TArray<TSharedPtr<FJsonValue>> SizeArray;
			SizeArray.Add(MakeShared<FJsonValueNumber>(Size.X));
			SizeArray.Add(MakeShared<FJsonValueNumber>(Size.Y));
			SlotObj->SetArrayField(TEXT("size"), SizeArray);

			TArray<TSharedPtr<FJsonValue>> AnchorsArray;
			AnchorsArray.Add(MakeShared<FJsonValueNumber>(Anchors.Minimum.X));
			AnchorsArray.Add(MakeShared<FJsonValueNumber>(Anchors.Minimum.Y));
			AnchorsArray.Add(MakeShared<FJsonValueNumber>(Anchors.Maximum.X));
			AnchorsArray.Add(MakeShared<FJsonValueNumber>(Anchors.Maximum.Y));
			SlotObj->SetArrayField(TEXT("anchors"), AnchorsArray);
		}

		WidgetObj->SetObjectField(TEXT("slot"), SlotObj);
	}

	// If it's a panel, include children
	UPanelWidget* Panel = Cast<UPanelWidget>(Widget);
	if (Panel && bRecursive)
	{
		TArray<TSharedPtr<FJsonValue>> ChildrenArray;
		for (int32 i = 0; i < Panel->GetChildrenCount(); ++i)
		{
			UWidget* Child = Panel->GetChildAt(i);
			if (Child)
			{
				TSharedPtr<FJsonObject> ChildJson = WidgetToJson(Child, true);
				if (ChildJson.IsValid())
				{
					ChildrenArray.Add(MakeShared<FJsonValueObject>(ChildJson));
				}
			}
		}
		WidgetObj->SetArrayField(TEXT("children"), ChildrenArray);
	}

	return WidgetObj;
}

// ============================================================================
// Widget Blueprint READ Operations
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleListWidgetBlueprints(const TSharedPtr<FJsonObject>& Params)
{
	FString SearchPath = TEXT("/Game");
	Params->TryGetStringField(TEXT("path"), SearchPath);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AssetDataList;
	FARFilter Filter;
	Filter.ClassNames.Add(UWidgetBlueprint::StaticClass()->GetFName());
	Filter.PackagePaths.Add(*SearchPath);
	Filter.bRecursivePaths = true;

	AssetRegistry.GetAssets(Filter, AssetDataList);

	TArray<TSharedPtr<FJsonValue>> WidgetBPArray;
	for (const FAssetData& AssetData : AssetDataList)
	{
		TSharedPtr<FJsonObject> WBPObj = MakeShared<FJsonObject>();
		WBPObj->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		WBPObj->SetStringField(TEXT("path"), AssetData.ObjectPath.ToString());
		WBPObj->SetStringField(TEXT("package"), AssetData.PackageName.ToString());
		WidgetBPArray.Add(MakeShared<FJsonValueObject>(WBPObj));
	}

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), true);
	Result->SetArrayField(TEXT("widget_blueprints"), WidgetBPArray);
	Result->SetNumberField(TEXT("count"), WidgetBPArray.Num());
	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleGetWidgetHierarchy(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintPath;
	if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
	{
		return CreateErrorResponse(TEXT("Missing 'blueprint_path' parameter"));
	}

	UWidgetBlueprint* WidgetBP = LoadWidgetBlueprint(BlueprintPath);
	if (!WidgetBP)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Failed to load Widget Blueprint: %s"), *BlueprintPath));
	}

	if (!WidgetBP->WidgetTree)
	{
		return CreateErrorResponse(TEXT("Widget Blueprint has no WidgetTree"));
	}

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), true);
	Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);

	if (WidgetBP->WidgetTree->RootWidget)
	{
		Result->SetObjectField(TEXT("root_widget"), WidgetToJson(WidgetBP->WidgetTree->RootWidget, true));
	}
	else
	{
		Result->SetField(TEXT("root_widget"), MakeShared<FJsonValueNull>());
	}

	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleGetWidgetProperties(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintPath, WidgetName;
	if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
	{
		return CreateErrorResponse(TEXT("Missing 'blueprint_path' parameter"));
	}
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	UWidgetBlueprint* WidgetBP = LoadWidgetBlueprint(BlueprintPath);
	if (!WidgetBP)
	{
		return CreateErrorResponse(TEXT("Failed to load Widget Blueprint"));
	}

	UWidget* Widget = FindWidgetByName(WidgetBP, WidgetName);
	if (!Widget)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Widget not found: %s"), *WidgetName));
	}

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), true);
	Result->SetObjectField(TEXT("widget"), WidgetToJson(Widget, false));

	// Add type-specific properties
	if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
	{
		Result->SetStringField(TEXT("text"), TextBlock->GetText().ToString());
	}

	return Result;
}

// ============================================================================
// Widget Blueprint CREATE Operations
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleCreateWidgetBlueprint(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetName;
	if (!Params->TryGetStringField(TEXT("name"), AssetName))
	{
		return CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	FString PackagePath = TEXT("/Game/Widgets");
	Params->TryGetStringField(TEXT("path"), PackagePath);

	// Create package
	FString PackageName = PackagePath / AssetName;
	UPackage* Package = CreatePackage(*PackageName);
	if (!Package)
	{
		return CreateErrorResponse(TEXT("Failed to create package"));
	}

	// Use factory to create Widget Blueprint
	UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
	Factory->ParentClass = UUserWidget::StaticClass();

	UWidgetBlueprint* NewWidgetBP = Cast<UWidgetBlueprint>(
		Factory->FactoryCreateNew(
			UWidgetBlueprint::StaticClass(),
			Package,
			*AssetName,
			RF_Public | RF_Standalone,
			nullptr,
			GWarn
		)
	);

	if (NewWidgetBP)
	{
		// Notify asset registry
		FAssetRegistryModule::AssetCreated(NewWidgetBP);
		Package->MarkPackageDirty();

		// Return success with asset path
		TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
		Result->SetBoolField(TEXT("success"), true);
		Result->SetStringField(TEXT("asset_path"), PackageName + TEXT(".") + AssetName);
		Result->SetStringField(TEXT("asset_name"), AssetName);
		return Result;
	}

	return CreateErrorResponse(TEXT("Failed to create Widget Blueprint"));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleAddWidgetToBlueprint(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintPath, WidgetType, WidgetName, ParentWidgetName;

	if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
	{
		return CreateErrorResponse(TEXT("Missing 'blueprint_path' parameter"));
	}
	if (!Params->TryGetStringField(TEXT("widget_type"), WidgetType))
	{
		return CreateErrorResponse(TEXT("Missing 'widget_type' parameter"));
	}
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	Params->TryGetStringField(TEXT("parent_widget"), ParentWidgetName);

	UWidgetBlueprint* WidgetBP = LoadWidgetBlueprint(BlueprintPath);
	if (!WidgetBP)
	{
		return CreateErrorResponse(TEXT("Failed to load Widget Blueprint"));
	}

	UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
	if (!WidgetTree)
	{
		return CreateErrorResponse(TEXT("Widget Blueprint has no WidgetTree"));
	}

	// Determine widget class to create
	UClass* WidgetClass = nullptr;
	if (WidgetType == TEXT("Button")) WidgetClass = UButton::StaticClass();
	else if (WidgetType == TEXT("TextBlock")) WidgetClass = UTextBlock::StaticClass();
	else if (WidgetType == TEXT("Image")) WidgetClass = UImage::StaticClass();
	else if (WidgetType == TEXT("CanvasPanel")) WidgetClass = UCanvasPanel::StaticClass();
	else if (WidgetType == TEXT("VerticalBox")) WidgetClass = UVerticalBox::StaticClass();
	else if (WidgetType == TEXT("HorizontalBox")) WidgetClass = UHorizontalBox::StaticClass();
	else if (WidgetType == TEXT("Border")) WidgetClass = UBorder::StaticClass();
	else if (WidgetType == TEXT("Overlay")) WidgetClass = UOverlay::StaticClass();
	else if (WidgetType == TEXT("SizeBox")) WidgetClass = USizeBox::StaticClass();
	else if (WidgetType == TEXT("ScrollBox")) WidgetClass = UScrollBox::StaticClass();
	else if (WidgetType == TEXT("Spacer")) WidgetClass = USpacer::StaticClass();
	else
	{
		return CreateErrorResponse(FString::Printf(TEXT("Unknown widget type: %s"), *WidgetType));
	}

	// Create the widget using WidgetTree
	UWidget* NewWidget = WidgetTree->ConstructWidget<UWidget>(WidgetClass, *WidgetName);
	if (!NewWidget)
	{
		return CreateErrorResponse(TEXT("Failed to construct widget"));
	}

	// Find parent or use root
	UPanelWidget* ParentPanel = nullptr;
	if (!ParentWidgetName.IsEmpty())
	{
		UWidget* ParentWidget = FindWidgetByName(WidgetBP, ParentWidgetName);
		ParentPanel = Cast<UPanelWidget>(ParentWidget);
		if (!ParentPanel)
		{
			return CreateErrorResponse(TEXT("Parent widget is not a panel or not found"));
		}
	}
	else
	{
		// If no parent specified, set as root or add to root panel
		if (!WidgetTree->RootWidget)
		{
			WidgetTree->RootWidget = NewWidget;
		}
		else
		{
			ParentPanel = Cast<UPanelWidget>(WidgetTree->RootWidget);
			if (!ParentPanel)
			{
				return CreateErrorResponse(TEXT("Root widget is not a panel; cannot add children without specifying parent"));
			}
		}
	}

	// Add to parent panel if applicable
	if (ParentPanel)
	{
		UPanelSlot* Slot = ParentPanel->AddChild(NewWidget);
		if (!Slot)
		{
			return CreateErrorResponse(TEXT("Failed to add widget to parent panel"));
		}
	}

	// Mark dirty and compile
	WidgetBP->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), true);
	Result->SetStringField(TEXT("widget_name"), WidgetName);
	Result->SetStringField(TEXT("widget_type"), WidgetType);
	return Result;
}

// ============================================================================
// Widget Blueprint UPDATE Operations
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleSetWidgetProperties(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintPath, WidgetName;
	if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
	{
		return CreateErrorResponse(TEXT("Missing 'blueprint_path' parameter"));
	}
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	UWidgetBlueprint* WidgetBP = LoadWidgetBlueprint(BlueprintPath);
	if (!WidgetBP)
	{
		return CreateErrorResponse(TEXT("Failed to load Widget Blueprint"));
	}

	UWidget* Widget = FindWidgetByName(WidgetBP, WidgetName);
	if (!Widget)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Widget not found: %s"), *WidgetName));
	}

	// Handle TextBlock text
	if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
	{
		FString TextValue;
		if (Params->TryGetStringField(TEXT("text"), TextValue))
		{
			TextBlock->SetText(FText::FromString(TextValue));
		}

		// Font size
		int32 FontSize;
		if (Params->TryGetNumberField(TEXT("font_size"), FontSize))
		{
			FSlateFontInfo FontInfo = TextBlock->Font;
			FontInfo.Size = FontSize;
			TextBlock->SetFont(FontInfo);
		}
	}

	// Handle visibility
	FString VisibilityStr;
	if (Params->TryGetStringField(TEXT("visibility"), VisibilityStr))
	{
		if (VisibilityStr == TEXT("Visible")) Widget->SetVisibility(ESlateVisibility::Visible);
		else if (VisibilityStr == TEXT("Hidden")) Widget->SetVisibility(ESlateVisibility::Hidden);
		else if (VisibilityStr == TEXT("Collapsed")) Widget->SetVisibility(ESlateVisibility::Collapsed);
		else if (VisibilityStr == TEXT("HitTestInvisible")) Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
		else if (VisibilityStr == TEXT("SelfHitTestInvisible")) Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	// Handle CanvasPanel slot properties (position, size, anchors)
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
	{
		const TArray<TSharedPtr<FJsonValue>>* PositionArray;
		if (Params->TryGetArrayField(TEXT("position"), PositionArray) && PositionArray->Num() >= 2)
		{
			FVector2D Position((*PositionArray)[0]->AsNumber(), (*PositionArray)[1]->AsNumber());
			CanvasSlot->SetPosition(Position);
		}

		const TArray<TSharedPtr<FJsonValue>>* SizeArray;
		if (Params->TryGetArrayField(TEXT("size"), SizeArray) && SizeArray->Num() >= 2)
		{
			FVector2D Size((*SizeArray)[0]->AsNumber(), (*SizeArray)[1]->AsNumber());
			CanvasSlot->SetSize(Size);
		}

		const TArray<TSharedPtr<FJsonValue>>* AnchorsArray;
		if (Params->TryGetArrayField(TEXT("anchors"), AnchorsArray) && AnchorsArray->Num() >= 4)
		{
			FAnchors Anchors;
			Anchors.Minimum.X = (*AnchorsArray)[0]->AsNumber();
			Anchors.Minimum.Y = (*AnchorsArray)[1]->AsNumber();
			Anchors.Maximum.X = (*AnchorsArray)[2]->AsNumber();
			Anchors.Maximum.Y = (*AnchorsArray)[3]->AsNumber();
			CanvasSlot->SetAnchors(Anchors);
		}
	}

	WidgetBP->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), true);
	Result->SetStringField(TEXT("widget_name"), WidgetName);
	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleRenameWidget(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintPath, WidgetName, NewName;
	if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
	{
		return CreateErrorResponse(TEXT("Missing 'blueprint_path' parameter"));
	}
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}
	if (!Params->TryGetStringField(TEXT("new_name"), NewName))
	{
		return CreateErrorResponse(TEXT("Missing 'new_name' parameter"));
	}

	UWidgetBlueprint* WidgetBP = LoadWidgetBlueprint(BlueprintPath);
	if (!WidgetBP)
	{
		return CreateErrorResponse(TEXT("Failed to load Widget Blueprint"));
	}

	UWidget* Widget = FindWidgetByName(WidgetBP, WidgetName);
	if (!Widget)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Widget not found: %s"), *WidgetName));
	}

	// Rename the widget
	Widget->Rename(*NewName, WidgetBP->WidgetTree);

	WidgetBP->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), true);
	Result->SetStringField(TEXT("old_name"), WidgetName);
	Result->SetStringField(TEXT("new_name"), NewName);
	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleReparentWidget(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintPath, WidgetName, NewParentName;
	if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
	{
		return CreateErrorResponse(TEXT("Missing 'blueprint_path' parameter"));
	}
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}
	if (!Params->TryGetStringField(TEXT("new_parent"), NewParentName))
	{
		return CreateErrorResponse(TEXT("Missing 'new_parent' parameter"));
	}

	UWidgetBlueprint* WidgetBP = LoadWidgetBlueprint(BlueprintPath);
	if (!WidgetBP)
	{
		return CreateErrorResponse(TEXT("Failed to load Widget Blueprint"));
	}

	UWidget* Widget = FindWidgetByName(WidgetBP, WidgetName);
	UWidget* NewParent = FindWidgetByName(WidgetBP, NewParentName);

	if (!Widget)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Widget not found: %s"), *WidgetName));
	}

	UPanelWidget* NewParentPanel = Cast<UPanelWidget>(NewParent);
	if (!NewParentPanel)
	{
		return CreateErrorResponse(TEXT("New parent must be a panel widget"));
	}

	// Remove from current parent
	if (Widget->GetParent())
	{
		Widget->RemoveFromParent();
	}

	// Add to new parent
	UPanelSlot* NewSlot = NewParentPanel->AddChild(Widget);
	if (!NewSlot)
	{
		return CreateErrorResponse(TEXT("Failed to add widget to new parent"));
	}

	WidgetBP->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), true);
	Result->SetStringField(TEXT("widget_name"), WidgetName);
	Result->SetStringField(TEXT("new_parent"), NewParentName);
	return Result;
}

// ============================================================================
// Widget Blueprint DELETE Operations
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleRemoveWidgetFromBlueprint(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintPath, WidgetName;
	if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
	{
		return CreateErrorResponse(TEXT("Missing 'blueprint_path' parameter"));
	}
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	UWidgetBlueprint* WidgetBP = LoadWidgetBlueprint(BlueprintPath);
	if (!WidgetBP || !WidgetBP->WidgetTree)
	{
		return CreateErrorResponse(TEXT("Failed to load Widget Blueprint"));
	}

	UWidget* Widget = FindWidgetByName(WidgetBP, WidgetName);
	if (!Widget)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Widget not found: %s"), *WidgetName));
	}

	// Remove widget from tree
	WidgetBP->WidgetTree->RemoveWidget(Widget);

	WidgetBP->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), true);
	Result->SetStringField(TEXT("removed_widget"), WidgetName);
	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleDeleteWidgetBlueprint(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath;
	if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
	{
		return CreateErrorResponse(TEXT("Missing 'asset_path' parameter"));
	}

	UWidgetBlueprint* WidgetBP = LoadWidgetBlueprint(AssetPath);
	if (!WidgetBP)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Failed to load Widget Blueprint: %s"), *AssetPath));
	}

	// Delete the asset
	TArray<UObject*> ObjectsToDelete;
	ObjectsToDelete.Add(WidgetBP);

	int32 DeletedCount = ObjectTools::DeleteObjects(ObjectsToDelete, true);

	if (DeletedCount > 0)
	{
		TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
		Result->SetBoolField(TEXT("success"), true);
		Result->SetStringField(TEXT("deleted_asset"), AssetPath);
		return Result;
	}

	return CreateErrorResponse(TEXT("Failed to delete Widget Blueprint"));
}

// ============================================================================
// Widget Blueprint RUNTIME Operations
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleShowWidget(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintPath;
	if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
	{
		return CreateErrorResponse(TEXT("Missing 'blueprint_path' parameter"));
	}

	int32 ZOrder = 0;
	Params->TryGetNumberField(TEXT("z_order"), ZOrder);

	// Load widget blueprint
	UWidgetBlueprint* WidgetBP = LoadWidgetBlueprint(BlueprintPath);
	if (!WidgetBP)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Failed to load Widget Blueprint: %s"), *BlueprintPath));
	}

	if (!WidgetBP->GeneratedClass)
	{
		return CreateErrorResponse(TEXT("Widget Blueprint has no GeneratedClass - compile it first"));
	}

	// Get the PIE world if playing, otherwise editor world
	UWorld* World = nullptr;
	if (GEditor && GEditor->PlayWorld)
	{
		World = GEditor->PlayWorld;
	}
	else
	{
		return CreateErrorResponse(TEXT("Cannot show widget - no Play session active. Press Play first."));
	}

	// Get player controller
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return CreateErrorResponse(TEXT("No player controller found in world"));
	}

	// Create widget and add to viewport
	// Cast GeneratedClass to the correct type for CreateWidget
	TSubclassOf<UUserWidget> WidgetClass = *WidgetBP->GeneratedClass;
	UUserWidget* Widget = CreateWidget<UUserWidget>(PC, WidgetClass);
	if (!Widget)
	{
		return CreateErrorResponse(TEXT("Failed to create widget instance"));
	}

	Widget->AddToViewport(ZOrder);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), true);
	Result->SetStringField(TEXT("message"), TEXT("Widget displayed on viewport"));
	Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
	Result->SetNumberField(TEXT("z_order"), ZOrder);
	return Result;
}

// ============================================================================
// Actor Property Commands
// ============================================================================

AActor* FEpicUnrealMCPEditorCommands::FindActorByName(const FString& ActorName)
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World) return nullptr;

	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

	for (AActor* Actor : AllActors)
	{
		if (Actor && Actor->GetName() == ActorName)
		{
			return Actor;
		}
	}
	return nullptr;
}

FString FEpicUnrealMCPEditorCommands::GetPropertyTypeName(UProperty* Property)
{
	if (!Property) return TEXT("Unknown");

	if (Cast<UBoolProperty>(Property)) return TEXT("Bool");
	if (UByteProperty* ByteProp = Cast<UByteProperty>(Property))
	{
		return ByteProp->Enum ? FString::Printf(TEXT("Enum:%s"), *ByteProp->Enum->GetName()) : TEXT("Byte");
	}
	if (Cast<UIntProperty>(Property)) return TEXT("Int");
	if (Cast<UInt64Property>(Property)) return TEXT("Int64");
	if (Cast<UFloatProperty>(Property)) return TEXT("Float");
	if (Cast<UDoubleProperty>(Property)) return TEXT("Double");
	if (Cast<UStrProperty>(Property)) return TEXT("String");
	if (Cast<UNameProperty>(Property)) return TEXT("Name");
	if (Cast<UTextProperty>(Property)) return TEXT("Text");
	if (UStructProperty* StructProp = Cast<UStructProperty>(Property))
	{
		return FString::Printf(TEXT("Struct:%s"), *StructProp->Struct->GetName());
	}
	if (UEnumProperty* EnumProp = Cast<UEnumProperty>(Property))
	{
		return FString::Printf(TEXT("Enum:%s"), *EnumProp->GetEnum()->GetName());
	}
	if (UObjectProperty* ObjProp = Cast<UObjectProperty>(Property))
	{
		return FString::Printf(TEXT("Object:%s"), *ObjProp->PropertyClass->GetName());
	}
	if (UClassProperty* ClassProp = Cast<UClassProperty>(Property))
	{
		return FString::Printf(TEXT("Class:%s"), *ClassProp->MetaClass->GetName());
	}
	if (UArrayProperty* ArrayProp = Cast<UArrayProperty>(Property))
	{
		return FString::Printf(TEXT("Array<%s>"), *GetPropertyTypeName(ArrayProp->Inner));
	}
	if (UMapProperty* MapProp = Cast<UMapProperty>(Property))
	{
		return FString::Printf(TEXT("Map<%s, %s>"),
			*GetPropertyTypeName(MapProp->KeyProp),
			*GetPropertyTypeName(MapProp->ValueProp));
	}
	if (USetProperty* SetProp = Cast<USetProperty>(Property))
	{
		return FString::Printf(TEXT("Set<%s>"), *GetPropertyTypeName(SetProp->ElementProp));
	}

	return Property->GetClass()->GetName();
}

TSharedPtr<FJsonValue> FEpicUnrealMCPEditorCommands::PropertyToJsonValue(UProperty* Property, const void* ValuePtr)
{
	if (!Property || !ValuePtr)
	{
		return MakeShared<FJsonValueNull>();
	}

	// Boolean
	if (UBoolProperty* BoolProp = Cast<UBoolProperty>(Property))
	{
		return MakeShared<FJsonValueBoolean>(BoolProp->GetPropertyValue(ValuePtr));
	}

	// Integer types
	if (UByteProperty* ByteProp = Cast<UByteProperty>(Property))
	{
		if (ByteProp->Enum)
		{
			uint8 Value = ByteProp->GetPropertyValue(ValuePtr);
			FString EnumName = ByteProp->Enum->GetNameStringByIndex(Value);
			return MakeShared<FJsonValueString>(EnumName);
		}
		return MakeShared<FJsonValueNumber>(ByteProp->GetPropertyValue(ValuePtr));
	}
	if (UIntProperty* IntProp = Cast<UIntProperty>(Property))
	{
		return MakeShared<FJsonValueNumber>(IntProp->GetPropertyValue(ValuePtr));
	}
	if (UInt64Property* Int64Prop = Cast<UInt64Property>(Property))
	{
		return MakeShared<FJsonValueNumber>((double)Int64Prop->GetPropertyValue(ValuePtr));
	}

	// Float types
	if (UFloatProperty* FloatProp = Cast<UFloatProperty>(Property))
	{
		return MakeShared<FJsonValueNumber>(FloatProp->GetPropertyValue(ValuePtr));
	}
	if (UDoubleProperty* DoubleProp = Cast<UDoubleProperty>(Property))
	{
		return MakeShared<FJsonValueNumber>(DoubleProp->GetPropertyValue(ValuePtr));
	}

	// String types
	if (UStrProperty* StrProp = Cast<UStrProperty>(Property))
	{
		return MakeShared<FJsonValueString>(StrProp->GetPropertyValue(ValuePtr));
	}
	if (UNameProperty* NameProp = Cast<UNameProperty>(Property))
	{
		return MakeShared<FJsonValueString>(NameProp->GetPropertyValue(ValuePtr).ToString());
	}
	if (UTextProperty* TextProp = Cast<UTextProperty>(Property))
	{
		return MakeShared<FJsonValueString>(TextProp->GetPropertyValue(ValuePtr).ToString());
	}

	// Struct types (Vector, Rotator, Transform, Color, LinearColor)
	if (UStructProperty* StructProp = Cast<UStructProperty>(Property))
	{
		UScriptStruct* Struct = StructProp->Struct;

		if (Struct == TBaseStructure<FVector>::Get())
		{
			const FVector* Vec = static_cast<const FVector*>(ValuePtr);
			TArray<TSharedPtr<FJsonValue>> Arr;
			Arr.Add(MakeShared<FJsonValueNumber>(Vec->X));
			Arr.Add(MakeShared<FJsonValueNumber>(Vec->Y));
			Arr.Add(MakeShared<FJsonValueNumber>(Vec->Z));
			return MakeShared<FJsonValueArray>(Arr);
		}
		if (Struct == TBaseStructure<FRotator>::Get())
		{
			const FRotator* Rot = static_cast<const FRotator*>(ValuePtr);
			TArray<TSharedPtr<FJsonValue>> Arr;
			Arr.Add(MakeShared<FJsonValueNumber>(Rot->Pitch));
			Arr.Add(MakeShared<FJsonValueNumber>(Rot->Yaw));
			Arr.Add(MakeShared<FJsonValueNumber>(Rot->Roll));
			return MakeShared<FJsonValueArray>(Arr);
		}
		if (Struct == TBaseStructure<FTransform>::Get())
		{
			const FTransform* Trans = static_cast<const FTransform*>(ValuePtr);
			TSharedPtr<FJsonObject> TransObj = MakeShared<FJsonObject>();

			TArray<TSharedPtr<FJsonValue>> LocArr;
			LocArr.Add(MakeShared<FJsonValueNumber>(Trans->GetLocation().X));
			LocArr.Add(MakeShared<FJsonValueNumber>(Trans->GetLocation().Y));
			LocArr.Add(MakeShared<FJsonValueNumber>(Trans->GetLocation().Z));
			TransObj->SetArrayField(TEXT("location"), LocArr);

			FRotator Rot = Trans->GetRotation().Rotator();
			TArray<TSharedPtr<FJsonValue>> RotArr;
			RotArr.Add(MakeShared<FJsonValueNumber>(Rot.Pitch));
			RotArr.Add(MakeShared<FJsonValueNumber>(Rot.Yaw));
			RotArr.Add(MakeShared<FJsonValueNumber>(Rot.Roll));
			TransObj->SetArrayField(TEXT("rotation"), RotArr);

			TArray<TSharedPtr<FJsonValue>> ScaleArr;
			ScaleArr.Add(MakeShared<FJsonValueNumber>(Trans->GetScale3D().X));
			ScaleArr.Add(MakeShared<FJsonValueNumber>(Trans->GetScale3D().Y));
			ScaleArr.Add(MakeShared<FJsonValueNumber>(Trans->GetScale3D().Z));
			TransObj->SetArrayField(TEXT("scale"), ScaleArr);

			return MakeShared<FJsonValueObject>(TransObj);
		}
		if (Struct == TBaseStructure<FLinearColor>::Get())
		{
			const FLinearColor* Color = static_cast<const FLinearColor*>(ValuePtr);
			TArray<TSharedPtr<FJsonValue>> Arr;
			Arr.Add(MakeShared<FJsonValueNumber>(Color->R));
			Arr.Add(MakeShared<FJsonValueNumber>(Color->G));
			Arr.Add(MakeShared<FJsonValueNumber>(Color->B));
			Arr.Add(MakeShared<FJsonValueNumber>(Color->A));
			return MakeShared<FJsonValueArray>(Arr);
		}
		if (Struct == TBaseStructure<FColor>::Get())
		{
			const FColor* Color = static_cast<const FColor*>(ValuePtr);
			TArray<TSharedPtr<FJsonValue>> Arr;
			Arr.Add(MakeShared<FJsonValueNumber>(Color->R));
			Arr.Add(MakeShared<FJsonValueNumber>(Color->G));
			Arr.Add(MakeShared<FJsonValueNumber>(Color->B));
			Arr.Add(MakeShared<FJsonValueNumber>(Color->A));
			return MakeShared<FJsonValueArray>(Arr);
		}
		if (Struct == TBaseStructure<FVector2D>::Get())
		{
			const FVector2D* Vec = static_cast<const FVector2D*>(ValuePtr);
			TArray<TSharedPtr<FJsonValue>> Arr;
			Arr.Add(MakeShared<FJsonValueNumber>(Vec->X));
			Arr.Add(MakeShared<FJsonValueNumber>(Vec->Y));
			return MakeShared<FJsonValueArray>(Arr);
		}

		// Generic struct - iterate fields as JSON object
		TSharedPtr<FJsonObject> StructJson = MakeShared<FJsonObject>();
		for (TFieldIterator<UProperty> PropIt(Struct); PropIt; ++PropIt)
		{
			UProperty* FieldProp = *PropIt;
			const void* FieldPtr = FieldProp->ContainerPtrToValuePtr<void>(ValuePtr);
			TSharedPtr<FJsonValue> FieldJson = PropertyToJsonValue(FieldProp, FieldPtr);
			StructJson->SetField(FieldProp->GetName(), FieldJson);
		}
		return MakeShared<FJsonValueObject>(StructJson);
	}

	// Enum
	if (UEnumProperty* EnumProp = Cast<UEnumProperty>(Property))
	{
		UEnum* Enum = EnumProp->GetEnum();
		UNumericProperty* UnderlyingProp = EnumProp->GetUnderlyingProperty();
		int64 Value = UnderlyingProp->GetSignedIntPropertyValue(ValuePtr);
		FString EnumName = Enum->GetNameStringByValue(Value);
		return MakeShared<FJsonValueString>(EnumName);
	}

	// Object reference
	if (UObjectProperty* ObjProp = Cast<UObjectProperty>(Property))
	{
		UObject* Obj = ObjProp->GetObjectPropertyValue(ValuePtr);
		if (Obj)
		{
			return MakeShared<FJsonValueString>(Obj->GetPathName());
		}
		return MakeShared<FJsonValueNull>();
	}

	// Class reference
	if (UClassProperty* ClassProp = Cast<UClassProperty>(Property))
	{
		UClass* Class = Cast<UClass>(ClassProp->GetObjectPropertyValue(ValuePtr));
		if (Class)
		{
			return MakeShared<FJsonValueString>(Class->GetPathName());
		}
		return MakeShared<FJsonValueNull>();
	}

	// Array property
	if (UArrayProperty* ArrayProp = Cast<UArrayProperty>(Property))
	{
		FScriptArrayHelper ArrayHelper(ArrayProp, ValuePtr);
		TArray<TSharedPtr<FJsonValue>> JsonArray;

		for (int32 i = 0; i < ArrayHelper.Num(); ++i)
		{
			void* ElementPtr = ArrayHelper.GetRawPtr(i);
			TSharedPtr<FJsonValue> ElementJson = PropertyToJsonValue(ArrayProp->Inner, ElementPtr);
			JsonArray.Add(ElementJson);
		}

		return MakeShared<FJsonValueArray>(JsonArray);
	}

	// Map property
	if (UMapProperty* MapProp = Cast<UMapProperty>(Property))
	{
		FScriptMapHelper MapHelper(MapProp, ValuePtr);
		TSharedPtr<FJsonObject> JsonMap = MakeShared<FJsonObject>();

		for (int32 i = 0; i < MapHelper.Num(); ++i)
		{
			if (MapHelper.IsValidIndex(i))
			{
				// Get key as string (for JSON object key)
				void* KeyPtr = MapHelper.GetKeyPtr(i);
				FString KeyStr;
				MapProp->KeyProp->ExportTextItem(KeyStr, KeyPtr, nullptr, nullptr, PPF_None);

				// Get value as JSON
				void* ValuePtrInner = MapHelper.GetValuePtr(i);
				TSharedPtr<FJsonValue> ValueJson = PropertyToJsonValue(MapProp->ValueProp, ValuePtrInner);

				JsonMap->SetField(KeyStr, ValueJson);
			}
		}

		return MakeShared<FJsonValueObject>(JsonMap);
	}

	// Set property
	if (USetProperty* SetProp = Cast<USetProperty>(Property))
	{
		FScriptSetHelper SetHelper(SetProp, ValuePtr);
		TArray<TSharedPtr<FJsonValue>> JsonArray;

		for (int32 i = 0; i < SetHelper.Num(); ++i)
		{
			if (SetHelper.IsValidIndex(i))
			{
				void* ElementPtr = SetHelper.GetElementPtr(i);
				TSharedPtr<FJsonValue> ElementJson = PropertyToJsonValue(SetProp->ElementProp, ElementPtr);
				JsonArray.Add(ElementJson);
			}
		}

		return MakeShared<FJsonValueArray>(JsonArray);
	}

	// Fallback: export as text
	FString ExportedText;
	Property->ExportTextItem(ExportedText, ValuePtr, nullptr, nullptr, PPF_None);
	return MakeShared<FJsonValueString>(ExportedText);
}

bool FEpicUnrealMCPEditorCommands::JsonValueToProperty(const TSharedPtr<FJsonValue>& JsonValue, UProperty* Property, void* ValuePtr)
{
	if (!JsonValue.IsValid() || !Property || !ValuePtr)
	{
		return false;
	}

	// Boolean
	if (UBoolProperty* BoolProp = Cast<UBoolProperty>(Property))
	{
		bool BoolVal;
		if (JsonValue->TryGetBool(BoolVal))
		{
			BoolProp->SetPropertyValue(ValuePtr, BoolVal);
			return true;
		}
		return false;
	}

	// Numeric types
	if (UByteProperty* ByteProp = Cast<UByteProperty>(Property))
	{
		if (!ByteProp->Enum)  // Regular byte, not enum
		{
			double NumVal;
			if (JsonValue->TryGetNumber(NumVal))
			{
				ByteProp->SetPropertyValue(ValuePtr, static_cast<uint8>(NumVal));
				return true;
			}
		}
		else  // Byte enum
		{
			FString EnumStr;
			if (JsonValue->TryGetString(EnumStr))
			{
				int32 EnumValue = ByteProp->Enum->GetIndexByNameString(EnumStr);
				if (EnumValue != INDEX_NONE)
				{
					ByteProp->SetPropertyValue(ValuePtr, static_cast<uint8>(EnumValue));
					return true;
				}
			}
		}
		return false;
	}
	if (UIntProperty* IntProp = Cast<UIntProperty>(Property))
	{
		double NumVal;
		if (JsonValue->TryGetNumber(NumVal))
		{
			IntProp->SetPropertyValue(ValuePtr, static_cast<int32>(NumVal));
			return true;
		}
		return false;
	}
	if (UInt64Property* Int64Prop = Cast<UInt64Property>(Property))
	{
		double NumVal;
		if (JsonValue->TryGetNumber(NumVal))
		{
			Int64Prop->SetPropertyValue(ValuePtr, static_cast<int64>(NumVal));
			return true;
		}
		return false;
	}
	if (UFloatProperty* FloatProp = Cast<UFloatProperty>(Property))
	{
		double NumVal;
		if (JsonValue->TryGetNumber(NumVal))
		{
			FloatProp->SetPropertyValue(ValuePtr, static_cast<float>(NumVal));
			return true;
		}
		return false;
	}
	if (UDoubleProperty* DoubleProp = Cast<UDoubleProperty>(Property))
	{
		double NumVal;
		if (JsonValue->TryGetNumber(NumVal))
		{
			DoubleProp->SetPropertyValue(ValuePtr, NumVal);
			return true;
		}
		return false;
	}

	// String types
	if (UStrProperty* StrProp = Cast<UStrProperty>(Property))
	{
		FString StrVal;
		if (JsonValue->TryGetString(StrVal))
		{
			StrProp->SetPropertyValue(ValuePtr, StrVal);
			return true;
		}
		return false;
	}
	if (UNameProperty* NameProp = Cast<UNameProperty>(Property))
	{
		FString StrVal;
		if (JsonValue->TryGetString(StrVal))
		{
			NameProp->SetPropertyValue(ValuePtr, FName(*StrVal));
			return true;
		}
		return false;
	}
	if (UTextProperty* TextProp = Cast<UTextProperty>(Property))
	{
		FString StrVal;
		if (JsonValue->TryGetString(StrVal))
		{
			TextProp->SetPropertyValue(ValuePtr, FText::FromString(StrVal));
			return true;
		}
		return false;
	}

	// Struct types
	if (UStructProperty* StructProp = Cast<UStructProperty>(Property))
	{
		UScriptStruct* Struct = StructProp->Struct;

		if (Struct == TBaseStructure<FVector>::Get())
		{
			const TArray<TSharedPtr<FJsonValue>>* Arr;
			if (JsonValue->TryGetArray(Arr) && Arr->Num() >= 3)
			{
				FVector* Vec = static_cast<FVector*>(ValuePtr);
				Vec->X = (*Arr)[0]->AsNumber();
				Vec->Y = (*Arr)[1]->AsNumber();
				Vec->Z = (*Arr)[2]->AsNumber();
				return true;
			}
			return false;
		}
		if (Struct == TBaseStructure<FRotator>::Get())
		{
			const TArray<TSharedPtr<FJsonValue>>* Arr;
			if (JsonValue->TryGetArray(Arr) && Arr->Num() >= 3)
			{
				FRotator* Rot = static_cast<FRotator*>(ValuePtr);
				Rot->Pitch = (*Arr)[0]->AsNumber();
				Rot->Yaw = (*Arr)[1]->AsNumber();
				Rot->Roll = (*Arr)[2]->AsNumber();
				return true;
			}
			return false;
		}
		if (Struct == TBaseStructure<FTransform>::Get())
		{
			const TSharedPtr<FJsonObject>* TransObj;
			if (JsonValue->TryGetObject(TransObj))
			{
				FTransform* Trans = static_cast<FTransform*>(ValuePtr);

				const TArray<TSharedPtr<FJsonValue>>* LocArr;
				if ((*TransObj)->TryGetArrayField(TEXT("location"), LocArr) && LocArr->Num() >= 3)
				{
					Trans->SetLocation(FVector(
						(*LocArr)[0]->AsNumber(),
						(*LocArr)[1]->AsNumber(),
						(*LocArr)[2]->AsNumber()
					));
				}

				const TArray<TSharedPtr<FJsonValue>>* RotArr;
				if ((*TransObj)->TryGetArrayField(TEXT("rotation"), RotArr) && RotArr->Num() >= 3)
				{
					Trans->SetRotation(FQuat(FRotator(
						(*RotArr)[0]->AsNumber(),
						(*RotArr)[1]->AsNumber(),
						(*RotArr)[2]->AsNumber()
					)));
				}

				const TArray<TSharedPtr<FJsonValue>>* ScaleArr;
				if ((*TransObj)->TryGetArrayField(TEXT("scale"), ScaleArr) && ScaleArr->Num() >= 3)
				{
					Trans->SetScale3D(FVector(
						(*ScaleArr)[0]->AsNumber(),
						(*ScaleArr)[1]->AsNumber(),
						(*ScaleArr)[2]->AsNumber()
					));
				}
				return true;
			}
			return false;
		}
		if (Struct == TBaseStructure<FLinearColor>::Get())
		{
			const TArray<TSharedPtr<FJsonValue>>* Arr;
			if (JsonValue->TryGetArray(Arr) && Arr->Num() >= 3)
			{
				FLinearColor* Color = static_cast<FLinearColor*>(ValuePtr);
				Color->R = (*Arr)[0]->AsNumber();
				Color->G = (*Arr)[1]->AsNumber();
				Color->B = (*Arr)[2]->AsNumber();
				Color->A = Arr->Num() >= 4 ? (*Arr)[3]->AsNumber() : 1.0f;
				return true;
			}
			return false;
		}
		if (Struct == TBaseStructure<FColor>::Get())
		{
			const TArray<TSharedPtr<FJsonValue>>* Arr;
			if (JsonValue->TryGetArray(Arr) && Arr->Num() >= 3)
			{
				FColor* Color = static_cast<FColor*>(ValuePtr);
				Color->R = static_cast<uint8>((*Arr)[0]->AsNumber());
				Color->G = static_cast<uint8>((*Arr)[1]->AsNumber());
				Color->B = static_cast<uint8>((*Arr)[2]->AsNumber());
				Color->A = Arr->Num() >= 4 ? static_cast<uint8>((*Arr)[3]->AsNumber()) : 255;
				return true;
			}
			return false;
		}
		if (Struct == TBaseStructure<FVector2D>::Get())
		{
			const TArray<TSharedPtr<FJsonValue>>* Arr;
			if (JsonValue->TryGetArray(Arr) && Arr->Num() >= 2)
			{
				FVector2D* Vec = static_cast<FVector2D*>(ValuePtr);
				Vec->X = (*Arr)[0]->AsNumber();
				Vec->Y = (*Arr)[1]->AsNumber();
				return true;
			}
			return false;
		}

		// Generic struct - try JSON object first, then string fallback
		const TSharedPtr<FJsonObject>* JsonObj;
		if (JsonValue->TryGetObject(JsonObj))
		{
			for (TFieldIterator<UProperty> PropIt(Struct); PropIt; ++PropIt)
			{
				UProperty* FieldProp = *PropIt;
				TSharedPtr<FJsonValue> FieldJson = (*JsonObj)->TryGetField(FieldProp->GetName());
				if (FieldJson.IsValid())
				{
					void* FieldPtr = FieldProp->ContainerPtrToValuePtr<void>(ValuePtr);
					JsonValueToProperty(FieldJson, FieldProp, FieldPtr);
				}
			}
			return true;
		}
		// String fallback for backwards compatibility
		FString StrVal;
		if (JsonValue->TryGetString(StrVal))
		{
			const TCHAR* Buffer = *StrVal;
			Property->ImportText(Buffer, ValuePtr, PPF_None, nullptr);
			return true;
		}
		return false;
	}

	// Enum property
	if (UEnumProperty* EnumProp = Cast<UEnumProperty>(Property))
	{
		FString EnumStr;
		if (JsonValue->TryGetString(EnumStr))
		{
			UEnum* Enum = EnumProp->GetEnum();
			int64 EnumValue = Enum->GetValueByNameString(EnumStr);
			if (EnumValue != INDEX_NONE)
			{
				EnumProp->GetUnderlyingProperty()->SetIntPropertyValue(ValuePtr, EnumValue);
				return true;
			}
		}
		return false;
	}

	// Object reference (by path)
	if (UObjectProperty* ObjProp = Cast<UObjectProperty>(Property))
	{
		FString PathStr;
		if (JsonValue->TryGetString(PathStr))
		{
			UObject* LoadedObj = StaticLoadObject(ObjProp->PropertyClass, nullptr, *PathStr);
			ObjProp->SetObjectPropertyValue(ValuePtr, LoadedObj);
			return true;
		}
		return false;
	}

	// Array property
	if (UArrayProperty* ArrayProp = Cast<UArrayProperty>(Property))
	{
		const TArray<TSharedPtr<FJsonValue>>* JsonArray;
		if (JsonValue->TryGetArray(JsonArray))
		{
			FScriptArrayHelper ArrayHelper(ArrayProp, ValuePtr);
			ArrayHelper.Resize(JsonArray->Num());

			for (int32 i = 0; i < JsonArray->Num(); ++i)
			{
				void* ElementPtr = ArrayHelper.GetRawPtr(i);
				if (!JsonValueToProperty((*JsonArray)[i], ArrayProp->Inner, ElementPtr))
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}

	// Map property
	if (UMapProperty* MapProp = Cast<UMapProperty>(Property))
	{
		const TSharedPtr<FJsonObject>* JsonObj;
		if (JsonValue->TryGetObject(JsonObj))
		{
			FScriptMapHelper MapHelper(MapProp, ValuePtr);
			MapHelper.EmptyValues();

			for (const auto& Pair : (*JsonObj)->Values)
			{
				// Add new entry
				int32 Index = MapHelper.AddDefaultValue_Invalid_NeedsRehash();

				// Set key from string
				void* KeyPtr = MapHelper.GetKeyPtr(Index);
				const TCHAR* KeyBuffer = *Pair.Key;
				MapProp->KeyProp->ImportText(KeyBuffer, KeyPtr, PPF_None, nullptr);

				// Set value
				void* ValuePtrInner = MapHelper.GetValuePtr(Index);
				if (!JsonValueToProperty(Pair.Value, MapProp->ValueProp, ValuePtrInner))
				{
					return false;
				}
			}

			MapHelper.Rehash();
			return true;
		}
		return false;
	}

	// Set property
	if (USetProperty* SetProp = Cast<USetProperty>(Property))
	{
		const TArray<TSharedPtr<FJsonValue>>* JsonArray;
		if (JsonValue->TryGetArray(JsonArray))
		{
			FScriptSetHelper SetHelper(SetProp, ValuePtr);
			SetHelper.EmptyElements();

			for (const TSharedPtr<FJsonValue>& ElementJson : *JsonArray)
			{
				int32 Index = SetHelper.AddDefaultValue_Invalid_NeedsRehash();
				void* ElementPtr = SetHelper.GetElementPtr(Index);
				if (!JsonValueToProperty(ElementJson, SetProp->ElementProp, ElementPtr))
				{
					return false;
				}
			}

			SetHelper.Rehash();
			return true;
		}
		return false;
	}

	return false;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleGetActorProperty(const TSharedPtr<FJsonObject>& Params)
{
	FString ActorName;
	if (!Params->TryGetStringField(TEXT("name"), ActorName))
	{
		return CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	FString PropertyName;
	if (!Params->TryGetStringField(TEXT("property"), PropertyName))
	{
		return CreateErrorResponse(TEXT("Missing 'property' parameter"));
	}

	AActor* Actor = FindActorByName(ActorName);
	if (!Actor)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
	}

	// Find the property on the actor's class
	UProperty* Property = Actor->GetClass()->FindPropertyByName(*PropertyName);
	if (!Property)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Property not found: %s"), *PropertyName));
	}

	// Get the value pointer
	const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Actor);
	if (!ValuePtr)
	{
		return CreateErrorResponse(TEXT("Failed to get property value pointer"));
	}

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), true);
	Result->SetStringField(TEXT("actor"), ActorName);
	Result->SetStringField(TEXT("property"), PropertyName);
	Result->SetStringField(TEXT("type"), GetPropertyTypeName(Property));
	Result->SetField(TEXT("value"), PropertyToJsonValue(Property, ValuePtr));

	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleSetActorProperty(const TSharedPtr<FJsonObject>& Params)
{
	FString ActorName;
	if (!Params->TryGetStringField(TEXT("name"), ActorName))
	{
		return CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	FString PropertyName;
	if (!Params->TryGetStringField(TEXT("property"), PropertyName))
	{
		return CreateErrorResponse(TEXT("Missing 'property' parameter"));
	}

	if (!Params->HasField(TEXT("value")))
	{
		return CreateErrorResponse(TEXT("Missing 'value' parameter"));
	}

	AActor* Actor = FindActorByName(ActorName);
	if (!Actor)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
	}

	// Find the property on the actor's class
	UProperty* Property = Actor->GetClass()->FindPropertyByName(*PropertyName);
	if (!Property)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Property not found: %s"), *PropertyName));
	}

	// Check if property is editable
	if (Property->HasAnyPropertyFlags(CPF_EditConst))
	{
		return CreateErrorResponse(FString::Printf(TEXT("Property is read-only: %s"), *PropertyName));
	}

	// Get the value pointer
	void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Actor);
	if (!ValuePtr)
	{
		return CreateErrorResponse(TEXT("Failed to get property value pointer"));
	}

	// Get the JSON value
	TSharedPtr<FJsonValue> JsonValue = Params->TryGetField(TEXT("value"));

	// Convert and set the value
	if (!JsonValueToProperty(JsonValue, Property, ValuePtr))
	{
		return CreateErrorResponse(FString::Printf(
			TEXT("Failed to set property value. Property type: %s"),
			*GetPropertyTypeName(Property)
		));
	}

	// Mark actor as modified for editor undo/save
	Actor->Modify();
	Actor->MarkPackageDirty();

	// Notify about property change
	Actor->PostEditChange();

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), true);
	Result->SetStringField(TEXT("actor"), ActorName);
	Result->SetStringField(TEXT("property"), PropertyName);
	Result->SetStringField(TEXT("type"), GetPropertyTypeName(Property));
	Result->SetField(TEXT("value"), PropertyToJsonValue(Property, ValuePtr));

	return Result;
}

// ============================================================================
// Blueprint Actor Commands
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleSpawnBlueprintActor(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintPath;
	if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
	{
		return CreateErrorResponse(TEXT("Missing 'blueprint_path' parameter"));
	}

	FString ActorName;
	if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
	{
		return CreateErrorResponse(TEXT("Missing 'actor_name' parameter"));
	}

	FVector Location = GetVectorFromJson(Params, TEXT("location"));
	FRotator Rotation = GetRotatorFromJson(Params, TEXT("rotation"));
	FVector Scale(1, 1, 1);
	if (Params->HasField(TEXT("scale")))
	{
		Scale = GetVectorFromJson(Params, TEXT("scale"));
	}

	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		return CreateErrorResponse(TEXT("No editor world available"));
	}

	// Load the Blueprint class (append _C if not present)
	FString ClassPath = BlueprintPath;
	if (!ClassPath.EndsWith(TEXT("_C")))
	{
		ClassPath += TEXT("_C");
	}

	UClass* ActorClass = LoadClass<AActor>(nullptr, *ClassPath);
	if (!ActorClass)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Failed to load Blueprint class: %s"), *ClassPath));
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*ActorName);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParams);
	if (!NewActor)
	{
		return CreateErrorResponse(TEXT("Failed to spawn Blueprint actor"));
	}

	NewActor->SetActorScale3D(Scale);
	NewActor->MarkPackageDirty();

	return ActorToJsonObject(NewActor, true);
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleCopyActor(const TSharedPtr<FJsonObject>& Params)
{
	FString SourceName;
	if (!Params->TryGetStringField(TEXT("source_name"), SourceName))
	{
		return CreateErrorResponse(TEXT("Missing 'source_name' parameter"));
	}

	FString NewName;
	if (!Params->TryGetStringField(TEXT("new_name"), NewName))
	{
		return CreateErrorResponse(TEXT("Missing 'new_name' parameter"));
	}

	AActor* SourceActor = FindActorByName(SourceName);
	if (!SourceActor)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Source actor not found: %s"), *SourceName));
	}

	UWorld* World = SourceActor->GetWorld();
	if (!World)
	{
		return CreateErrorResponse(TEXT("No world available"));
	}

	// Determine spawn location
	FVector SpawnLocation = SourceActor->GetActorLocation();
	FRotator SpawnRotation = SourceActor->GetActorRotation();

	if (Params->HasField(TEXT("location")))
	{
		SpawnLocation = GetVectorFromJson(Params, TEXT("location"));
	}
	else if (Params->HasField(TEXT("offset")))
	{
		SpawnLocation += GetVectorFromJson(Params, TEXT("offset"));
	}
	else
	{
		SpawnLocation += FVector(500, 0, 0); // Default offset
	}

	if (Params->HasField(TEXT("rotation")))
	{
		SpawnRotation = GetRotatorFromJson(Params, TEXT("rotation"));
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*NewName);
	SpawnParams.Template = SourceActor; // Copy all properties from source
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = World->SpawnActor<AActor>(
		SourceActor->GetClass(),
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!NewActor)
	{
		return CreateErrorResponse(TEXT("Failed to copy actor"));
	}

	NewActor->SetActorScale3D(SourceActor->GetActorScale3D());
	NewActor->MarkPackageDirty();

	return ActorToJsonObject(NewActor, true);
}

// ============================================================================
// Asset Property Commands
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleGetAssetProperty(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath;
	if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
	{
		return CreateErrorResponse(TEXT("Missing 'asset_path' parameter"));
	}

	FString PropertyName;
	if (!Params->TryGetStringField(TEXT("property"), PropertyName))
	{
		return CreateErrorResponse(TEXT("Missing 'property' parameter"));
	}

	// Load the asset
	UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
	if (!Asset)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Failed to load asset: %s"), *AssetPath));
	}

	// Find the property
	UProperty* Property = Asset->GetClass()->FindPropertyByName(*PropertyName);
	if (!Property)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Property not found: %s"), *PropertyName));
	}

	// Get value pointer
	void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Asset);
	if (!ValuePtr)
	{
		return CreateErrorResponse(TEXT("Failed to get property value pointer"));
	}

	// Build response
	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("asset"), AssetPath);
	Response->SetStringField(TEXT("property"), PropertyName);
	Response->SetStringField(TEXT("type"), GetPropertyTypeName(Property));
	Response->SetField(TEXT("value"), PropertyToJsonValue(Property, ValuePtr));

	return Response;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleSetAssetProperty(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath;
	if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
	{
		return CreateErrorResponse(TEXT("Missing 'asset_path' parameter"));
	}

	FString PropertyName;
	if (!Params->TryGetStringField(TEXT("property"), PropertyName))
	{
		return CreateErrorResponse(TEXT("Missing 'property' parameter"));
	}

	if (!Params->HasField(TEXT("value")))
	{
		return CreateErrorResponse(TEXT("Missing 'value' parameter"));
	}

	// Load the asset
	UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
	if (!Asset)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Failed to load asset: %s"), *AssetPath));
	}

	// Find the property
	UProperty* Property = Asset->GetClass()->FindPropertyByName(*PropertyName);
	if (!Property)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Property not found: %s"), *PropertyName));
	}

	// Check if editable
	if (Property->HasAnyPropertyFlags(CPF_EditConst))
	{
		return CreateErrorResponse(TEXT("Property is read-only"));
	}

	// Get value pointer
	void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Asset);
	if (!ValuePtr)
	{
		return CreateErrorResponse(TEXT("Failed to get property value pointer"));
	}

	// Set the value using existing helper
	TSharedPtr<FJsonValue> JsonValue = Params->TryGetField(TEXT("value"));
	if (!JsonValueToProperty(JsonValue, Property, ValuePtr))
	{
		return CreateErrorResponse(TEXT("Failed to set property value"));
	}

	// Mark dirty (no compilation needed for Data Assets)
	Asset->Modify();
	Asset->MarkPackageDirty();

	// Build response
	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("asset"), AssetPath);
	Response->SetStringField(TEXT("property"), PropertyName);
	Response->SetField(TEXT("new_value"), PropertyToJsonValue(Property, ValuePtr));

	return Response;
}

// ============================================================================
// Blueprint Default Property Commands
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleGetBlueprintDefaultProperty(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintPath;
	if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
	{
		return CreateErrorResponse(TEXT("Missing 'blueprint_path' parameter"));
	}

	FString PropertyName;
	if (!Params->TryGetStringField(TEXT("property"), PropertyName))
	{
		return CreateErrorResponse(TEXT("Missing 'property' parameter"));
	}

	// Load the Blueprint class (append _C if not present)
	FString ClassPath = BlueprintPath;
	if (!ClassPath.EndsWith(TEXT("_C")))
	{
		ClassPath += TEXT("_C");
	}

	UClass* BPClass = LoadClass<UObject>(nullptr, *ClassPath);
	if (!BPClass)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Failed to load Blueprint class: %s"), *ClassPath));
	}

	// Get the Class Default Object (CDO)
	UObject* CDO = BPClass->GetDefaultObject();
	if (!CDO)
	{
		return CreateErrorResponse(TEXT("Failed to get Class Default Object"));
	}

	// Find the property
	UProperty* Property = BPClass->FindPropertyByName(*PropertyName);
	if (!Property)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Property not found: %s"), *PropertyName));
	}

	// Get value pointer from CDO
	void* ValuePtr = Property->ContainerPtrToValuePtr<void>(CDO);
	if (!ValuePtr)
	{
		return CreateErrorResponse(TEXT("Failed to get property value pointer"));
	}

	// Build response
	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("blueprint"), BlueprintPath);
	Response->SetStringField(TEXT("property"), PropertyName);
	Response->SetStringField(TEXT("type"), GetPropertyTypeName(Property));
	Response->SetField(TEXT("value"), PropertyToJsonValue(Property, ValuePtr));

	return Response;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleSetBlueprintDefaultProperty(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintPath;
	if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
	{
		return CreateErrorResponse(TEXT("Missing 'blueprint_path' parameter"));
	}

	FString PropertyName;
	if (!Params->TryGetStringField(TEXT("property"), PropertyName))
	{
		return CreateErrorResponse(TEXT("Missing 'property' parameter"));
	}

	if (!Params->HasField(TEXT("value")))
	{
		return CreateErrorResponse(TEXT("Missing 'value' parameter"));
	}

	// Load the Blueprint asset (not just the class)
	FString BPAssetPath = BlueprintPath;
	if (BPAssetPath.EndsWith(TEXT("_C")))
	{
		BPAssetPath = BPAssetPath.LeftChop(2);
	}

	UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *BPAssetPath);
	if (!Blueprint)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Failed to load Blueprint: %s"), *BPAssetPath));
	}

	// Get the generated class
	UClass* BPClass = Blueprint->GeneratedClass;
	if (!BPClass)
	{
		return CreateErrorResponse(TEXT("Blueprint has no GeneratedClass - needs compilation"));
	}

	// Get the CDO
	UObject* CDO = BPClass->GetDefaultObject();
	if (!CDO)
	{
		return CreateErrorResponse(TEXT("Failed to get Class Default Object"));
	}

	// Find the property
	UProperty* Property = BPClass->FindPropertyByName(*PropertyName);
	if (!Property)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Property not found: %s"), *PropertyName));
	}

	// Check if editable
	if (Property->HasAnyPropertyFlags(CPF_EditConst))
	{
		return CreateErrorResponse(TEXT("Property is read-only"));
	}

	// Get value pointer from CDO
	void* ValuePtr = Property->ContainerPtrToValuePtr<void>(CDO);
	if (!ValuePtr)
	{
		return CreateErrorResponse(TEXT("Failed to get property value pointer"));
	}

	// Set the value
	TSharedPtr<FJsonValue> JsonValue = Params->TryGetField(TEXT("value"));
	if (!JsonValueToProperty(JsonValue, Property, ValuePtr))
	{
		return CreateErrorResponse(TEXT("Failed to set property value"));
	}

	// Mark Blueprint dirty and recompile
	Blueprint->Modify();
	Blueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(Blueprint);

	// Build response
	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("blueprint"), BlueprintPath);
	Response->SetStringField(TEXT("property"), PropertyName);
	Response->SetField(TEXT("new_value"), PropertyToJsonValue(Property, ValuePtr));

	return Response;
}

// ============================================================================
// Data Table Commands
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::RowStructToJson(UScriptStruct* RowStruct, const void* RowData)
{
	TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();

	if (!RowStruct || !RowData)
	{
		return JsonObj;
	}

	// Iterate all properties in the struct
	for (TFieldIterator<UProperty> PropIt(RowStruct); PropIt; ++PropIt)
	{
		UProperty* Property = *PropIt;
		FString PropName = Property->GetName();

		const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

		// Create field object with type and value
		TSharedPtr<FJsonObject> FieldObj = MakeShared<FJsonObject>();
		FieldObj->SetStringField(TEXT("type"), GetPropertyTypeName(Property));
		FieldObj->SetField(TEXT("value"), PropertyToJsonValue(Property, ValuePtr));

		JsonObj->SetObjectField(PropName, FieldObj);
	}

	return JsonObj;
}

bool FEpicUnrealMCPEditorCommands::JsonToRowStruct(const TSharedPtr<FJsonObject>& JsonObj, UScriptStruct* RowStruct, void* RowData)
{
	if (!JsonObj.IsValid() || !RowStruct || !RowData)
	{
		return false;
	}

	for (TFieldIterator<UProperty> PropIt(RowStruct); PropIt; ++PropIt)
	{
		UProperty* Property = *PropIt;
		FString PropName = Property->GetName();

		if (JsonObj->HasField(PropName))
		{
			TSharedPtr<FJsonValue> JsonValue;

			// Support both {type, value} format and direct value
			const TSharedPtr<FJsonObject>* FieldObj;
			if (JsonObj->TryGetObjectField(PropName, FieldObj) && (*FieldObj)->HasField(TEXT("value")))
			{
				JsonValue = (*FieldObj)->TryGetField(TEXT("value"));
			}
			else
			{
				JsonValue = JsonObj->TryGetField(PropName);
			}

			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

			if (!JsonValueToProperty(JsonValue, Property, ValuePtr))
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to set DataTable row property: %s"), *PropName);
				// Continue with other properties
			}
		}
	}
	return true;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleListDataTableRows(const TSharedPtr<FJsonObject>& Params)
{
	FString DataTablePath;
	if (!Params->TryGetStringField(TEXT("data_table_path"), DataTablePath))
	{
		return CreateErrorResponse(TEXT("Missing 'data_table_path' parameter"));
	}

	UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *DataTablePath);
	if (!DataTable)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Failed to load DataTable: %s"), *DataTablePath));
	}

	TArray<FName> RowNames = DataTable->GetRowNames();
	TArray<TSharedPtr<FJsonValue>> RowNamesJson;

	for (const FName& RowName : RowNames)
	{
		RowNamesJson.Add(MakeShared<FJsonValueString>(RowName.ToString()));
	}

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), true);
	Result->SetStringField(TEXT("data_table_path"), DataTablePath);
	Result->SetStringField(TEXT("row_struct"), DataTable->GetRowStruct() ? DataTable->GetRowStruct()->GetName() : TEXT("Unknown"));
	Result->SetArrayField(TEXT("row_names"), RowNamesJson);
	Result->SetNumberField(TEXT("count"), RowNames.Num());

	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleGetDataTableRow(const TSharedPtr<FJsonObject>& Params)
{
	FString DataTablePath;
	if (!Params->TryGetStringField(TEXT("data_table_path"), DataTablePath))
	{
		return CreateErrorResponse(TEXT("Missing 'data_table_path' parameter"));
	}

	FString RowName;
	if (!Params->TryGetStringField(TEXT("row_name"), RowName))
	{
		return CreateErrorResponse(TEXT("Missing 'row_name' parameter"));
	}

	UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *DataTablePath);
	if (!DataTable)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Failed to load DataTable: %s"), *DataTablePath));
	}

	const UScriptStruct* RowStruct = DataTable->GetRowStruct();
	if (!RowStruct)
	{
		return CreateErrorResponse(TEXT("DataTable has no row struct"));
	}

	void* RowData = DataTable->FindRowUnchecked(FName(*RowName));
	if (!RowData)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Row not found: %s"), *RowName));
	}

	TSharedPtr<FJsonObject> RowJson = RowStructToJson(const_cast<UScriptStruct*>(RowStruct), RowData);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), true);
	Result->SetStringField(TEXT("data_table_path"), DataTablePath);
	Result->SetStringField(TEXT("row_name"), RowName);
	Result->SetStringField(TEXT("row_struct"), RowStruct->GetName());
	Result->SetObjectField(TEXT("row_data"), RowJson);

	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleSetDataTableRowField(const TSharedPtr<FJsonObject>& Params)
{
	FString DataTablePath;
	if (!Params->TryGetStringField(TEXT("data_table_path"), DataTablePath))
	{
		return CreateErrorResponse(TEXT("Missing 'data_table_path' parameter"));
	}

	FString RowName;
	if (!Params->TryGetStringField(TEXT("row_name"), RowName))
	{
		return CreateErrorResponse(TEXT("Missing 'row_name' parameter"));
	}

	FString FieldName;
	if (!Params->TryGetStringField(TEXT("field_name"), FieldName))
	{
		return CreateErrorResponse(TEXT("Missing 'field_name' parameter"));
	}

	if (!Params->HasField(TEXT("value")))
	{
		return CreateErrorResponse(TEXT("Missing 'value' parameter"));
	}

	UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *DataTablePath);
	if (!DataTable)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Failed to load DataTable: %s"), *DataTablePath));
	}

	const UScriptStruct* RowStruct = DataTable->GetRowStruct();
	if (!RowStruct)
	{
		return CreateErrorResponse(TEXT("DataTable has no row struct"));
	}

	void* RowData = DataTable->FindRowUnchecked(FName(*RowName));
	if (!RowData)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Row not found: %s"), *RowName));
	}

	// Find the property in the row struct
	FProperty* Property = const_cast<UScriptStruct*>(RowStruct)->FindPropertyByName(FName(*FieldName));
	if (!Property)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Field not found: %s"), *FieldName));
	}

	void* FieldPtr = Property->ContainerPtrToValuePtr<void>(RowData);
	TSharedPtr<FJsonValue> JsonValue = Params->TryGetField(TEXT("value"));

	if (!JsonValueToProperty(JsonValue, Property, FieldPtr))
	{
		return CreateErrorResponse(FString::Printf(
			TEXT("Failed to set field value. Field type: %s"),
			*GetPropertyTypeName(Property)
		));
	}

	// Mark dirty and notify
	DataTable->Modify();
	DataTable->MarkPackageDirty();
	DataTable->HandleDataTableChanged(FName(*RowName));

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), true);
	Result->SetStringField(TEXT("data_table_path"), DataTablePath);
	Result->SetStringField(TEXT("row_name"), RowName);
	Result->SetStringField(TEXT("field_name"), FieldName);
	Result->SetStringField(TEXT("field_type"), GetPropertyTypeName(Property));
	Result->SetField(TEXT("new_value"), PropertyToJsonValue(Property, FieldPtr));

	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleAddDataTableRow(const TSharedPtr<FJsonObject>& Params)
{
	FString DataTablePath;
	if (!Params->TryGetStringField(TEXT("data_table_path"), DataTablePath))
	{
		return CreateErrorResponse(TEXT("Missing 'data_table_path' parameter"));
	}

	FString RowName;
	if (!Params->TryGetStringField(TEXT("row_name"), RowName))
	{
		return CreateErrorResponse(TEXT("Missing 'row_name' parameter"));
	}

	UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *DataTablePath);
	if (!DataTable)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Failed to load DataTable: %s"), *DataTablePath));
	}

	const UScriptStruct* RowStructConst = DataTable->GetRowStruct();
	if (!RowStructConst)
	{
		return CreateErrorResponse(TEXT("DataTable has no row struct"));
	}
	UScriptStruct* RowStruct = const_cast<UScriptStruct*>(RowStructConst);

	// Check if row already exists
	if (DataTable->FindRowUnchecked(FName(*RowName)))
	{
		return CreateErrorResponse(FString::Printf(TEXT("Row already exists: %s"), *RowName));
	}

	// Allocate and initialize new row data
	void* NewRowData = FMemory::Malloc(RowStruct->GetStructureSize());
	RowStruct->InitializeStruct(NewRowData);

	// If row_data provided, populate the fields
	const TSharedPtr<FJsonObject>* RowDataJson;
	if (Params->TryGetObjectField(TEXT("row_data"), RowDataJson))
	{
		if (!JsonToRowStruct(*RowDataJson, RowStruct, NewRowData))
		{
			RowStruct->DestroyStruct(NewRowData);
			FMemory::Free(NewRowData);
			return CreateErrorResponse(TEXT("Failed to parse row_data"));
		}
	}

	// Add the row to the DataTable
	DataTable->AddRow(FName(*RowName), *(FTableRowBase*)NewRowData);

	// Clean up our temporary allocation (AddRow copies the data)
	RowStruct->DestroyStruct(NewRowData);
	FMemory::Free(NewRowData);

	DataTable->Modify();
	DataTable->MarkPackageDirty();

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), true);
	Result->SetStringField(TEXT("data_table_path"), DataTablePath);
	Result->SetStringField(TEXT("row_name"), RowName);
	Result->SetStringField(TEXT("message"), TEXT("Row added successfully"));

	return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleDeleteDataTableRow(const TSharedPtr<FJsonObject>& Params)
{
	FString DataTablePath;
	if (!Params->TryGetStringField(TEXT("data_table_path"), DataTablePath))
	{
		return CreateErrorResponse(TEXT("Missing 'data_table_path' parameter"));
	}

	FString RowName;
	if (!Params->TryGetStringField(TEXT("row_name"), RowName))
	{
		return CreateErrorResponse(TEXT("Missing 'row_name' parameter"));
	}

	UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *DataTablePath);
	if (!DataTable)
	{
		return CreateErrorResponse(FString::Printf(TEXT("Failed to load DataTable: %s"), *DataTablePath));
	}

	// Check if row exists
	if (!DataTable->FindRowUnchecked(FName(*RowName)))
	{
		return CreateErrorResponse(FString::Printf(TEXT("Row not found: %s"), *RowName));
	}

	// Remove the row
	DataTable->RemoveRow(FName(*RowName));

	DataTable->Modify();
	DataTable->MarkPackageDirty();

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("success"), true);
	Result->SetStringField(TEXT("data_table_path"), DataTablePath);
	Result->SetStringField(TEXT("row_name"), RowName);
	Result->SetStringField(TEXT("message"), TEXT("Row deleted successfully"));

	return Result;
}
