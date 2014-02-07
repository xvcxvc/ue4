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
	virtual ~FTrueSkyPlugin();

	static FTrueSkyPlugin* Instance;

	/** IModuleInterface implementation */
	virtual void StartupModule() OVERRIDE;
	virtual void ShutdownModule() OVERRIDE;
	virtual bool SupportsDynamicReloading() OVERRIDE;

	/** Render delegate */
	void RenderFrame( FPostOpaqueRenderParameters& RenderParameters );

	/** Extend menu */
	void FillMenu( FMenuBuilder& MenuBuilder );

	/** Init rendering */
	void InitRenderingInterface( const TCHAR* SimulPath, const TCHAR* QtPath );

	/** Enable rendering */
	void SetRenderingEnabled( bool Enabled );

	/** Open editor */
	virtual void OpenEditor(UTrueSkySequenceAsset* const TrueSkySequence);

	struct SEditorInstance
	{
		/** Editor window */
		TSharedPtr<SWindow>		EditorWindow;
		/** Editor widow HWND */
		HWND					EditorWindowHWND;
		/** Original window message procedure */
		WNDPROC					OrigEditorWindowWndProc;
		/** Asset in editing */
		UTrueSkySequenceAsset*	Asset;
		/** ctor */
//		SEditorInstance() : EditorWindow(NULL), EditorWindowHWND(0), OrigEditorWindowWndProc(NULL), Asset(NULL) {}
	};
	TArray<SEditorInstance>	EditorInstances;

	SEditorInstance* FindEditorInstance(const TSharedRef<SWindow>& EditorWindow)
	{
		for (int i = 0; i < EditorInstances.Num(); ++i)
		{
			if ( EditorInstances[i].EditorWindow.ToSharedRef() == EditorWindow )
			{
				return &EditorInstances[i];
			}
		}
		return NULL;
	}
	SEditorInstance* FindEditorInstance(HWND const EditorWindowHWND)
	{
		for (int i = 0; i < EditorInstances.Num(); ++i)
		{
			if ( EditorInstances[i].EditorWindowHWND == EditorWindowHWND )
			{
				return &EditorInstances[i];
			}
		}
		return NULL;
	}
	SEditorInstance* FindEditorInstance(UTrueSkySequenceAsset* const Asset)
	{
		for (int i = 0; i < EditorInstances.Num(); ++i)
		{
			if ( EditorInstances[i].Asset == Asset )
			{
				return &EditorInstances[i];
			}
		}
		return NULL;
	}
	int FindEditorInstance(SEditorInstance* const Instance)
	{
		for (int i = 0; i < EditorInstances.Num(); ++i)
		{
			if ( &EditorInstances[i] == Instance )
			{
				return i;
			}
		}
		return INDEX_NONE;
	}

protected:

	void					OnMainWindowClosed(const TSharedRef<SWindow>& Window);
	TSharedRef<SDockTab>	DockTabSpawner(const FSpawnTabArgs& Args);

	/** Open menu item */
	void					OnOpen();

	/** Initializes all necessary paths */
	void					InitPaths();

	/** Creates new instance of UI */
	SEditorInstance*		CreateEditorInstance( const TCHAR* SimulPath, const TCHAR* QtPath );

	TSharedPtr< FExtender > MenuExtender;
	TSharedPtr<FAssetTypeActions_TrueSkySequence> SequenceAssetTypeActions;
	
	typedef void (* FOpenUI)(HWND, RECT*, RECT*);
	typedef void (* FCloseUI)(HWND);
	typedef void (* FSetStyleSheetPath)(const TCHAR*);
	typedef void (* FSetSequence)(HWND, std::string&);

	typedef int (* FStaticInitInterface)( const char* shaderPath, const char* texturePath );
	typedef int (* FStaticRenderFrame)( void* device, float* viewMatrix4x4, float* projMatrix4x4,  void* fullResDepthBuffer, void* halfResDepthBuffer, int viewportSizeX, int viewportSizeY );
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
	TCHAR*					SimulPath;
	TCHAR*					QtPath;

	bool					RenderingEnabled;
	bool					RendererInitialized;

	float					CachedDeltaSeconds;

	static LRESULT CALLBACK EditorWindowWndProc(HWND, ::UINT, WPARAM, LPARAM);
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


FTrueSkyPlugin* FTrueSkyPlugin::Instance = NULL;


FTrueSkyPlugin::FTrueSkyPlugin()
{
	Instance = this;
	EditorInstances.Reset();
}

FTrueSkyPlugin::~FTrueSkyPlugin()
{
	Instance = NULL;
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

	OpenUI = NULL;
	CloseUI = NULL;
	SetStyleSheetPath = NULL;
	SetSequence = NULL;

	RenderingEnabled = false;
	RendererInitialized = false;
	StaticInitInterface = NULL;
	StaticRenderFrame  = NULL;
	StaticTick  = NULL;
	StaticOnDeviceChanged = NULL;

	//we need to pass through real DeltaSecond; from our scene Actor?
	CachedDeltaSeconds = 0.0333f;

	PathEnv = NULL;
	SimulPath = NULL;
	QtPath = NULL;

	MessageId = RegisterWindowMessage(L"RESIZE");
}


void FTrueSkyPlugin::SetRenderingEnabled( bool Enabled )
{
	RenderingEnabled = Enabled;
}


void FTrueSkyPlugin::RenderFrame( FPostOpaqueRenderParameters& RenderParameters )
{	
	check(IsInRenderingThread());

	if( RenderingEnabled )
	{
		StaticTick( CachedDeltaSeconds );

		FD3D11DynamicRHI * d3d11rhi = (FD3D11DynamicRHI*)GDynamicRHI;
		ID3D11Device * device = d3d11rhi->GetDevice();
		ID3D11DeviceContext * context = d3d11rhi->GetDeviceContext();

		FMatrix mirroredViewMatrix = RenderParameters.ViewMatrix;
		mirroredViewMatrix.Mirror(EAxis::Z,EAxis::Y);

		//unreal unit is 10cm
		mirroredViewMatrix.M[3][0] *= 0.1f;
		mirroredViewMatrix.M[3][1] *= 0.1f;
		mirroredViewMatrix.M[3][2] *= 0.1f;
		mirroredViewMatrix.M[3][3] *= 0.1f;

		FD3D11TextureBase * depthTex = static_cast<FD3D11Texture2D*>(RenderParameters.DepthTexture);	
		FD3D11TextureBase * halfDepthTex = static_cast<FD3D11Texture2D*>(RenderParameters.SmallDepthTexture);		

		StaticRenderFrame( device, &(mirroredViewMatrix.M[0][0]), &(RenderParameters.ProjMatrix.M[0][0]), (void*)depthTex->GetShaderResourceView(), 
						   (void*)halfDepthTex->GetShaderResourceView(), RenderParameters.ViewportRect.Width(),
						   RenderParameters.ViewportRect.Height() );
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
	delete SimulPath;
	delete QtPath;
	PathEnv = NULL;
	SimulPath = NULL;
	QtPath = NULL;
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
#ifdef _DEBUG
	const TCHAR* const DllPath = ConstructPath( SimulPath, L"\\exe\\x64\\VC11\\Debug\\UE4PluginRenderInterface.dll" );
#else
	const TCHAR* const DllPath = ConstructPath( SimulPath, L"\\exe\\x64\\VC11\\Release\\UE4PluginRenderInterface.dll" );
#endif
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

		RendererInitialized = true;
	}
}


FTrueSkyPlugin::SEditorInstance* FTrueSkyPlugin::CreateEditorInstance( const TCHAR* SimulPath, const TCHAR* QtPath )
{
#ifdef _DEBUG
	const TCHAR* const DllPath = ConstructPath( SimulPath, L"\\exe\\x64\\VC11\\Debug\\TrueSkyUI.dll" );
#else
	const TCHAR* const DllPath = ConstructPath( SimulPath, L"\\exe\\x64\\VC11\\Release\\TrueSkyUI.dll" );
#endif
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
			IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
			TSharedPtr<SWindow> ParentWindow = MainFrameModule.GetParentWindow();
			if ( ParentWindow.IsValid() )
			{
				SEditorInstance EditorInstance;
				memset(&EditorInstance, 0, sizeof(EditorInstance));

				EditorInstance.EditorWindow = SNew(SWindow)
					.Title( FText::FromString(TEXT("TrueSky")) )
					.ClientSize( FVector2D(800.0f, 600.0f) )
					.AutoCenter( EAutoCenter::PrimaryWorkArea )
					.SizingRule( ESizingRule::UserSized )
					// .IsPopupWindow( true );
					;
				EditorInstance.EditorWindow->SetOnWindowClosed( FOnWindowClosed::CreateRaw(this, &FTrueSkyPlugin::OnMainWindowClosed) );
				FSlateApplication::Get().AddWindowAsNativeChild( EditorInstance.EditorWindow.ToSharedRef(), ParentWindow.ToSharedRef() );

				EditorInstance.EditorWindowHWND = GetSWindowHWND(EditorInstance.EditorWindow);
				if ( EditorInstance.EditorWindowHWND )
				{
					const LONG_PTR wndStyle = GetWindowLongPtr( EditorInstance.EditorWindowHWND, GWL_STYLE );
					SetWindowLongPtr( EditorInstance.EditorWindowHWND, GWL_STYLE, wndStyle | WS_CLIPCHILDREN );

					const FVector2D ClientSize = EditorInstance.EditorWindow->GetClientSizeInScreen();
					const FMargin Margin = EditorInstance.EditorWindow->GetWindowBorderSize();
					RECT ClientRect;
					ClientRect.left = Margin.Left;
					ClientRect.top = Margin.Top + EditorInstance.EditorWindow->GetTitleBarSize().Get();
					ClientRect.right = ClientSize.X - Margin.Right;
					ClientRect.bottom = ClientSize.Y - Margin.Bottom;
					RECT ParentRect;
					ParentRect.left = 0;
					ParentRect.top = 0;
					ParentRect.right = ClientSize.X;
					ParentRect.bottom = ClientSize.Y;

					OpenUI( EditorInstance.EditorWindowHWND, &ClientRect, &ParentRect );

					// Overload main window's WndProc
					EditorInstance.OrigEditorWindowWndProc = (WNDPROC)GetWindowLongPtr( EditorInstance.EditorWindowHWND, GWLP_WNDPROC );
					SetWindowLongPtr( EditorInstance.EditorWindowHWND, GWLP_WNDPROC, (LONG_PTR)EditorWindowWndProc );

					return &EditorInstances[ EditorInstances.Add(EditorInstance) ];
				}
			}
		}
	}

	return NULL;;
}


void FTrueSkyPlugin::OnMainWindowClosed(const TSharedRef<SWindow>& Window)
{
	if ( SEditorInstance* const EditorInstance = FindEditorInstance(Window) )
	{
		if ( CloseUI )
		{
			CloseUI( EditorInstance->EditorWindowHWND );
		}
		EditorInstance->EditorWindow = NULL;
		EditorInstances.RemoveAt( FindEditorInstance(EditorInstance) );
	}
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



::UINT FTrueSkyPlugin::MessageId = 0;

LRESULT CALLBACK FTrueSkyPlugin::EditorWindowWndProc(HWND hWnd, ::UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ( uMsg == WM_SIZE )
	{
		if ( HWND ChildHWND = GetWindow(hWnd, GW_CHILD) ) 
		{
			PostMessage(ChildHWND, MessageId, wParam, lParam);
		}
	}
	check( Instance );
	if ( SEditorInstance* const EditorInstance = Instance->FindEditorInstance(hWnd) )
	{
		return CallWindowProc( EditorInstance->OrigEditorWindowWndProc, hWnd, uMsg, wParam, lParam );
	}
	return 0;
}


void FTrueSkyPlugin::InitPaths()
{
	if ( SimulPath == NULL )
	{
		SimulPath = GetEnvVariable(L"SIMUL");
	}
	if ( QtPath == NULL )
	{
		QtPath = GetEnvVariable(L"QTDIR");
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
#ifdef _DEBUG
			wcscat_s( PathEnv, iPathSize, SimulPath ); wcscat_s( PathEnv, iPathSize, L"\\exe\\x64\\VC11\\Debug" );
#else
			wcscat_s( PathEnv, iPathSize, SimulPath ); wcscat_s( PathEnv, iPathSize, L"\\exe\\x64\\VC11\\Release" );
#endif
		}
		else
		{
			wcscpy_s( PathEnv, iPathSize, QtPath ); wcscat_s( PathEnv, iPathSize, L"\\bin;" );
			wcscat_s( PathEnv, iPathSize, SimulPath ); wcscat_s( PathEnv, iPathSize, L"\\exe\\x64;" );
#ifdef _DEBUG
			wcscat_s( PathEnv, iPathSize, SimulPath ); wcscat_s( PathEnv, iPathSize, L"\\exe\\x64\\VC11\\Debug" );
#else
			wcscat_s( PathEnv, iPathSize, SimulPath ); wcscat_s( PathEnv, iPathSize, L"\\exe\\x64\\VC11\\Release" );
#endif
		}

		SetEnvironmentVariable( L"PATH", PathEnv );
	}
}


void FTrueSkyPlugin::OnOpen()
{
	InitPaths();
	if( RendererInitialized )
	{
		SetRenderingEnabled(!RenderingEnabled);
	}
	else
	{
		InitRenderingInterface( SimulPath, QtPath );
		SetRenderingEnabled(true);
	}
}


void FTrueSkyPlugin::OpenEditor(UTrueSkySequenceAsset* const TrueSkySequence)
{
	if ( TrueSkySequence == NULL )
		return;

	// TrueSky environment reads from std::string
	std::string SequenceInputText;
	if ( TrueSkySequence && TrueSkySequence->SequenceText.Num() > 0 )
	{
		SequenceInputText = std::string( (const char*)TrueSkySequence->SequenceText.GetData() );
	}

	SEditorInstance* EditorInstance = FindEditorInstance(TrueSkySequence);
	if ( EditorInstance == NULL )
	{
		InitPaths();
		EditorInstance = CreateEditorInstance( SimulPath, QtPath );
		if ( EditorInstance )
		{
			EditorInstance->Asset = TrueSkySequence;
		}
		InitRenderingInterface( SimulPath, QtPath );
	}

	// Set sequence asset to UI
	if ( EditorInstance && SetSequence )
	{
		SetSequence( EditorInstance->EditorWindowHWND, SequenceInputText );
	}
}





