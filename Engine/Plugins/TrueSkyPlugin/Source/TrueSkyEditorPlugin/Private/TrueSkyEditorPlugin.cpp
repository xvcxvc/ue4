#define TRUESKY_LICENCE_KEY "Enter LicenceKey"
// Replace the string above with your own licence key!

// Copyright 2013-2014 Simul Software Ltd. All Rights Reserved.

#include "TrueSkyEditorPluginPrivatePCH.h"
//#include "PlacementModePrivatePCH.h"
#include "TrueSkySequenceAsset.h"
#include "TrueSkySequenceActor.h"
#if UE_EDITOR
#include "Editor.h"
//#include "TrueSkySequenceFactory.h"
#include "LevelEditor.h"
#include "IMainFrameModule.h"
#include "IDocumentation.h"
#include "AssetToolsModule.h"
#endif
//#include "SlateStyle.h"
#include "GenericWindow.h"
#include "WindowsWindow.h"
#include "RendererInterface.h"
#include "DynamicRHI.h"
#include "UnrealClient.h"

DEFINE_LOG_CATEGORY_STATIC(TrueSky, Log, All);

#include "D3D11RHI.h"
// Dependencies.
#include "Core.h"
#include "RHI.h"
#include "GPUProfiler.h"
#include "ShaderCore.h"
#include "Engine.h"
#ifdef UE_EDITOR
DECLARE_LOG_CATEGORY_EXTERN(LogD3D11RHI, Log, All);
#endif
#include "../Private/Windows/D3D11RHIBasePrivate.h"
#include "StaticArray.h"
#include "ActorCrossThreadProperties.h"

ActorCrossThreadProperties actorCrossThreadProperties;
extern ActorCrossThreadProperties *GetActorCrossThreadProperties()
{
	return &actorCrossThreadProperties;
}

/** This is a macro that casts a dynamically bound RHI reference to the appropriate D3D type. */
#define DYNAMIC_CAST_D3D11RESOURCE(Type,Name) \
	FD3D11##Type* Name = (FD3D11##Type*)Name##RHI;

// D3D RHI public headers.
#include "D3D11Util.h"
#include "D3D11State.h"
#include "D3D11Resources.h"
#include "D3D11Viewport.h"
#include "D3D11ConstantBuffer.h"
//#include "../Private/Windows/D3D11StateCache.h"
#include "../Private/D3D11StateCachePrivate.h"

typedef FD3D11StateCacheBase FD3D11StateCache;



#include "D3D11Resources.h"
#include "Tickable.h"
#include "AssetTypeActions_TrueSkySequence.h"
//#include "TrueSkyPlugin.generated.inl"
#include "EngineModule.h"
#include <string>

#define ENABLE_AUTO_SAVING

#pragma optimize("",off)

#define DECLARE_TOGGLE(name)\
	void					OnToggle##name();\
	bool					IsToggled##name();

#define IMPLEMENT_TOGGLE(name)\
	void FTrueSkyEditorPlugin::OnToggle##name()\
{\
	ITrueSkyPlugin &trueSkyPlugin=ITrueSkyPlugin::Get();\
	bool c=trueSkyPlugin.GetRenderBool(#name);\
	trueSkyPlugin.SetRenderBool(#name,!c);\
}\
\
bool FTrueSkyEditorPlugin::IsToggled##name()\
{\
	ITrueSkyPlugin &trueSkyPlugin=ITrueSkyPlugin::Get();\
	return trueSkyPlugin.GetRenderBool(#name);\
}

#define DECLARE_ACTION(name)\
	void					OnTrigger##name()\
	{\
		ITrueSkyPlugin &trueSkyPlugin=ITrueSkyPlugin::Get();\
		return trueSkyPlugin.TriggerAction(#name);\
	}


class FTrueSkyTickable : public  FTickableGameObject
{
public:
	/** Tick interface */
	void					Tick( float DeltaTime );
	bool					IsTickable() const;
	TStatId					GetStatId() const;
};

class FTrueSkyEditorPlugin : public ITrueSkyEditorPlugin
#ifdef SHARED_FROM_THIS
	, public TSharedFromThis<FTrueSkyEditorPlugin,(ESPMode::Type)0>
#endif
{
public:
	FTrueSkyEditorPlugin();
	virtual ~FTrueSkyEditorPlugin();

	static FTrueSkyEditorPlugin*	Instance;
	
	void					OnDebugTrueSky(class UCanvas* Canvas, APlayerController*);

	/** IModuleInterface implementation */
	virtual void			StartupModule() override;
	virtual void			ShutdownModule() override;
	virtual bool			SupportsDynamicReloading() override;
	
#if UE_EDITOR
	/** TrueSKY menu */
	void					FillMenu( FMenuBuilder& MenuBuilder );
	/** Overlay sub-menu */
	void					FillOverlayMenu(FMenuBuilder& MenuBuilder);
#endif
	
	/** Open editor */
	virtual void			OpenEditor(UTrueSkySequenceAsset* const TrueSkySequence);

#if UE_EDITOR
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
	/** Saves all Environments */
	void					SaveAllEditorInstances();
#endif
	void					Tick( float DeltaTime );

#if UE_EDITOR
	virtual void			SetUIString(SEditorInstance* const EditorInstance,const char* name, const char*  value);
	virtual const char*		GetUIString(SEditorInstance* const EditorInstance,const char* name) const;
#endif
	virtual void			SetCloudShadowRenderTarget(FRenderTarget *t);
protected:
	
	void					OnMainWindowClosed(const TSharedRef<SWindow>& Window);

	/** Called when Toggle rendering button is pressed */
	void					OnToggleRendering();
	/** Returns true if Toggle rendering button should be enabled */
	bool					IsToggleRenderingEnabled();
	/** Returns true if Toggle rendering button should be checked */
	bool					IsToggleRenderingChecked();
	/** Always returns true */
	bool					IsMenuEnabled();

	/** Called when the Toggle Fades Overlay button is pressed*/
	DECLARE_TOGGLE(ShowFades)
	DECLARE_TOGGLE(ShowCompositing)
	DECLARE_TOGGLE(Show3DCloudTextures)
	DECLARE_TOGGLE(Show2DCloudTextures)
	DECLARE_ACTION(RecompileShaders)
	void ShowDocumentation()
	{
	//	FString DocumentationUrl = FDocumentationLink::ToUrl(Link);
		FString DocumentationUrl="https://www.simul.co/wp-content/uploads/documentation/ue4/TrueSkyUE4.html";
		FPlatformProcess::LaunchURL(*DocumentationUrl, NULL, NULL);
	}
	
	/** Adds a TrueSkySequenceActor to the current scene */
	void					OnAddSequence();
	void					OnSequenceDestroyed();
	/** Returns true if user can add a sequence actor */
	bool					IsAddSequenceEnabled();

	/** Initializes all necessary paths */
	void					InitPaths();
	
#if UE_EDITOR
	/** Creates new instance of UI */
	SEditorInstance*		CreateEditorInstance(  void* Env );
	TSharedPtr< FExtender > MenuExtender;
	TSharedPtr<FAssetTypeActions_TrueSkySequence> SequenceAssetTypeActions;
#endif
	typedef void (*FOpenUI)(HWND, RECT*, RECT*, void*,PluginStyle style);
	typedef void (*FCloseUI)(HWND);
	
	typedef void (*FStaticSetUIString)(HWND,const char* name,const char* value );
	typedef const char * (*FStaticGetUIString)(HWND,const char* name);

	typedef void (*FPushStyleSheetPath)(const char*);
	typedef void (*FSetSequence)(HWND, const char*);
	typedef char* (*FAlloc)(int size);
	typedef char* (*FGetSequence)(HWND, FAlloc);
	typedef void (*FOnTimeChangedCallback)(HWND,float);
	typedef void (*FSetOnTimeChangedInUICallback)(FOnTimeChangedCallback);
	typedef void (*FOnSequenceChangeCallback)(HWND,const char *);
	typedef void (*FSetOnPropertiesChangedCallback)( FOnSequenceChangeCallback);

	typedef int (*FStaticInitInterface)(  );
	typedef int (*FStaticPushPath)(const char*,const char*);
	typedef int (*FStaticGetOrAddView)( void *);
	typedef int (*FStaticRenderFrame)(void* device, int view_id
		,float* viewMatrix4x4
		, float* projMatrix4x4
		, ID3D11Texture2D* depthTexture
		,ID3D11ShaderResourceView* depthResource
		,const Viewport *v
		,PluginStyle s);

	typedef int (*FStaticTick)( float deltaTime );
	typedef int (*FStaticOnDeviceChanged)( void * device );
	typedef int (*FStaticSetSequence)( std::string sequenceInputText );
	typedef class UE4PluginRenderInterface* (*FStaticGetRenderInterfaceInstance)();
	typedef void (*FStaticTriggerAction)( const char* name );
	
	FOpenUI								OpenUI;
	FCloseUI							CloseUI;
	FStaticSetUIString					StaticSetUIString;
	FStaticGetUIString					StaticGetUIString;
	FPushStyleSheetPath					PushStyleSheetPath;
	FSetSequence						SetSequence;
	FGetSequence						GetSequence;
	FSetOnPropertiesChangedCallback		SetOnPropertiesChangedCallback;
	FSetOnTimeChangedInUICallback		SetOnTimeChangedInUICallback;
	FStaticInitInterface				StaticInitInterface;
	FStaticPushPath						StaticPushPath;
	FStaticGetOrAddView					StaticGetOrAddView;
	FStaticRenderFrame					StaticRenderFrame;
	FStaticTick							StaticTick;
	FStaticOnDeviceChanged				StaticOnDeviceChanged;
	FStaticSetSequence					StaticSetSequence;
	FStaticTriggerAction				StaticTriggerAction;

	TCHAR*					PathEnv;

	bool					RenderingEnabled;
	bool					RendererInitialized;

	float					CachedDeltaSeconds;
	float					AutoSaveTimer;		// 0.0f == no auto-saving
	
#if UE_EDITOR
	TArray<SEditorInstance>	EditorInstances;

	static LRESULT CALLBACK EditorWindowWndProc(HWND, ::UINT, WPARAM, LPARAM);
	static ::UINT			MessageId;
#endif
	static void				OnSequenceChangeCallback(HWND OwnerHWND,const char *);
	
	static void				OnTimeChangedCallback(HWND OwnerHWND,float t);
	
	FRenderTarget			*cloudShadowRenderTarget;

	bool					haveEditor;
};

IMPLEMENT_MODULE( FTrueSkyEditorPlugin, TrueSkyPlugin )


#if UE_EDITOR
class FTrueSkyCommands : public TCommands<FTrueSkyCommands>
{
public:
	//	TCommands( const FName InContextName, const FText& InContextDesc, const FName InContextParent, const FName InStyleSetName )
	FTrueSkyCommands()
		: TCommands<FTrueSkyCommands>(
		TEXT("TrueSky"), // Context name for fast lookup
		NSLOCTEXT("Contexts", "TrueSkyCmd", "Simul TrueSky"), // Localized context name for displaying
		NAME_None, // Parent context name. 
		FEditorStyle::GetStyleSetName()) // Parent
	{
	}
	virtual void RegisterCommands() override
	{
		UI_COMMAND(AddSequence				,"Add Sequence To Scene","Adds a TrueSkySequenceActor to the current scene", EUserInterfaceActionType::Button, FInputGesture());
		UI_COMMAND(ToggleFades				,"Atmospheric Tables"	,"Toggles the atmospheric tables overlay.", EUserInterfaceActionType::ToggleButton, FInputGesture());
		UI_COMMAND(ToggleShowCompositing	,"Compositing"			,"Toggles the compositing overlay.", EUserInterfaceActionType::ToggleButton, FInputGesture());
		UI_COMMAND(ToggleShow3DCloudTextures,"Show 3D Cloud Textures","Toggles the 3D cloud overlay.", EUserInterfaceActionType::ToggleButton, FInputGesture());
		UI_COMMAND(ToggleShow2DCloudTextures,"Show 2D Cloud Textures","Toggles the 2D cloud overlay.", EUserInterfaceActionType::ToggleButton, FInputGesture());
		UI_COMMAND(TriggerRecompileShaders	,"Recompile Shaders"	,"Recompiles the shaders.", EUserInterfaceActionType::Button, FInputGesture());
		UI_COMMAND(TriggerShowDocumentation	,"trueSKY Documentation..."	,"Shows the trueSKY help pages.", EUserInterfaceActionType::Button, FInputGesture());
	}
public:
	TSharedPtr<FUICommandInfo> AddSequence;
	TSharedPtr<FUICommandInfo> ToggleFades;
	TSharedPtr<FUICommandInfo> ToggleShowCompositing;
	TSharedPtr<FUICommandInfo> ToggleShow3DCloudTextures;
	TSharedPtr<FUICommandInfo> ToggleShow2DCloudTextures;
	TSharedPtr<FUICommandInfo> TriggerRecompileShaders;
	TSharedPtr<FUICommandInfo> TriggerShowDocumentation;
};
#endif
FTrueSkyEditorPlugin* FTrueSkyEditorPlugin::Instance = NULL;

//TSharedRef<FTrueSkyEditorPlugin> staticSharedRef;
static std::string trueSkyPluginPath="../../Plugins/TrueSkyPlugin";
FTrueSkyEditorPlugin::FTrueSkyEditorPlugin()
	:cloudShadowRenderTarget(NULL)
	,haveEditor(false)
{
	Instance = this;
#ifdef SHARED_FROM_THIS
	//TSharedRef<FTrueSkyEditorPlugin> sharedRef=AsShared();
TSharedRef< FTrueSkyEditorPlugin,(ESPMode::Type)0 > ref=AsShared();
#endif
#if UE_EDITOR
	EditorInstances.Reset();
#endif
	AutoSaveTimer = 0.0f;
	//we need to pass through real DeltaSecond; from our scene Actor?
	CachedDeltaSeconds = 0.0333f;

}

FTrueSkyEditorPlugin::~FTrueSkyEditorPlugin()
{
	Instance = nullptr;
}


bool FTrueSkyEditorPlugin::SupportsDynamicReloading()
{
	return false;
}

void FTrueSkyEditorPlugin::Tick( float DeltaTime )
{
	if(IsInGameThread())
	{

	}
	CachedDeltaSeconds = DeltaTime;
#ifdef ENABLE_AUTO_SAVING
#if UE_EDITOR
	if ( AutoSaveTimer > 0.0f )
	{
		if ( (AutoSaveTimer -= DeltaTime) <= 0.0f )
		{
			SaveAllEditorInstances();
			AutoSaveTimer = 4.0f;
		}
	}
#endif
#endif
}

void FTrueSkyEditorPlugin::SetCloudShadowRenderTarget(FRenderTarget *t)
{
	cloudShadowRenderTarget=t;
}

#if UE_EDITOR
void FTrueSkyEditorPlugin::SetUIString(SEditorInstance* const EditorInstance,const char* name, const char*  value)
{
	if( StaticSetUIString != NULL&&EditorInstance!=NULL)
	{
		StaticSetUIString(EditorInstance->EditorWindowHWND,name, value );
	}
	else
	{
		UE_LOG(TrueSky, Warning, TEXT("Trying to set UI string before StaticSetUIString has been set"), TEXT(""));
	}
}

const char*	 FTrueSkyEditorPlugin::GetUIString(SEditorInstance* const EditorInstance,const char* name) const
{
	if( StaticSetUIString != NULL&&EditorInstance!=NULL)
	{
		return StaticGetUIString(EditorInstance->EditorWindowHWND,name);
	}

	UE_LOG(TrueSky, Warning, TEXT("Trying to get UI string before StaticSetUIString has been set"), TEXT(""));
	return NULL;
}
#endif
/** Tickable object interface */
void FTrueSkyTickable::Tick( float DeltaTime )
{
	if(FTrueSkyEditorPlugin::Instance)
		FTrueSkyEditorPlugin::Instance->Tick(DeltaTime);
}

bool FTrueSkyTickable::IsTickable() const
{
	return true;
}

TStatId FTrueSkyTickable::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FTrueSkyTickable, STATGROUP_Tickables);
}


void FTrueSkyEditorPlugin::StartupModule()
{
#if UE_EDITOR
	FTrueSkyCommands::Register();
	if(FModuleManager::Get().IsModuleLoaded("MainFrame") )
	{
		haveEditor=true;
		IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
		const TSharedRef<FUICommandList>& CommandList = MainFrameModule.GetMainFrameCommandBindings();
		CommandList->MapAction( FTrueSkyCommands::Get().AddSequence,
								FExecuteAction::CreateRaw(this, &FTrueSkyEditorPlugin::OnAddSequence),
								FCanExecuteAction::CreateRaw(this, &FTrueSkyEditorPlugin::IsAddSequenceEnabled)
								);
			CommandList->MapAction( FTrueSkyCommands::Get().TriggerRecompileShaders,
									FExecuteAction::CreateRaw(this, &FTrueSkyEditorPlugin::OnTriggerRecompileShaders),
									FCanExecuteAction::CreateRaw(this, &FTrueSkyEditorPlugin::IsMenuEnabled)
									);
			CommandList->MapAction( FTrueSkyCommands::Get().TriggerShowDocumentation,
									FExecuteAction::CreateRaw(this, &FTrueSkyEditorPlugin::ShowDocumentation)
									);
		{
			CommandList->MapAction( FTrueSkyCommands::Get().ToggleFades,
									FExecuteAction::CreateRaw(this, &FTrueSkyEditorPlugin::OnToggleShowFades),
									FCanExecuteAction::CreateRaw(this, &FTrueSkyEditorPlugin::IsMenuEnabled),
									FIsActionChecked::CreateRaw(this, &FTrueSkyEditorPlugin::IsToggledShowFades)
									);
			CommandList->MapAction( FTrueSkyCommands::Get().ToggleShowCompositing,
									FExecuteAction::CreateRaw(this, &FTrueSkyEditorPlugin::OnToggleShowCompositing),
									FCanExecuteAction::CreateRaw(this, &FTrueSkyEditorPlugin::IsMenuEnabled),
									FIsActionChecked::CreateRaw(this, &FTrueSkyEditorPlugin::IsToggledShowCompositing)
									);
			CommandList->MapAction( FTrueSkyCommands::Get().ToggleShow3DCloudTextures,
									FExecuteAction::CreateRaw(this, &FTrueSkyEditorPlugin::OnToggleShow3DCloudTextures),
									FCanExecuteAction::CreateRaw(this, &FTrueSkyEditorPlugin::IsMenuEnabled),
									FIsActionChecked::CreateRaw(this, &FTrueSkyEditorPlugin::IsToggledShow3DCloudTextures)
									);
			CommandList->MapAction( FTrueSkyCommands::Get().ToggleShow2DCloudTextures,
									FExecuteAction::CreateRaw(this, &FTrueSkyEditorPlugin::OnToggleShow2DCloudTextures),
									FCanExecuteAction::CreateRaw(this, &FTrueSkyEditorPlugin::IsMenuEnabled),
									FIsActionChecked::CreateRaw(this, &FTrueSkyEditorPlugin::IsToggledShow2DCloudTextures)
									);
		}

		MenuExtender = MakeShareable(new FExtender);
		MenuExtender->AddMenuExtension("WindowGlobalTabSpawners", EExtensionHook::After, CommandList, FMenuExtensionDelegate::CreateRaw(this, &FTrueSkyEditorPlugin::FillMenu));
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>( "LevelEditor" );
		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

		SequenceAssetTypeActions = MakeShareable(new FAssetTypeActions_TrueSkySequence);
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().RegisterAssetTypeActions(SequenceAssetTypeActions.ToSharedRef());
	}
#endif
	
#if !UE_BUILD_SHIPPING && WITH_EDITOR
	// Register for debug drawing
	//UDebugDrawService::Register(TEXT("TrueSKY"), FDebugDrawDelegate::CreateUObject(this, &FTrueSkyEditorPlugin::OnDebugTrueSky));
#endif
	const FName IconName(TEXT("../../Plugins/TrueSkyPlugin/Resources/icon_64x.png"));
	//	check( FPaths::FileExists( IconName.ToString() ) );
	//FSlateStyleSet& SlateStyleSet = (FSlateStyleSet&) FEditorStyle::GetInstance();
	//SlateStyleSet.Set( TEXT("ClassThumbnail.TrueSkySequenceAsset"), new FSlateImageBrush(IconName, FVector2D(64.0f, 64.0f)) );

	OpenUI							= NULL;
	CloseUI							= NULL;
	PushStyleSheetPath				= NULL;
	SetSequence						= NULL;
	GetSequence						= NULL;
	SetOnPropertiesChangedCallback	= NULL;
	SetOnTimeChangedInUICallback	= NULL;

	RenderingEnabled				=false;
	RendererInitialized				=false;
	StaticInitInterface				=NULL;
	StaticPushPath					=NULL;
	StaticRenderFrame				=NULL;
	StaticTick						=NULL;
	StaticOnDeviceChanged			=NULL;

	StaticTriggerAction				=NULL;

	PathEnv = NULL;
#if UE_EDITOR
	MessageId = RegisterWindowMessage(L"RESIZE");
#endif
}

void FTrueSkyEditorPlugin::OnDebugTrueSky(class UCanvas* Canvas, APlayerController*)
{
	const FColor OldDrawColor = Canvas->DrawColor;
	const FFontRenderInfo FontRenderInfo = Canvas->CreateFontRenderInfo(false, true);

	Canvas->SetDrawColor(FColor::White);

	UFont* RenderFont = GEngine->GetSmallFont();
	Canvas->DrawText(RenderFont, FString("trueSKY Debug Display"), 0.3f, 0.3f, 1.f, 1.f, FontRenderInfo);
	/*
		
	float res=Canvas->DrawText
	(
    UFont * InFont,
    const FString & InText,
    float X,
    float Y,
    float XScale,
    float YScale,
    const FFontRenderInfo & RenderInfo
)
		*/
	Canvas->SetDrawColor(OldDrawColor);
}

void FTrueSkyEditorPlugin::ShutdownModule()
{
#if !UE_BUILD_SHIPPING && WITH_EDITOR
	// Unregister for debug drawing
	//UDebugDrawService::Unregister(FDebugDrawDelegate::CreateUObject(this, &FTrueSkyEditorPlugin::OnDebugTrueSky));
#endif
#if UE_EDITOR
	FTrueSkyCommands::Unregister();
	if ( FModuleManager::Get().IsModuleLoaded("LevelEditor") )
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetMenuExtensibilityManager()->RemoveExtender( MenuExtender );

		FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(SequenceAssetTypeActions.ToSharedRef());
	}
#endif
	delete PathEnv;
	PathEnv = NULL;
}


#if UE_EDITOR
void FTrueSkyEditorPlugin::FillMenu( FMenuBuilder& MenuBuilder )
{
	MenuBuilder.BeginSection( "TrueSky", FText::FromString(TEXT("TrueSky")) );
	{		
		MenuBuilder.AddMenuEntry( FTrueSkyCommands::Get().AddSequence );
		MenuBuilder.AddMenuEntry( FTrueSkyCommands::Get().TriggerRecompileShaders);
		MenuBuilder.AddMenuEntry( FTrueSkyCommands::Get().TriggerShowDocumentation);
		try
		{
			FNewMenuDelegate d;
#ifdef SHARED_FROM_THIS
			TSharedRef< FTrueSkyEditorPlugin,(ESPMode::Type)0 > ref=AsShared();
			d=FNewMenuDelegate::CreateSP(this, &FTrueSkyEditorPlugin::FillOverlayMenu);
			MenuBuilder.AddSubMenu(FText::FromString("Overlays"),FText::FromString("TrueSKY overlays"),d);
#else
			MenuBuilder.AddSubMenu(FText::FromString("Overlays"),FText::FromString("TrueSKY overlays"),FNewMenuDelegate::CreateRaw(this,&FTrueSkyEditorPlugin::FillOverlayMenu ));
#endif
		}
		catch(...)
		{
			UE_LOG(TrueSky, Warning, TEXT("Failed to add trueSKY submenu"), TEXT(""));
		}
	}
	MenuBuilder.EndSection();
	
}
	
void FTrueSkyEditorPlugin::FillOverlayMenu(FMenuBuilder& MenuBuilder)
{		
	MenuBuilder.AddMenuEntry(FTrueSkyCommands::Get().ToggleFades);
	MenuBuilder.AddMenuEntry(FTrueSkyCommands::Get().ToggleShowCompositing);
	MenuBuilder.AddMenuEntry(FTrueSkyCommands::Get().ToggleShow3DCloudTextures);
	MenuBuilder.AddMenuEntry(FTrueSkyCommands::Get().ToggleShow2DCloudTextures);
}
#endif
/** Returns environment variable value */
static wchar_t* GetEnvVariable( const wchar_t* const VariableName, int iEnvSize = 1024)
{
	wchar_t* Env = new wchar_t[iEnvSize];
	check( Env );
	memset(Env, 0, iEnvSize * sizeof(wchar_t));
	if ( (int)GetEnvironmentVariableW(VariableName, Env, iEnvSize) > iEnvSize )
	{
		delete [] Env;
		Env = NULL;
	}
	else if ( wcslen(Env) == 0 )
	{
		return NULL;
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
static std::string ConstructPathUTF8(const TCHAR* const BasePath, const TCHAR* const RelativePath)
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

		delete [] NewPath;
		std::string ret=utf8NewPath;
		delete [] utf8NewPath;
		return ret;
	}
	return "";
}


/** Returns HWND for a given SWindow (if native!) */
#if UE_EDITOR
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
#endif

static std::wstring Utf8ToWString(const char *src_utf8)
{
	int src_length=(int)strlen(src_utf8);
#ifdef _MSC_VER
	int length = MultiByteToWideChar(CP_UTF8, 0, src_utf8,src_length, 0, 0);
#else
	int length=src_length;
#endif
	wchar_t *output_buffer = new wchar_t [length+1];
#ifdef _MSC_VER
	MultiByteToWideChar(CP_UTF8, 0, src_utf8, src_length, output_buffer, length);
#else
	mbstowcs(output_buffer, src_utf8, (size_t)length );
#endif
	output_buffer[length]=0;
	std::wstring wstr=std::wstring(output_buffer);
	delete [] output_buffer;
	return wstr;
}
static std::string WStringToUtf8(const wchar_t *src_w)
{
	int src_length=(int)wcslen(src_w);
#ifdef _MSC_VER
	int size_needed = WideCharToMultiByte(CP_UTF8, 0,src_w, (int)src_length, NULL, 0, NULL, NULL);
#else
	int size_needed=2*src_length;
#endif
	char *output_buffer = new char [size_needed+1];
#ifdef _MSC_VER
	WideCharToMultiByte (CP_UTF8,0,src_w,(int)src_length,output_buffer, size_needed, NULL, NULL);
#else
	wcstombs(output_buffer, src_w, (size_t)size_needed );
#endif
	output_buffer[size_needed]=0;
	std::string str_utf8=std::string(output_buffer);
	delete [] output_buffer;
	return str_utf8;
}

#if UE_EDITOR
#define warnf(expr, ...)				{ if(!(expr)) FDebug::AssertFailed( #expr, __FILE__, __LINE__, ##__VA_ARGS__ ); CA_ASSUME(expr); }
FTrueSkyEditorPlugin::SEditorInstance* FTrueSkyEditorPlugin::CreateEditorInstance(   void* Env )
{
#if 0
	const TCHAR* const DllPath =L"TrueSkyUI_MDd.dll";
#else
	const TCHAR* const DllPath =L"TrueSkyUI_MD.dll";
#endif
	void* const DllHandle = FPlatformProcess::GetDllHandle( DllPath );
	if(DllHandle==NULL)
	{
		UE_LOG(TrueSky, Warning, TEXT("Failed to load %s"), DllPath);
	}
	if ( DllHandle != NULL )
	{
		OpenUI = (FOpenUI)FPlatformProcess::GetDllExport(DllHandle, TEXT("OpenUI") );
		CloseUI = (FCloseUI)FPlatformProcess::GetDllExport(DllHandle, TEXT("CloseUI") );
						
		StaticSetUIString = (FStaticSetUIString)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticSetUIString") );
		StaticGetUIString = (FStaticGetUIString)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticGetUIString") );
		
		PushStyleSheetPath = (FPushStyleSheetPath)FPlatformProcess::GetDllExport(DllHandle, TEXT("PushStyleSheetPath") );
		SetSequence = (FSetSequence)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticSetSequence") );
		GetSequence = (FGetSequence)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticGetSequence") );
		SetOnPropertiesChangedCallback = (FSetOnPropertiesChangedCallback)FPlatformProcess::GetDllExport(DllHandle, TEXT("SetOnPropertiesChangedCallback") );
		SetOnTimeChangedInUICallback = (FSetOnTimeChangedInUICallback)FPlatformProcess::GetDllExport(DllHandle, TEXT("SetOnTimeChangedCallback") );

		checkf( OpenUI, L"OpenUI function not found!" );
		checkf( CloseUI, L"CloseUI function not found!" );
		checkf( PushStyleSheetPath, L"PushStyleSheetPath function not found!" );
		checkf( SetSequence, L"SetSequence function not found!" );
		checkf( GetSequence, L"GetSequence function not found!" );
		checkf( SetOnPropertiesChangedCallback, L"SetOnPropertiesChangedCallback function not found!" );
		checkf( SetOnTimeChangedInUICallback, L"SetOnTimeChangedInUICallback function not found!" );
		
		checkf( StaticSetUIString, L"StaticSetUIString function not found!" );
		checkf( StaticGetUIString, L"StaticGetUIString function not found!" );

		PushStyleSheetPath((trueSkyPluginPath+"\\Resources\\qss\\").c_str());

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
			EditorInstance.EditorWindow->SetOnWindowClosed( FOnWindowClosed::CreateRaw(this, &FTrueSkyEditorPlugin::OnMainWindowClosed) );
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

			//	GetWindowRect(EditorInstance.EditorWindowHWND, &ParentRect);

				OpenUI( EditorInstance.EditorWindowHWND, &ClientRect, &ParentRect, Env, UNREAL_STYLE);
				SetUIString(&EditorInstance,"LicenceKey",TRUESKY_LICENCE_KEY);
				// Overload main window's WndProc
				EditorInstance.OrigEditorWindowWndProc = (WNDPROC)GetWindowLongPtr( EditorInstance.EditorWindowHWND, GWLP_WNDPROC );
				SetWindowLongPtr( EditorInstance.EditorWindowHWND, GWLP_WNDPROC, (LONG_PTR)EditorWindowWndProc );

				// Setup notification callback
				SetOnPropertiesChangedCallback(  OnSequenceChangeCallback );
				SetOnTimeChangedInUICallback(OnTimeChangedCallback);
				EditorInstance.EditorWindow->Restore();
				return &EditorInstances[ EditorInstances.Add(EditorInstance) ];
			}
		}
	}

	return NULL;
}


FTrueSkyEditorPlugin::SEditorInstance* FTrueSkyEditorPlugin::FindEditorInstance(const TSharedRef<SWindow>& EditorWindow)
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

FTrueSkyEditorPlugin::SEditorInstance* FTrueSkyEditorPlugin::FindEditorInstance(HWND const EditorWindowHWND)
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

FTrueSkyEditorPlugin::SEditorInstance* FTrueSkyEditorPlugin::FindEditorInstance(UTrueSkySequenceAsset* const Asset)
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

int FTrueSkyEditorPlugin::FindEditorInstance(FTrueSkyEditorPlugin::SEditorInstance* const Instance)
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

void FTrueSkyEditorPlugin::SaveAllEditorInstances()
{
	for (int i = 0; i < EditorInstances.Num(); ++i)
	{
		EditorInstances[i].SaveSequenceData();
	}
}

void FTrueSkyEditorPlugin::SEditorInstance::SaveSequenceData()
{
	if ( Asset )
	{
		struct Local
		{
			static char* AllocString(int size)
			{
				check( size > 0 );
				return new char[size];
			}
		};
		check( FTrueSkyEditorPlugin::Instance );
		check( FTrueSkyEditorPlugin::Instance->GetSequence );
		if ( char* const OutputText = FTrueSkyEditorPlugin::Instance->GetSequence(EditorWindowHWND, Local::AllocString) )
		{
			if ( Asset->SequenceText.Num() > 0 )
			{
				if ( strcmp(OutputText, (const char*)Asset->SequenceText.GetData()) == 0 )
				{
					// No change -> quit
					delete OutputText;
					return;
				}
			}

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

void FTrueSkyEditorPlugin::SEditorInstance::LoadSequenceData()
{
	if ( Asset && Asset->SequenceText.Num() > 0 )
	{
		check( FTrueSkyEditorPlugin::Instance );
		check( FTrueSkyEditorPlugin::Instance->SetSequence );
		FTrueSkyEditorPlugin::Instance->SetSequence( EditorWindowHWND, (const char*)Asset->SequenceText.GetData() );
	}
}


void FTrueSkyEditorPlugin::OnMainWindowClosed(const TSharedRef<SWindow>& Window)
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
void FTrueSkyEditorPlugin::OnSequenceChangeCallback(HWND OwnerHWND,const char *txt)
{
	check( Instance );
	if ( SEditorInstance* const EditorInstance = Instance->FindEditorInstance(OwnerHWND) )
	{
		EditorInstance->SaveSequenceData();
	}
}

void FTrueSkyEditorPlugin::OnTimeChangedCallback(HWND OwnerHWND,float t)
{
	check( Instance );
	if ( SEditorInstance* const EditorInstance = Instance->FindEditorInstance(OwnerHWND) )
	{
		//EditorInstance->SaveSequenceData();
	}
}

::UINT FTrueSkyEditorPlugin::MessageId = 0;

LRESULT CALLBACK FTrueSkyEditorPlugin::EditorWindowWndProc(HWND hWnd, ::UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ( uMsg == WM_SIZE)//||uMsg==WM_ACTIVATE||uMsg==WM_MOVE)
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
#endif

void FTrueSkyEditorPlugin::InitPaths()
{
	if ( PathEnv == NULL )
	{
		const int iPathSize = 4096;
		const wchar_t *p=GetEnvVariable(L"PATH", iPathSize);
		delete [] p;
		if ( p == NULL )
		{
			return;
		}
		std::wstring PathEnv = p;
		std::wstring trueSkyPluginPathW=Utf8ToWString(trueSkyPluginPath.c_str());
		PathEnv=(trueSkyPluginPathW+L"\\Binaries\\Win64;")+PathEnv;

		SetEnvironmentVariable( L"PATH", PathEnv.c_str());
	}
}

void FTrueSkyEditorPlugin::OnToggleRendering()
{
	ITrueSkyPlugin &trueSkyPlugin=ITrueSkyPlugin::Get();
	trueSkyPlugin.OnToggleRendering();
}

IMPLEMENT_TOGGLE(ShowFades)
IMPLEMENT_TOGGLE(ShowCompositing)
IMPLEMENT_TOGGLE(Show3DCloudTextures)
IMPLEMENT_TOGGLE(Show2DCloudTextures)

bool FTrueSkyEditorPlugin::IsToggleRenderingEnabled()
{
	ITrueSkyPlugin &trueSkyPlugin=ITrueSkyPlugin::Get();
	if(trueSkyPlugin.GetActiveSequence())
	{
		return true;
	}
	// No active sequence found!
	trueSkyPlugin.SetRenderingEnabled(false);
	return false;
}

bool FTrueSkyEditorPlugin::IsMenuEnabled()
{
	return true;
}

bool FTrueSkyEditorPlugin::IsToggleRenderingChecked()
{
	return RenderingEnabled;
}

void FTrueSkyEditorPlugin::OnAddSequence()
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
		SequenceActor=GWorld->SpawnActor<ATrueSkySequenceActor>(ATrueSkySequenceActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	}
	else
	{
		// Sequence actor already exists -- error message?
	}
}

void FTrueSkyEditorPlugin::OnSequenceDestroyed()
{
}

bool FTrueSkyEditorPlugin::IsAddSequenceEnabled()
{
	// Returns false if TrueSkySequenceActor already exists!
	ULevel* const Level = GWorld->PersistentLevel;
	for(int i=0;i<Level->Actors.Num();i++)
	{
		if ( Cast<ATrueSkySequenceActor>(Level->Actors[i]) )
			return false;
	}
	return true;
}


void FTrueSkyEditorPlugin::OpenEditor(UTrueSkySequenceAsset* const TrueSkySequence)
{
#if UE_EDITOR
	if ( TrueSkySequence == NULL )
		return;

	SEditorInstance* EditorInstance = FindEditorInstance(TrueSkySequence);
	if ( EditorInstance == NULL )
	{
		InitPaths();
		
		ITrueSkyPlugin &trueSkyPlugin=ITrueSkyPlugin::Get();

//		trueSkyPlugin.InitRenderingInterface();
		void* const Env = (trueSkyPlugin.GetActiveSequence() == TrueSkySequence)? trueSkyPlugin.GetRenderEnvironment() : NULL;

		EditorInstance = CreateEditorInstance(  Env );
		if ( EditorInstance )
		{
			EditorInstance->Asset = TrueSkySequence;
#ifdef ENABLE_AUTO_SAVING
			AutoSaveTimer = 4.0f;
#endif
		}
	}

	// Set sequence asset to UI
	if ( EditorInstance )
	{
		EditorInstance->LoadSequenceData();
	}
#endif
}



