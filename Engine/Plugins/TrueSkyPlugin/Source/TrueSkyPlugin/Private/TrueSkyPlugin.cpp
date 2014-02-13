// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#include "TrueSkyPluginPrivatePCH.h"
#include "PlacementModePrivatePCH.h"
#include "TrueSkySequenceAsset.h"
#include "TrueSkySequenceFactory.h"
#include "LevelEditor.h"
#include "IMainFrameModule.h"
#include "SlateStyle.h"
#include "GenericWindow.h"
#include "WindowsWindow.h"
#include "RendererInterface.h"
#include "DynamicRHI.h"
#include "D3D11RHIPrivate.h"
#include "Tickable.h"
#include "AssetToolsModule.h"
#include "AssetTypeActions_TrueSkySequence.h"
#include "TrueSkyPlugin.generated.inl"
#include "AssetPaletteFactoryFilter.h"
#include "Editor.h"
#include <string>

class FTrueSkyPlugin : public ITrueSkyPlugin, FTickableGameObject
{
public:
	FTrueSkyPlugin();
	virtual ~FTrueSkyPlugin();

	static FTrueSkyPlugin*	Instance;

	/** IModuleInterface implementation */
	virtual void			StartupModule() OVERRIDE;
	virtual void			ShutdownModule() OVERRIDE;
	virtual bool			SupportsDynamicReloading() OVERRIDE;

	/** Render delegate */
	void					RenderFrame( FPostOpaqueRenderParameters& RenderParameters );

	/** Extend menu */
	void					FillMenu( FMenuBuilder& MenuBuilder );

	/** Init rendering */
	void					InitRenderingInterface( const TCHAR* SimulPath, const TCHAR* QtPath );

	/** Enable rendering */
	void					SetRenderingEnabled( bool Enabled );

	/** Open editor */
	virtual void			OpenEditor(UTrueSkySequenceAsset* const TrueSkySequence);

	/** If there is a TrueSkySequenceActor in the persistent level, this returns that actor's TrueSkySequenceAsset */
	UTrueSkySequenceAsset*	GetActiveSequence();

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
		/** Saves current sequence data into Asset */
		void					SaveSequenceData();
		/** Loads current Asset data into Environment */
		void					LoadSequenceData();
	};

	SEditorInstance*		FindEditorInstance(const TSharedRef<SWindow>& EditorWindow);
	SEditorInstance*		FindEditorInstance(HWND const EditorWindowHWND);
	SEditorInstance*		FindEditorInstance(UTrueSkySequenceAsset* const Asset);
	int						FindEditorInstance(SEditorInstance* const Instance);

protected:

	void					OnMainWindowClosed(const TSharedRef<SWindow>& Window);

	/** Called when Toggle rendering button is pressed */
	void					OnToggleRendering();
	/** Returns true if Toggle rendering button should be enabled */
	bool					IsToggleRenderingEnabled();
	/** Returns true if Toggle rendering button should be checked */
	bool					IsToggleRenderingChecked();

	/** Adds a TrueSkySequenceActor to the current scene */
	void					OnAddSequence();
	/** Returns true if user can add a sequence actor */
	bool					IsAddSequenceEnabled();

	/** Initializes all necessary paths */
	void					InitPaths();

	/** Tick interface */
	void					Tick( float DeltaTime );
	bool					IsTickable() const;
	TStatId					GetStatId() const;

	/** Creates new instance of UI */
	SEditorInstance*		CreateEditorInstance( const TCHAR* SimulPath, const TCHAR* QtPath, void* Env );

	TSharedPtr< FExtender > MenuExtender;
	TSharedPtr<FAssetTypeActions_TrueSkySequence> SequenceAssetTypeActions;

	typedef void (* FOpenUI)(HWND, RECT*, RECT*, void*);
	typedef void (* FCloseUI)(HWND);
	typedef void (* FSetStyleSheetPath)(const TCHAR*);
	typedef void (* FSetSequence)(HWND, const char*);
	typedef char* (*FAlloc)(int size);
	typedef char* (* FGetSequence)(HWND, FAlloc);
	typedef void (* FOnSequenceChangeCallback)(HWND);
	typedef void (* FSetOnPropertiesChangedCallback)(HWND, FOnSequenceChangeCallback);

	typedef int (* FStaticInitInterface)( const char* shaderPath, const char* texturePath );
	typedef int (* FStaticRenderFrame)( void* device, float* viewMatrix4x4, float* projMatrix4x4,  void* fullResDepthBuffer, void* halfResDepthBuffer, int viewportSizeX, int viewportSizeY );
	typedef int (* FStaticTick)( float deltaTime );
	typedef int (* FStaticOnDeviceChanged)( void * device );
	typedef void* (* FStaticGetEnvironment)();
	typedef int (* FStaticSetSequence)( std::string sequenceInputText );

	FOpenUI					OpenUI;
	FCloseUI				CloseUI;
	FSetStyleSheetPath		SetStyleSheetPath;
	FSetSequence			SetSequence;
	FGetSequence			GetSequence;
	FSetOnPropertiesChangedCallback SetOnPropertiesChangedCallback;

	FStaticInitInterface	StaticInitInterface;
	FStaticRenderFrame		StaticRenderFrame;
	FStaticTick				StaticTick;
	FStaticOnDeviceChanged	StaticOnDeviceChanged;
	FStaticGetEnvironment	StaticGetEnvironment;
	FStaticSetSequence		StaticSetSequence;

	TCHAR*					PathEnv;
	TCHAR*					SimulPath;
	TCHAR*					QtPath;

	bool					RenderingEnabled;
	bool					RendererInitialized;

	float					CachedDeltaSeconds;

	TArray<SEditorInstance>	EditorInstances;

	static LRESULT CALLBACK EditorWindowWndProc(HWND, ::UINT, WPARAM, LPARAM);
	static ::UINT			MessageId;

	static void				OnSequenceChangeCallback(HWND OwnerHWND);
	static char*			AllocString(int size);
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
		UI_COMMAND(ToggleRendering, "Toggle Rendering", "Toggles TrueSky plugin rendering.", EUserInterfaceActionType::ToggleButton, FInputGesture());
		UI_COMMAND(AddSequence, "Add Sequence To Scene", "Adds a TrueSkySequenceActor to the current scene", EUserInterfaceActionType::Button, FInputGesture())
	}
public:
	TSharedPtr<FUICommandInfo> ToggleRendering;
	TSharedPtr<FUICommandInfo> AddSequence;
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

/** Tickable object interface */
void FTrueSkyPlugin::Tick( float DeltaTime )
{
	CachedDeltaSeconds = DeltaTime;
}

bool FTrueSkyPlugin::IsTickable() const
{
	return true;
}

TStatId FTrueSkyPlugin::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FTrueSkyPlugin, STATGROUP_Tickables);
}


void FTrueSkyPlugin::StartupModule()
{
	FTrueSkyCommands::Register();
	IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
	const TSharedRef<FUICommandList>& CommandList = MainFrameModule.GetMainFrameCommandBindings();
//	CommandList->MapAction( FTrueSkyCommands::Get().ToggleRendering, FExecuteAction::CreateRaw(this, &FTrueSkyPlugin::OnToggleRendering) );
	CommandList->MapAction( FTrueSkyCommands::Get().ToggleRendering,
							FExecuteAction::CreateRaw(this, &FTrueSkyPlugin::OnToggleRendering),
							FCanExecuteAction::CreateRaw(this, &FTrueSkyPlugin::IsToggleRenderingEnabled),
							FIsActionChecked::CreateRaw(this, &FTrueSkyPlugin::IsToggleRenderingChecked)
							);
	CommandList->MapAction( FTrueSkyCommands::Get().AddSequence,
							FExecuteAction::CreateRaw(this, &FTrueSkyPlugin::OnAddSequence),
							FCanExecuteAction::CreateRaw(this, &FTrueSkyPlugin::IsAddSequenceEnabled)
							);

	MenuExtender = MakeShareable(new FExtender);
	MenuExtender->AddMenuExtension("WindowGlobalTabSpawners", EExtensionHook::After, CommandList, FMenuExtensionDelegate::CreateRaw(this, &FTrueSkyPlugin::FillMenu));
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>( "LevelEditor" );
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

	SequenceAssetTypeActions = MakeShareable(new FAssetTypeActions_TrueSkySequence);
	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().RegisterAssetTypeActions(SequenceAssetTypeActions.ToSharedRef());

	GetRendererModule().RegisterPostOpaqueRenderDelegate( FPostOpaqueRenderDelegate::CreateRaw(this, &FTrueSkyPlugin::RenderFrame) );

	const FName IconName(TEXT("../../Plugins/TrueSkyPlugin/Resources/icon_64x.png"));
	//	check( FPaths::FileExists( IconName.ToString() ) );
	FSlateStyleSet& SlateStyleSet = (FSlateStyleSet&)FEditorStyle::GetInstance();
	SlateStyleSet.Set( TEXT("ClassThumbnail.TrueSkySequenceAsset"), new FSlateImageBrush(IconName, FVector2D(64.0f, 64.0f)) );

	OpenUI = NULL;
	CloseUI = NULL;
	SetStyleSheetPath = NULL;
	SetSequence = NULL;
	GetSequence = NULL;
	SetOnPropertiesChangedCallback = NULL;

	RenderingEnabled = false;
	RendererInitialized = false;
	StaticInitInterface = NULL;
	StaticRenderFrame  = NULL;
	StaticTick  = NULL;
	StaticOnDeviceChanged = NULL;
	StaticGetEnvironment = NULL;

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
		MenuBuilder.AddMenuEntry( FTrueSkyCommands::Get().ToggleRendering );
		MenuBuilder.AddMenuEntry( FTrueSkyCommands::Get().AddSequence );
	}
	MenuBuilder.EndSection();
}


/** Returns environment variable value */
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


/** Returns HWND for a given SWindow (if native!) */
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
		StaticGetEnvironment = (FStaticGetEnvironment)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticGetEnvironment"));
		StaticSetSequence = (FStaticSetSequence)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticSetSequence"));

		if( StaticInitInterface == NULL || StaticRenderFrame == NULL || 
			StaticOnDeviceChanged == NULL || StaticTick == NULL  || 
			StaticGetEnvironment == NULL || StaticSetSequence == NULL )
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


FTrueSkyPlugin::SEditorInstance* FTrueSkyPlugin::CreateEditorInstance( const TCHAR* SimulPath, const TCHAR* QtPath, void* Env )
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
		GetSequence = (FGetSequence)FPlatformProcess::GetDllExport(DllHandle, TEXT("GetSequence") );
		SetOnPropertiesChangedCallback = (FSetOnPropertiesChangedCallback)FPlatformProcess::GetDllExport(DllHandle, TEXT("SetOnPropertiesChangedCallback") );

		checkf( OpenUI, L"OpenUI function not found!" );
		checkf( CloseUI, L"CloseUI function not found!" );
		checkf( SetStyleSheetPath, L"SetStyleSheetPath function not found!" );
		checkf( SetSequence, L"SetSequence function not found!" );
		checkf( GetSequence, L"GetSequence function not found!" );
		checkf( SetOnPropertiesChangedCallback, L"SetOnPropertiesChangedCallback function not found!" );

		const TCHAR* StyleSheetPath = ConstructPath( SimulPath, L"\\PlugIns\\UE4\\TrueSkyUI\\qss\\" );
		SetStyleSheetPath( StyleSheetPath );
		delete StyleSheetPath;

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

				OpenUI( EditorInstance.EditorWindowHWND, &ClientRect, &ParentRect, Env );

				// Overload main window's WndProc
				EditorInstance.OrigEditorWindowWndProc = (WNDPROC)GetWindowLongPtr( EditorInstance.EditorWindowHWND, GWLP_WNDPROC );
				SetWindowLongPtr( EditorInstance.EditorWindowHWND, GWLP_WNDPROC, (LONG_PTR)EditorWindowWndProc );

				// Setup notification callback
				SetOnPropertiesChangedCallback( EditorInstance.EditorWindowHWND, OnSequenceChangeCallback );

				return &EditorInstances[ EditorInstances.Add(EditorInstance) ];
			}
		}
	}

	return NULL;
}


FTrueSkyPlugin::SEditorInstance* FTrueSkyPlugin::FindEditorInstance(const TSharedRef<SWindow>& EditorWindow)
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

FTrueSkyPlugin::SEditorInstance* FTrueSkyPlugin::FindEditorInstance(HWND const EditorWindowHWND)
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

FTrueSkyPlugin::SEditorInstance* FTrueSkyPlugin::FindEditorInstance(UTrueSkySequenceAsset* const Asset)
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

int FTrueSkyPlugin::FindEditorInstance(FTrueSkyPlugin::SEditorInstance* const Instance)
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


void FTrueSkyPlugin::SEditorInstance::SaveSequenceData()
{
	if ( Asset )
	{
		check( FTrueSkyPlugin::Instance );
		check( FTrueSkyPlugin::Instance->GetSequence );
		char* const OutputText = FTrueSkyPlugin::Instance->GetSequence( EditorWindowHWND, FTrueSkyPlugin::AllocString );
		if ( OutputText )
		{
			const int OutputTextLen = strlen( OutputText );
			Asset->SequenceText.Reset( OutputTextLen + 1 );

			for (char* ptr = OutputText; *ptr; ++ptr)
			{
				Asset->SequenceText.Add( *ptr );
			}
			Asset->SequenceText.Add( 0 );

			// Mark as dirty
			Asset->Modify( true );

			delete OutputText;
		}
	}
}

void FTrueSkyPlugin::SEditorInstance::LoadSequenceData()
{
	if ( Asset && Asset->SequenceText.Num() > 0 )
	{
		check( FTrueSkyPlugin::Instance );
		check( FTrueSkyPlugin::Instance->SetSequence );
		FTrueSkyPlugin::Instance->SetSequence( EditorWindowHWND, (const char*)Asset->SequenceText.GetData() );
	}
}


void FTrueSkyPlugin::OnMainWindowClosed(const TSharedRef<SWindow>& Window)
{
	if ( SEditorInstance* const EditorInstance = FindEditorInstance(Window) )
	{
		EditorInstance->SaveSequenceData();

		check( CloseUI );
		CloseUI( EditorInstance->EditorWindowHWND );

		EditorInstance->EditorWindow = NULL;
		EditorInstances.RemoveAt( FindEditorInstance(EditorInstance) );
	}
}


/** Called when TrueSkyUI properties have changed */
void FTrueSkyPlugin::OnSequenceChangeCallback(HWND OwnerHWND)
{
	check( Instance );
	if ( SEditorInstance* const EditorInstance = Instance->FindEditorInstance(OwnerHWND) )
	{
		EditorInstance->SaveSequenceData();
	}
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


void FTrueSkyPlugin::OnToggleRendering()
{
	if ( UTrueSkySequenceAsset* const ActiveSequence = GetActiveSequence() )
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

		if ( ActiveSequence->SequenceText.Num() > 0 )
		{
			std::string SequenceInputText;
			SequenceInputText = std::string( (const char*)ActiveSequence->SequenceText.GetData() );
			StaticSetSequence( SequenceInputText );
		}
	}
	else if(RenderingEnabled)
	{
		// no active sequence, so disable rendering
		SetRenderingEnabled(false);
	}
}

bool FTrueSkyPlugin::IsToggleRenderingEnabled()
{
	if ( GetActiveSequence() )
	{
		return true;
	}
	// No active sequence found!
	SetRenderingEnabled( false );
	return false;
}

bool FTrueSkyPlugin::IsToggleRenderingChecked()
{
	return RenderingEnabled;
}


void FTrueSkyPlugin::OnAddSequence()
{
	ULevel* const Level = GWorld->PersistentLevel;
	ATrueSkySequenceActor* SequenceActor = NULL;
	// Check for existing sequence actor
	for(int i = 0; i < Level->Actors.Num() && SequenceActor == NULL; i++)
	{
		SequenceActor = Cast<ATrueSkySequenceActor>( Level->Actors[i] );
	}
	if ( SequenceActor == NULL )
	{
		// Add sequence actor
		GWorld->SpawnActor<ATrueSkySequenceActor>(ATrueSkySequenceActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	}
	else
	{
		// Sequence actor already exists -- error message?
	}
}

bool FTrueSkyPlugin::IsAddSequenceEnabled()
{
	// Returns false if TrueSkySequenceActor already exists!
	ULevel* const Level = GWorld->PersistentLevel;
	for(int i = 0; i < Level->Actors.Num(); i++)
	{
		if ( Cast<ATrueSkySequenceActor>(Level->Actors[i]) )
			return false;
	}
	return true;
}


UTrueSkySequenceAsset* FTrueSkyPlugin::GetActiveSequence()
{
	ULevel* const Level = GWorld->PersistentLevel;
	for(int i = 0; i < Level->Actors.Num(); i++)
	{
		if ( ATrueSkySequenceActor* SequenceActor = Cast<ATrueSkySequenceActor>(Level->Actors[i]) )
		{
			return SequenceActor->ActiveSequence;
		}
	}
	return NULL;
}


void FTrueSkyPlugin::OpenEditor(UTrueSkySequenceAsset* const TrueSkySequence)
{
	if ( TrueSkySequence == NULL )
		return;

	SEditorInstance* EditorInstance = FindEditorInstance(TrueSkySequence);
	if ( EditorInstance == NULL )
	{
		InitPaths();

		InitRenderingInterface( SimulPath, QtPath );

		check( StaticGetEnvironment );
		void* const Env = GetActiveSequence() == TrueSkySequence ? StaticGetEnvironment() : NULL;

		EditorInstance = CreateEditorInstance( SimulPath, QtPath, Env );
		if ( EditorInstance )
		{
			EditorInstance->Asset = TrueSkySequence;
		}
	}

	// Set sequence asset to UI
	if ( EditorInstance )
	{
		EditorInstance->LoadSequenceData();
	}
}


char* FTrueSkyPlugin::AllocString(int size)
{
	check( size );
	return new char[size];
}



