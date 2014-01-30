// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#include "TrueSkyPluginPrivatePCH.h"
#include "LevelEditor.h"
#include "IMainFrameModule.h"

class FTrueSkyPlugin : public ITrueSkyPlugin
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() OVERRIDE;
	virtual void ShutdownModule() OVERRIDE;

	/** Extend menu */
	void FillMenu( FMenuBuilder& MenuBuilder );

	/** Menu command */
	void OnOpen();

private:

	TSharedPtr< FExtender > MenuExtender;

	typedef void (* FOpenUI)();
	typedef void (* FCloseUI)();
	typedef void (* FSetStyleSheetPath)(const TCHAR*);

	FOpenUI					OpenUI;
	FCloseUI				CloseUI;
	FSetStyleSheetPath		SetStyleSheetPath;

	TCHAR* PathEnv;
};

IMPLEMENT_MODULE( FTrueSkyPlugin, TrueSkyPlugin )


class FTrueSkyCommands : public TCommands<FTrueSkyCommands>
{
public:
	FTrueSkyCommands()
		: TCommands<FTrueSkyCommands>(
		TEXT("TrueSky"), // Context name for fast lookup
//		NSLOCTEXT("Contexts", "PaperEditor", "Sprite Editor"), // Localized context name for displaying
		FText::FromString(TEXT("TrueSky plugin")),
		NAME_None) // Parent
	{
	}
	virtual void RegisterCommands() OVERRIDE
	{
		UI_COMMAND(Open, "Open", "Opens SkySequencer.", EUserInterfaceActionType::Button, FInputGesture());
	}
public:
	TSharedPtr<FUICommandInfo> Open;
};


void FTrueSkyPlugin::StartupModule()
{
	FTrueSkyCommands::Register();
 	IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
 	const TSharedRef<FUICommandList>& CommandList = MainFrameModule.GetMainFrameCommandBindings();
	CommandList->MapAction( FTrueSkyCommands::Get().Open, FExecuteAction::CreateRaw(this, &FTrueSkyPlugin::OnOpen) );

	MenuExtender = MakeShareable(new FExtender);
	MenuExtender->AddMenuExtension("WindowGlobalTabSpawners", EExtensionHook::After, CommandList, FMenuExtensionDelegate::CreateRaw(this, &FTrueSkyPlugin::FillMenu));
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>( "LevelEditor" );
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

	PathEnv = NULL;
}


void FTrueSkyPlugin::ShutdownModule()
{
// 	if ( FModuleManager::Get().IsModuleLoaded("MainFrame") )
// 	{
// 		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
// 		MainFrameModule.OnMainFrameCreationFinished().RemoveAll(this);
// 	}
	FTrueSkyCommands::Unregister();
	if ( FModuleManager::Get().IsModuleLoaded("LevelEditor") )
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetMenuExtensibilityManager()->RemoveExtender( MenuExtender );
	}

	delete PathEnv;
	PathEnv = NULL;
}


void FTrueSkyPlugin::FillMenu( FMenuBuilder& MenuBuilder )
{
	MenuBuilder.BeginSection( "TrueSky", FText::FromString(TEXT("TrueSky")) );
	{		
		MenuBuilder.AddMenuEntry( FTrueSkyCommands::Get().Open );
	}
	MenuBuilder.EndSection();
}

/** Return environment variable value */
static TCHAR* GetEnvVariable( const TCHAR* const VariableName, int iEnvSize = 1024)
{
	TCHAR* Env = new TCHAR[iEnvSize];
	check( Env );
	memset(Env, 0, iEnvSize * sizeof(TCHAR));
	if ( (int)GetEnvironmentVariable(VariableName, Env, iEnvSize) > iEnvSize )
	{
		delete Env;
		Env = NULL;
	}
	else if ( wcslen(Env) == 0 )
	{
		TCHAR errorMsg[512];
		swprintf_s(errorMsg, 512, L"No %s environment variable set!", VariableName);
		MessageBox(NULL, errorMsg, L"Error", MB_OK);
	}
	return Env;
}

/** Takes Base path, concatenates it with Relative path */
static const TCHAR* ConstructPath(const TCHAR* const BasePath, const TCHAR* const RelativePath)
{
	if ( BasePath )
	{
		const int iPathLen = 1024;
		TCHAR* const NewPath = new TCHAR[iPathLen];
		check( NewPath );
		wcscpy_s( NewPath, iPathLen, BasePath );
		if ( RelativePath )
		{
			wcscat_s( NewPath, iPathLen, RelativePath );
		}
		return NewPath;
	}
	return NULL;
}

/** Takes Base path, concatenates it with Relative path and returns it as 8-bit char string */
static const char* ConstructPathUTF8(const TCHAR* const BasePath, const TCHAR* const RelativePath)
{
	if ( BasePath )
	{
		const int iPathLen = 1024;

		TCHAR* const NewPath = new TCHAR[iPathLen];
		check( NewPath );
		wcscpy_s( NewPath, iPathLen, BasePath );
		if ( RelativePath )
		{
			wcscat_s( NewPath, iPathLen, RelativePath );
		}

		char* const utf8NewPath = new char[iPathLen];
		check ( utf8NewPath );
		memset(utf8NewPath, 0, iPathLen);
		WideCharToMultiByte( CP_UTF8, 0, NewPath, iPathLen, utf8NewPath, iPathLen, NULL, NULL );

		delete NewPath;

		return utf8NewPath;
	}
	return NULL;
}



void FTrueSkyPlugin::OnOpen()
{
#if PLATFORM_WINDOWS

	const TCHAR* const SimulPath = GetEnvVariable(L"SIMUL");
	const TCHAR* const QtPath = GetEnvVariable(L"QTDIR");
	if ( SimulPath == NULL || QtPath == NULL )
	{
		delete SimulPath;
		delete QtPath;
		return;
	}

	if ( PathEnv == NULL )
	{
		const int iPathSize = 4096;
		PathEnv = GetEnvVariable(L"PATH", iPathSize);
		if ( PathEnv == NULL )
		{
			delete SimulPath;
			delete QtPath;
			return;
		}
		if ( wcslen(PathEnv) > 0 )
		{
			wcscat_s( PathEnv, iPathSize, L";" );
			wcscat_s( PathEnv, iPathSize, QtPath ); wcscat_s( PathEnv, iPathSize, L"\\bin;" );
			wcscat_s( PathEnv, iPathSize, SimulPath ); wcscat_s( PathEnv, iPathSize, L"\\exe\\x64;" );
			wcscat_s( PathEnv, iPathSize, SimulPath ); wcscat_s( PathEnv, iPathSize, L"\\exe\\x64\\VC11\\Debug" );
		}
		else
		{
			wcscpy_s( PathEnv, iPathSize, QtPath ); wcscat_s( PathEnv, iPathSize, L"\\bin;" );
			wcscat_s( PathEnv, iPathSize, SimulPath ); wcscat_s( PathEnv, iPathSize, L"\\exe\\x64;" );
			wcscat_s( PathEnv, iPathSize, SimulPath ); wcscat_s( PathEnv, iPathSize, L"\\exe\\x64\\VC11\\Debug" );
		}
		SetEnvironmentVariable( L"PATH", PathEnv );
	}

	const TCHAR* const DllPath = ConstructPath( SimulPath, L"\\exe\\x64\\VC11\\Debug\\TrueSkyUI.dll" );
	check( DllPath );

	void* const DllHandle = FPlatformProcess::GetDllHandle( DllPath );

	if ( DllHandle != NULL )
	{
		OpenUI = (FOpenUI)FPlatformProcess::GetDllExport(DllHandle, TEXT("OpenUI") );
		CloseUI = (FCloseUI)FPlatformProcess::GetDllExport(DllHandle, TEXT("CloseUI") );
		SetStyleSheetPath = (FSetStyleSheetPath)FPlatformProcess::GetDllExport(DllHandle, TEXT("SetStyleSheetPath") );

		if ( SetStyleSheetPath )
		{
			const TCHAR* StyleSheetPath = ConstructPath( SimulPath, L"\\PlugIns\\UE4\\TrueSkyUI\\qss\\" );
			SetStyleSheetPath( StyleSheetPath );
			delete StyleSheetPath;
		}
		if ( OpenUI )
		{
			OpenUI();
		}
	}

	delete QtPath;
	delete SimulPath;
	delete DllPath;

#endif
}

