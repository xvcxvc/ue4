// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#include "TrueSkyPluginPrivatePCH.h"
#include "LevelEditor.h"
#include "IMainFrameModule.h"
#include "GenericWindow.h"
#include "WindowsWindow.h"
#include "RendererInterface.h"
#include "DynamicRHI.h"
#include "D3D11RHIPrivate.h"


class FTrueSkyPlugin : public ITrueSkyPlugin
{
public:

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

	/** Menu command */
	void OnOpen();

protected:

	void		OnMainWindowClosed(const TSharedRef<SWindow>& Window);

private:

	TSharedPtr< FExtender > MenuExtender;

	typedef void (* FOpenUI)(HWND, RECT*, RECT*);
	typedef void (* FCloseUI)();
	typedef void (* FSetStyleSheetPath)(const TCHAR*);

	typedef int (* FStaticInitInterface)( const char* shaderPath, const char* texturePath );
	typedef int (* FStaticRenderFrame)( void* device, float* viewMatrix4x4, float* projMatrix4x4, void* halfResDepthBuffer );
	typedef int (* FStaticTick)( float deltaTime );
	typedef int (* FStaticOnDeviceChanged)( void * device );

	FOpenUI					OpenUI;
	FCloseUI				CloseUI;
	FSetStyleSheetPath		SetStyleSheetPath;

	FStaticInitInterface	StaticInitInterface;
	FStaticRenderFrame		StaticRenderFrame;
	FStaticTick				StaticTick;
	FStaticOnDeviceChanged	StaticOnDeviceChanged;

	TCHAR*					PathEnv;
	TSharedPtr<SWindow>		MainWindow;
	bool					RenderingEnabled;

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

	GetRendererModule().RegisterPostOpaqueRenderDelegate( FPostOpaqueRenderDelegate::CreateRaw(this, &FTrueSkyPlugin::RenderFrame) );

	RenderingEnabled = false;
	PathEnv = NULL;
	MessageId = RegisterWindowMessage(L"RESIZE");
}


void FTrueSkyPlugin::SetRenderingEnabled( bool Enabled )
{
	RenderingEnabled = Enabled;
}


void FTrueSkyPlugin::RenderFrame( FPostOpaqueRenderParameters& RenderParameters )
{	
	//if( RenderingEnabled )
	if( false )
	{
		FD3D11DynamicRHI * d3d11rhi = (FD3D11DynamicRHI*)GDynamicRHI;
		ID3D11Device * device = d3d11rhi->GetDevice();
		ID3D11DeviceContext * context = d3d11rhi->GetDeviceContext();

		StaticRenderFrame( device, NULL, NULL, NULL );
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

		if ( StaticInitInterface )
		{
			const char* const ShaderPath = ConstructPathUTF8( SimulPath, L"\\Platform\\DirectX11\\HLSL" );
			const char* const ResourcePath = ConstructPathUTF8( SimulPath, L"\\Media\\Textures" );

			StaticInitInterface( ShaderPath, ResourcePath );

			if( StaticOnDeviceChanged != NULL )
			{
				FD3D11DynamicRHI * d3d11rhi = (FD3D11DynamicRHI*)GDynamicRHI;
				ID3D11Device * device = d3d11rhi->GetDevice();

				if( device != NULL )
				{
					StaticOnDeviceChanged(device);
				}
			}

			SetRenderingEnabled(true);
		}
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
						//						.IsPopupWindow( true );
						;

					MainWindow->SetOnWindowClosed( FOnWindowClosed::CreateRaw(this, &FTrueSkyPlugin::OnMainWindowClosed) );

					FSlateApplication::Get().AddWindowAsNativeChild( MainWindow.ToSharedRef(), ParentWindow.ToSharedRef() );
				}

				HWND MainWindowHWND = GetSWindowHWND(MainWindow);
				if ( MainWindowHWND )
				{
					const LONG_PTR wndStyle = GetWindowLongPtr( MainWindowHWND, GWL_STYLE );
					SetWindowLongPtr( MainWindowHWND, GWL_STYLE, wndStyle | WS_CLIPCHILDREN );

					const FVector2D ClientSize = MainWindow->GetClientSizeInScreen();
					const FMargin Margin = MainWindow->GetWindowBorderSize();
					RECT WndRect;
					WndRect.left = Margin.Left;
					WndRect.top = Margin.Top + MainWindow->GetTitleBarSize().Get();
					WndRect.right = ClientSize.X - Margin.Right;
					WndRect.bottom = ClientSize.Y - Margin.Bottom;
					RECT ParentRect;
					ParentRect.left = 0;
					ParentRect.top = 0;
					ParentRect.right = ClientSize.X;
					ParentRect.bottom = ClientSize.Y;

					OpenUI( MainWindowHWND, &WndRect, &ParentRect );

					// Overload main window's WndProc
					OrigMainWindowWndProc = (WNDPROC)GetWindowLongPtr( MainWindowHWND, GWLP_WNDPROC );
					SetWindowLongPtr( MainWindowHWND, GWLP_WNDPROC, (LONG_PTR)MainWindowWndProc );
				}
			}
		}
	}
}


void FTrueSkyPlugin::OnMainWindowClosed(const TSharedRef<SWindow>& Window)
{
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





