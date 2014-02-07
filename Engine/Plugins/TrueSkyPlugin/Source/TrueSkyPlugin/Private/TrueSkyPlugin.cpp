// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#include "TrueSkyPluginPrivatePCH.h"
#include "PlacementModePrivatePCH.h"
#include "TrueSkySequenceAsset.h"
#include "TrueSkySequenceFactory.h"
#include "LevelEditor.h"
#include "IMainFrameModule.h"
#include "GenericWindow.h"
#include "WindowsWindow.h"
#include "RendererInterface.h"
#include "DynamicRHI.h"
#include "D3D11RHIPrivate.h"
#include "AssetToolsModule.h"
#include "AssetTypeActions_TrueSkySequence.h"
#include "TrueSkyPlugin.generated.inl"
#include "AssetPaletteFactoryFilter.h"
#include <string>


class FTrueSkyPlugin : public ITrueSkyPlugin
{
public:
	FTrueSkyPlugin();

	/** IModuleInterface implementation */
	virtual void StartupModule() OVERRIDE;
	virtual void ShutdownModule() OVERRIDE;
	virtual bool SupportsDynamicReloading() OVERRIDE;

	/** Render delegate */
	void RenderFrame( FPostOpaqueRenderParameters& RenderParameters );

	/** Extend menu */
	void FillMenu( FMenuBuilder& MenuBuilder );

	/** Open UI */
	void StartUI( const TCHAR* SimulPath, const TCHAR* QtPath );

	/** Init rendering */
	void InitRenderingInterface( const TCHAR* SimulPath, const TCHAR* QtPath );

	/** Enable rendering */
	void SetRenderingEnabled( bool Enabled );

	/** Open editor */
	virtual void OpenEditor(UTrueSkySequenceAsset* const TrueSkySequence);

protected:

	void					OnMainWindowClosed(const TSharedRef<SWindow>& Window);
	TSharedRef<SDockTab>	DockTabSpawner(const FSpawnTabArgs& Args);

	/** Open menu item */
	void	OnOpen();

private:

	TSharedPtr< FExtender > MenuExtender;
	TSharedPtr<FAssetTypeActions_TrueSkySequence> SequenceAssetTypeActions;
	
	typedef void (* FOpenUI)(HWND, RECT*, RECT*);
	typedef void (* FCloseUI)();
	typedef void (* FSetStyleSheetPath)(const TCHAR*);
	typedef void (* FSetSequence)(std::string&);

	typedef int (* FStaticInitInterface)( const char* shaderPath, const char* texturePath );
	typedef int (* FStaticRenderFrame)( void* device, float* viewMatrix4x4, float* projMatrix4x4, void* halfResDepthBuffer );
	typedef int (* FStaticTick)( float deltaTime );
	typedef int (* FStaticOnDeviceChanged)( void * device );

	FOpenUI					OpenUI;
	FCloseUI				CloseUI;
	FSetStyleSheetPath		SetStyleSheetPath;
	FSetSequence			SetSequence;

	FStaticInitInterface	StaticInitInterface;
	FStaticRenderFrame		StaticRenderFrame;
	FStaticTick				StaticTick;
	FStaticOnDeviceChanged	StaticOnDeviceChanged;

	TCHAR*					PathEnv;
	TSharedPtr<SDockTab>	DockTab;
	TSharedPtr<SWindow>		MainWindow;
	bool					RenderingEnabled;
	bool					EditorOpened;

	static WNDPROC			OrigMainWindowWndProc;
	static LRESULT CALLBACK MainWindowWndProc(HWND, ::UINT, WPARAM, LPARAM);
	static ::UINT			MessageId;

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


FTrueSkyPlugin::FTrueSkyPlugin() : EditorOpened(false)
{
}


bool FTrueSkyPlugin::SupportsDynamicReloading()
{
	return false;
}


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

	SequenceAssetTypeActions = MakeShareable(new FAssetTypeActions_TrueSkySequence);
	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().RegisterAssetTypeActions(SequenceAssetTypeActions.ToSharedRef());

	GetRendererModule().RegisterPostOpaqueRenderDelegate( FPostOpaqueRenderDelegate::CreateRaw(this, &FTrueSkyPlugin::RenderFrame) );

	RenderingEnabled = false;
	StaticInitInterface = NULL;
	StaticRenderFrame  = NULL;
	StaticTick  = NULL;
	StaticOnDeviceChanged = NULL;

	PathEnv = NULL;
	MessageId = RegisterWindowMessage(L"RESIZE");
}

void FTrueSkyPlugin::SetRenderingEnabled( bool Enabled )
{
	RenderingEnabled = Enabled;
}


void FTrueSkyPlugin::RenderFrame( FPostOpaqueRenderParameters& RenderParameters )
{	
	if( RenderingEnabled )
	{
		StaticTick(0.0f);

		FD3D11DynamicRHI * d3d11rhi = (FD3D11DynamicRHI*)GDynamicRHI;
		ID3D11Device * device = d3d11rhi->GetDevice();
		ID3D11DeviceContext * context = d3d11rhi->GetDeviceContext();

		StaticRenderFrame( device, &(RenderParameters.ViewMatrix.M[0][0]), &(RenderParameters.ProjMatrix.M[0][0]), NULL );
	}
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

		FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(SequenceAssetTypeActions.ToSharedRef());
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


static HWND GetSWindowHWND(const TSharedPtr<SWindow>& Window)
{
	if ( Window.IsValid() )
	{
		TSharedPtr<FWindowsWindow> WindowsWindow = StaticCastSharedPtr<FWindowsWindow>(Window->GetNativeWindow());
		if ( WindowsWindow.IsValid() )
		{
			return WindowsWindow->GetHWnd();
		}
	}
	return 0;
}


void FTrueSkyPlugin::InitRenderingInterface( const TCHAR* SimulPath, const TCHAR* QtPath )
{
	const TCHAR* const DllPath = ConstructPath( SimulPath, L"\\exe\\x64\\VC11\\Debug\\UE4PluginRenderInterface.dll" );
	check( DllPath );

	void* const DllHandle = FPlatformProcess::GetDllHandle( DllPath );

	if ( DllHandle != NULL )
	{
		StaticInitInterface = (FStaticInitInterface)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticInitInterface") );
		StaticRenderFrame = (FStaticRenderFrame)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticRenderFrame") );
		StaticOnDeviceChanged = (FStaticOnDeviceChanged)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticOnDeviceChanged") );
		StaticTick = (FStaticTick)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticTick") );

		if( StaticInitInterface == NULL || StaticRenderFrame == NULL || 
			StaticOnDeviceChanged == NULL || StaticTick == NULL )
		{
			//missing dll functions... cancel initialization
			SetRenderingEnabled(false);
			return;
		}

		const char* const ShaderPath = ConstructPathUTF8( SimulPath, L"\\Platform\\DirectX11\\HLSL" );
		const char* const ResourcePath = ConstructPathUTF8( SimulPath, L"\\Media\\Textures" );

		StaticInitInterface( ShaderPath, ResourcePath );

		FD3D11DynamicRHI * d3d11rhi = (FD3D11DynamicRHI*)GDynamicRHI;
		ID3D11Device * device = d3d11rhi->GetDevice();

		if( device != NULL )
		{
			StaticOnDeviceChanged(device);
		}

		SetRenderingEnabled(true);
	}
}


void FTrueSkyPlugin::StartUI( const TCHAR* SimulPath, const TCHAR* QtPath )
{
	const TCHAR* const DllPath = ConstructPath( SimulPath, L"\\exe\\x64\\VC11\\Debug\\TrueSkyUI.dll" );
	check( DllPath );

	void* const DllHandle = FPlatformProcess::GetDllHandle( DllPath );

	if ( DllHandle != NULL )
	{
		OpenUI = (FOpenUI)FPlatformProcess::GetDllExport(DllHandle, TEXT("OpenUI") );
		CloseUI = (FCloseUI)FPlatformProcess::GetDllExport(DllHandle, TEXT("CloseUI") );
		SetStyleSheetPath = (FSetStyleSheetPath)FPlatformProcess::GetDllExport(DllHandle, TEXT("SetStyleSheetPath") );
		SetSequence = (FSetSequence)FPlatformProcess::GetDllExport(DllHandle, TEXT("SetSequence") );

		if ( SetStyleSheetPath )
		{
			const TCHAR* StyleSheetPath = ConstructPath( SimulPath, L"\\PlugIns\\UE4\\TrueSkyUI\\qss\\" );
			SetStyleSheetPath( StyleSheetPath );
			delete StyleSheetPath;
		}
		if ( OpenUI )
		{
			// Get HWND
			IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
			TSharedPtr<SWindow> ParentWindow = MainFrameModule.GetParentWindow();
			if ( ParentWindow.IsValid() )
			{
				if ( !MainWindow.IsValid() )
				{
					MainWindow = SNew(SWindow)
						.Title( FText::FromString(TEXT("TrueSky")) )
						.ClientSize( FVector2D(800.0f, 600.0f) )
						.AutoCenter( EAutoCenter::PrimaryWorkArea )
						.SizingRule( ESizingRule::UserSized )
						// .IsPopupWindow( true );
						;

					MainWindow->SetOnWindowClosed( FOnWindowClosed::CreateRaw(this, &FTrueSkyPlugin::OnMainWindowClosed) );

					FSlateApplication::Get().AddWindowAsNativeChild( MainWindow.ToSharedRef(), ParentWindow.ToSharedRef() );

//					FGlobalTabmanager::Get()->RegisterNomadTabSpawner( FName("TrueSky"), FOnSpawnTab::CreateRaw(this, &FTrueSkyPlugin::DockTabSpawner) );
				}

				HWND MainWindowHWND = GetSWindowHWND(MainWindow);
				if ( MainWindowHWND )
				{
					const LONG_PTR wndStyle = GetWindowLongPtr( MainWindowHWND, GWL_STYLE );
					SetWindowLongPtr( MainWindowHWND, GWL_STYLE, wndStyle | WS_CLIPCHILDREN );

					const FVector2D ClientSize = MainWindow->GetClientSizeInScreen();
					const FMargin Margin = MainWindow->GetWindowBorderSize();
					RECT ClientRect;
					ClientRect.left = Margin.Left;
					ClientRect.top = Margin.Top + MainWindow->GetTitleBarSize().Get();
					ClientRect.right = ClientSize.X - Margin.Right;
					ClientRect.bottom = ClientSize.Y - Margin.Bottom;
					RECT ParentRect;
					ParentRect.left = 0;
					ParentRect.top = 0;
					ParentRect.right = ClientSize.X;
					ParentRect.bottom = ClientSize.Y;

					OpenUI( MainWindowHWND, &ClientRect, &ParentRect );

					// Overload main window's WndProc
					OrigMainWindowWndProc = (WNDPROC)GetWindowLongPtr( MainWindowHWND, GWLP_WNDPROC );
					SetWindowLongPtr( MainWindowHWND, GWLP_WNDPROC, (LONG_PTR)MainWindowWndProc );

					EditorOpened = true;
				}
			}
		}
	}
}


void FTrueSkyPlugin::OnMainWindowClosed(const TSharedRef<SWindow>& Window)
{
	if ( CloseUI )
	{
		CloseUI();
	}
	MainWindow = NULL;
	EditorOpened = false;
}


TSharedRef<SDockTab> FTrueSkyPlugin::DockTabSpawner(const FSpawnTabArgs& Args)
{
	// Test only
	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("ClassViewer.TabIcon"))
		.TabRole( ETabRole::NomadTab )
		.Label( FText::FromString(L"TrueSky") )
		[
			SNew(SButton)
		];
}



WNDPROC FTrueSkyPlugin::OrigMainWindowWndProc = NULL;
::UINT FTrueSkyPlugin::MessageId = 0;

LRESULT CALLBACK FTrueSkyPlugin::MainWindowWndProc(HWND hWnd, ::UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ( uMsg == WM_SIZE )
	{
		if ( HWND ChildHWND = GetWindow(hWnd, GW_CHILD) ) 
		{
			PostMessage(ChildHWND, MessageId, wParam, lParam);
		}
	}
	return CallWindowProc( OrigMainWindowWndProc, hWnd, uMsg, wParam, lParam );
}


void FTrueSkyPlugin::OnOpen()
{
	OpenEditor( NULL );
}


void FTrueSkyPlugin::OpenEditor(UTrueSkySequenceAsset* const TrueSkySequence)
{
	// TrueSky environment reads from std::string
	std::string SequenceInputText;
	if ( TrueSkySequence && TrueSkySequence->SequenceText.Num() > 0 )
	{
		SequenceInputText = std::string( (const char*)TrueSkySequence->SequenceText.GetData() );
	}

	if ( !EditorOpened )
	{
		// Open UI + initialize render

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

		StartUI( SimulPath, QtPath );
		InitRenderingInterface( SimulPath, QtPath );

		delete QtPath;
		delete SimulPath;
	}

	// Set sequence asset to UI
	if ( SetSequence )
	{
		SetSequence( SequenceInputText );
	}
}





