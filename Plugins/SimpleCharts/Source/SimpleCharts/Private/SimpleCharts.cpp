// Copyright 2020 LHM. All Rights Reserved
#include "SimpleCharts.h"
#include "Runtime/Core/Public/Misc/LocalTimestampDirectoryVisitor.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"
#include "Runtime/Core/Public/HAL/FileManager.h"

void FSimpleChartsModule::CopyHtmlFiles()
{
	
#if PLATFORM_ANDROID
	extern FString GExternalFilePath;
	IFileManager* FileManager = &IFileManager::Get();
	FString Path = GExternalFilePath / "ThirdParty/";
	if (FileManager->DirectoryExists(*Path))
	{
		return;
	}
	//FString DirectoryPath = FPaths::ProjectDir() + "Plugins/SimpleCharts/Source/ThirdParty";
	FString DirectoryPath = FPaths::ConvertRelativePathToFull(FPaths::EnginePluginsDir() + "Marketplace/SimpleCharts/Source/ThirdParty");
	if (!FPaths::DirectoryExists(DirectoryPath))
	{
		DirectoryPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectPluginsDir() + "SimpleCharts/Source/ThirdParty");
		if (!FPaths::DirectoryExists(DirectoryPath))
		{
			DirectoryPath = FPaths::ConvertRelativePathToFull(FPaths::EnterprisePluginsDir() + "Marketplace/SimpleCharts/Source/ThirdParty");
		}
	}

	//only do this on Android
	extern FString GExternalFilePath;

	// iterate over all the files in provided directory
	TArray<FString> directoriesToIgnoreAndNotRecurse;
	FLocalTimestampDirectoryVisitor Visitor(FPlatformFileManager::Get().GetPlatformFile(), directoriesToIgnoreAndNotRecurse, directoriesToIgnoreAndNotRecurse, false);
	FileManager->IterateDirectory(*DirectoryPath, Visitor);

	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		// read the file contents and write it if successful to external path
		TArray<uint8> MemFile;
		const FString SourceFilename = TimestampIt.Key();
		if (FFileHelper::LoadFileToArray(MemFile, *SourceFilename, 0))
		{
			FString Left;
			FString Right;
			SourceFilename.Split(TEXT("SimpleCharts"), &Left, &Right);
			FString DestFilename = GExternalFilePath / Right;
			FFileHelper::SaveArrayToFile(MemFile, *DestFilename);
		}
	}
#endif
}

FSimpleChartsModule* FSimpleChartsModule::Singleton = nullptr;

void FSimpleChartsModule::StartupModule()
{
	CopyHtmlFiles();
	Singleton = this;
}

void FSimpleChartsModule::ShutdownModule()
{
	Singleton = nullptr;
}

FSimpleChartsModule& FSimpleChartsModule::Get()
{
	return *Singleton;
}

IMPLEMENT_MODULE(FSimpleChartsModule, SimpleCharts)
