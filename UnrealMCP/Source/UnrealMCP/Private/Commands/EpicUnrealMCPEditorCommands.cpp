#include "Commands/EpicUnrealMCPEditorCommands.h"
#include "Editor.h"
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
