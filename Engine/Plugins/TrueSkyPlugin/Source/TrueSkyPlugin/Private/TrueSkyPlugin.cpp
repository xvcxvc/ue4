#define TRUESKY_LICENCE_KEY "place key here"
// Replace the string above with your own licence key!

// Copyright 2013-2014 Simul Software Ltd. All Rights Reserved.

#include "TrueSkyPluginPrivatePCH.h"
#include "PlacementModePrivatePCH.h"
#include "TrueSkySequenceAsset.h"
#include "TrueSkySequenceFactory.h"
#include "TrueSkySequenceActor.h"
#include "LevelEditor.h"
#include "IMainFrameModule.h"
#include "SlateStyle.h"
#include "IDocumentation.h"
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

DECLARE_LOG_CATEGORY_EXTERN(LogD3D11RHI, Log, All);

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
#include "AssetToolsModule.h"
#include "AssetTypeActions_TrueSkySequence.h"
//#include "TrueSkyPlugin.generated.inl"
#include "EngineModule.h"
#include "Editor.h"
#include <string>

#define ENABLE_AUTO_SAVING

#pragma optimize("",off)

#define DECLARE_TOGGLE(name)\
	void					OnToggle##name();\
	bool					IsToggled##name();

#define IMPLEMENT_TOGGLE(name)\
	void FTrueSkyPlugin::OnToggle##name()\
{\
	if(StaticGetRenderBool!=NULL&&StaticSetRenderBool!=NULL)\
	{\
		bool current=StaticGetRenderBool(#name);\
		StaticSetRenderBool(#name,!current);\
	}\
}\
\
bool FTrueSkyPlugin::IsToggled##name()\
{\
	if(StaticGetRenderBool!=NULL)\
		if(StaticGetRenderBool)\
			return StaticGetRenderBool(#name);\
	return false;\
}

#define DECLARE_ACTION(name)\
	void					OnTrigger##name()\
	{\
		if(StaticTriggerAction!=NULL)\
			StaticTriggerAction(#name);\
	}


class FTrueSkyTickable : public  FTickableGameObject
{
public:
	/** Tick interface */
	void					Tick( float DeltaTime );
	bool					IsTickable() const;
	TStatId					GetStatId() const;
};

class FTrueSkyPlugin : public ITrueSkyPlugin
#ifdef SHARED_FROM_THIS
	, public TSharedFromThis<FTrueSkyPlugin,(ESPMode::Type)0>
#endif
{
public:
	FTrueSkyPlugin();
	virtual ~FTrueSkyPlugin();

	static FTrueSkyPlugin*	Instance;
	
	void					OnDebugTrueSky(class UCanvas* Canvas, APlayerController*);

	/** IModuleInterface implementation */
	virtual void			StartupModule() override;
	virtual void			ShutdownModule() override;
	virtual bool			SupportsDynamicReloading() override;

	/** Render delegate */
	void					RenderFrame( FPostOpaqueRenderParameters& RenderParameters );

	/** TrueSKY menu */
	void					FillMenu( FMenuBuilder& MenuBuilder );
	/** Overlay sub-menu */
	void					FillOverlayMenu(FMenuBuilder& MenuBuilder);

	/** Init rendering */
	bool					InitRenderingInterface(  );

	/** Enable rendering */
	void					SetRenderingEnabled( bool Enabled );

	/** Open editor */
	virtual void			OpenEditor(UTrueSkySequenceAsset* const TrueSkySequence);

	/** If there is a TrueSkySequenceActor in the persistent level, this returns that actor's TrueSkySequenceAsset */
	UTrueSkySequenceAsset*	GetActiveSequence();
	
	void UpdateFromActor();

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
	
	void					Tick( float DeltaTime );

	virtual void			SetRenderFloat(const char* name, float value);
	virtual float			GetRenderFloat(const char* name) const;
	
	virtual void			SetRenderString(const char* name, const char*  value);
	virtual const char*		GetRenderString(const char* name) const;
	
	virtual void			SetUIString(SEditorInstance* const EditorInstance,const char* name, const char*  value);
	virtual const char*		GetUIString(SEditorInstance* const EditorInstance,const char* name) const;
	
	virtual void			SetCloudShadowRenderTarget(FRenderTarget *t);
protected:
	
	void					RenderCloudShadow();
	void					OnMainWindowClosed(const TSharedRef<SWindow>& Window);

	/** Called when Toggle rendering button is pressed */
	void					OnToggleRendering();
	/** Returns true if Toggle rendering button should be enabled */
	bool					IsToggleRenderingEnabled();
	/** Returns true if Toggle rendering button should be checked */
	bool					IsToggleRenderingChecked();

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

	/** Creates new instance of UI */
	SEditorInstance*		CreateEditorInstance(  void* Env );

	TSharedPtr< FExtender > MenuExtender;
	TSharedPtr<FAssetTypeActions_TrueSkySequence> SequenceAssetTypeActions;
	enum PluginStyle
	{
		DEFAULT_STYLE=0
		,UNREAL_STYLE=1
		,UNITY_STYLE=2
		,UNITY_STYLE_DEFERRED=3
	};
	struct Viewport
	{
		int x,y,w,h;
	};
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
	typedef void* (*FStaticGetEnvironment)();
	typedef int (*FStaticSetSequence)( std::string sequenceInputText );
	typedef class UE4PluginRenderInterface* (*FStaticGetRenderInterfaceInstance)();
	typedef void (*FStaticSetRenderBool)( const char* name,bool value );
	typedef bool (*FStaticGetRenderBool)( const char* name );
	typedef void (*FStaticSetRenderFloat)( const char* name,float value );
	typedef float (*FStaticGetRenderFloat)( const char* name );
	typedef void (*FStaticSetRenderString)( const char* name,const char* value );
	typedef void (*FStaticGetRenderString)( const char* name ,char* value,int len);
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
	FStaticGetEnvironment				StaticGetEnvironment;
	FStaticSetSequence					StaticSetSequence;
	FStaticGetRenderInterfaceInstance	StaticGetRenderInterfaceInstance;
	FStaticSetRenderBool				StaticSetRenderBool;
	FStaticGetRenderBool				StaticGetRenderBool;
	FStaticSetRenderFloat				StaticSetRenderFloat;
	FStaticGetRenderFloat				StaticGetRenderFloat;
	FStaticSetRenderString				StaticSetRenderString;
	FStaticGetRenderString				StaticGetRenderString;
	FStaticTriggerAction				StaticTriggerAction;

	TCHAR*					PathEnv;

	bool					RenderingEnabled;
	bool					RendererInitialized;

	float					CachedDeltaSeconds;
	float					AutoSaveTimer;		// 0.0f == no auto-saving

	TArray<SEditorInstance>	EditorInstances;

	static LRESULT CALLBACK EditorWindowWndProc(HWND, ::UINT, WPARAM, LPARAM);
	static ::UINT			MessageId;

	static void				OnSequenceChangeCallback(HWND OwnerHWND,const char *);
	
	static void				OnTimeChangedCallback(HWND OwnerHWND,float t);
	
	FRenderTarget			*cloudShadowRenderTarget;

	bool					actorPropertiesChanged;
};


IMPLEMENT_MODULE( FTrueSkyPlugin, TrueSkyPlugin )


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

FTrueSkyPlugin* FTrueSkyPlugin::Instance = NULL;

//TSharedRef<FTrueSkyPlugin> staticSharedRef;
static std::string trueSkyPluginPath="../../Plugins/TrueSkyPlugin";
FTrueSkyPlugin::FTrueSkyPlugin()
	:cloudShadowRenderTarget(NULL)
	,actorPropertiesChanged(true)
{
	Instance = this;
#ifdef SHARED_FROM_THIS
	//TSharedRef<FTrueSkyPlugin> sharedRef=AsShared();
TSharedRef< FTrueSkyPlugin,(ESPMode::Type)0 > ref=AsShared();
#endif
	EditorInstances.Reset();
	AutoSaveTimer = 0.0f;
	//we need to pass through real DeltaSecond; from our scene Actor?
	CachedDeltaSeconds = 0.0333f;

}

FTrueSkyPlugin::~FTrueSkyPlugin()
{
	Instance = NULL;
}


bool FTrueSkyPlugin::SupportsDynamicReloading()
{
	return false;
}

void FTrueSkyPlugin::Tick( float DeltaTime )
{
	if(IsInGameThread())
	{

	}
	CachedDeltaSeconds = DeltaTime;
#ifdef ENABLE_AUTO_SAVING
	if ( AutoSaveTimer > 0.0f )
	{
		if ( (AutoSaveTimer -= DeltaTime) <= 0.0f )
		{
			SaveAllEditorInstances();
			AutoSaveTimer = 4.0f;
		}
	}
#endif
}

void FTrueSkyPlugin::SetCloudShadowRenderTarget(FRenderTarget *t)
{
	cloudShadowRenderTarget=t;
}

void FTrueSkyPlugin::RenderCloudShadow()
{
	if(!cloudShadowRenderTarget)
		return;
//	FTextureRenderTarget2DResource* res = (FTextureRenderTarget2DResource*)cloudShadowRenderTarget->Resource;
/*	FCanvas* Canvas = new FCanvas(cloudShadowRenderTarget, NULL, NULL);
	Canvas->Clear(FLinearColor::Blue);
	// Write text (no text is visible since the Canvas has no effect
	UFont* Font = GEngine->GetSmallFont();
	Canvas->DrawShadowedString(100, 100, TEXT("Test"), Font, FLinearColor::White);
	Canvas->Flush();
	delete Canvas;*/
}

void FTrueSkyPlugin::SetRenderFloat(const char* name, float value)
{
	if( StaticSetRenderFloat != NULL )
	{
		StaticSetRenderFloat( name, value );
	}
	else
	{
		UE_LOG(TrueSky, Warning, TEXT("Trying to set render float before StaticSetRenderFloat has been set"), TEXT(""));
	}
}

float FTrueSkyPlugin::GetRenderFloat(const char* name) const
{
	if( StaticGetRenderFloat != NULL )
	{
		return StaticGetRenderFloat( name );
	}

	UE_LOG(TrueSky, Warning, TEXT("Trying to get render float before StaticGetRenderFloat has been set"), TEXT(""));
	return 0.0f;
}

void FTrueSkyPlugin::SetRenderString(const char* name, const char*  value)
{
	if( StaticSetRenderString != NULL )
	{
		StaticSetRenderString( name, value );
	}
	else
	{
		UE_LOG(TrueSky, Warning, TEXT("Trying to set render string before StaticSetRenderString has been set"), TEXT(""));
	}
}

const char*	 FTrueSkyPlugin::GetRenderString(const char* name) const
{
	if( StaticGetRenderString != NULL )
	{
		static char txt[50];
		StaticGetRenderString( name,txt,50);
		return txt;
	}

	UE_LOG(TrueSky, Warning, TEXT("Trying to get render string before StaticGetRenderString has been set"), TEXT(""));
	return NULL;
}


void FTrueSkyPlugin::SetUIString(SEditorInstance* const EditorInstance,const char* name, const char*  value)
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

const char*	 FTrueSkyPlugin::GetUIString(SEditorInstance* const EditorInstance,const char* name) const
{
	if( StaticSetUIString != NULL&&EditorInstance!=NULL)
	{
		return StaticGetUIString(EditorInstance->EditorWindowHWND,name);
	}

	UE_LOG(TrueSky, Warning, TEXT("Trying to get UI string before StaticSetUIString has been set"), TEXT(""));
	return NULL;
}

/** Tickable object interface */
void FTrueSkyTickable::Tick( float DeltaTime )
{
	if(FTrueSkyPlugin::Instance)
		FTrueSkyPlugin::Instance->Tick(DeltaTime);
}

bool FTrueSkyTickable::IsTickable() const
{
	return true;
}

TStatId FTrueSkyTickable::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FTrueSkyTickable, STATGROUP_Tickables);
}


void FTrueSkyPlugin::StartupModule()
{
	FTrueSkyCommands::Register();
	IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
	const TSharedRef<FUICommandList>& CommandList = MainFrameModule.GetMainFrameCommandBindings();
	CommandList->MapAction( FTrueSkyCommands::Get().AddSequence,
							FExecuteAction::CreateRaw(this, &FTrueSkyPlugin::OnAddSequence),
							FCanExecuteAction::CreateRaw(this, &FTrueSkyPlugin::IsAddSequenceEnabled)
							);
		CommandList->MapAction( FTrueSkyCommands::Get().TriggerRecompileShaders,
								FExecuteAction::CreateRaw(this, &FTrueSkyPlugin::OnTriggerRecompileShaders),
								FCanExecuteAction::CreateRaw(this, &FTrueSkyPlugin::IsToggleRenderingChecked)
								);
		CommandList->MapAction( FTrueSkyCommands::Get().TriggerShowDocumentation,
								FExecuteAction::CreateRaw(this, &FTrueSkyPlugin::ShowDocumentation)
								);
	{
		CommandList->MapAction( FTrueSkyCommands::Get().ToggleFades,
								FExecuteAction::CreateRaw(this, &FTrueSkyPlugin::OnToggleShowFades),
								FCanExecuteAction::CreateRaw(this, &FTrueSkyPlugin::IsToggleRenderingChecked),
								FIsActionChecked::CreateRaw(this, &FTrueSkyPlugin::IsToggledShowFades)
								);
		CommandList->MapAction( FTrueSkyCommands::Get().ToggleShowCompositing,
								FExecuteAction::CreateRaw(this, &FTrueSkyPlugin::OnToggleShowCompositing),
								FCanExecuteAction::CreateRaw(this, &FTrueSkyPlugin::IsToggleRenderingChecked),
								FIsActionChecked::CreateRaw(this, &FTrueSkyPlugin::IsToggledShowCompositing)
								);
		CommandList->MapAction( FTrueSkyCommands::Get().ToggleShow3DCloudTextures,
								FExecuteAction::CreateRaw(this, &FTrueSkyPlugin::OnToggleShow3DCloudTextures),
								FCanExecuteAction::CreateRaw(this, &FTrueSkyPlugin::IsToggleRenderingChecked),
								FIsActionChecked::CreateRaw(this, &FTrueSkyPlugin::IsToggledShow3DCloudTextures)
								);
		CommandList->MapAction( FTrueSkyCommands::Get().ToggleShow2DCloudTextures,
								FExecuteAction::CreateRaw(this, &FTrueSkyPlugin::OnToggleShow2DCloudTextures),
								FCanExecuteAction::CreateRaw(this, &FTrueSkyPlugin::IsToggleRenderingChecked),
								FIsActionChecked::CreateRaw(this, &FTrueSkyPlugin::IsToggledShow2DCloudTextures)
								);
	}

	MenuExtender = MakeShareable(new FExtender);
	MenuExtender->AddMenuExtension("WindowGlobalTabSpawners", EExtensionHook::After, CommandList, FMenuExtensionDelegate::CreateRaw(this, &FTrueSkyPlugin::FillMenu));
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>( "LevelEditor" );
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

	SequenceAssetTypeActions = MakeShareable(new FAssetTypeActions_TrueSkySequence);
	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().RegisterAssetTypeActions(SequenceAssetTypeActions.ToSharedRef());

	GetRendererModule().RegisterPostOpaqueRenderDelegate( FPostOpaqueRenderDelegate::CreateRaw(this, &FTrueSkyPlugin::RenderFrame) );
	
#if !UE_BUILD_SHIPPING && WITH_EDITOR
	// Register for debug drawing
	//UDebugDrawService::Register(TEXT("TrueSKY"), FDebugDrawDelegate::CreateUObject(this, &FTrueSkyPlugin::OnDebugTrueSky));
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
	StaticGetEnvironment			=NULL;

	StaticGetRenderInterfaceInstance=NULL;
	
	StaticSetRenderBool				=NULL;
	StaticGetRenderBool				=NULL;
	StaticTriggerAction				=NULL;

	StaticSetRenderFloat			=NULL;
	StaticGetRenderFloat			=NULL;
	

	StaticSetRenderString			=NULL;
	StaticGetRenderString			=NULL;

	PathEnv = NULL;

	MessageId = RegisterWindowMessage(L"RESIZE");
}

void FTrueSkyPlugin::SetRenderingEnabled( bool Enabled )
{
	RenderingEnabled = Enabled;
}

void FTrueSkyPlugin::RenderFrame( FPostOpaqueRenderParameters& RenderParameters )
{	
	check(IsInRenderingThread());
	if(!RenderParameters.ViewportRect.Width()||!RenderParameters.ViewportRect.Height())
		return;
	UpdateFromActor();
	if(!RenderingEnabled )
		return;
	SCOPED_DRAW_EVENT(TrueSkyRenderFrame, FColor( 0, 0, 255 ) );
	if( RenderingEnabled )
	{
		FSceneView *View=(FSceneView*)(RenderParameters.Uid);
		StaticTick( 0 );

		//FD3D11DynamicRHI * d3d11rhi = (FD3D11DynamicRHI*)GDynamicRHI;
		ID3D11Device * device =(ID3D11Device *)GDynamicRHI->RHIGetNativeDevice();
		ID3D11DeviceContext * context =NULL;// d3d11rhi->GetDeviceContext();
		device->GetImmediateContext(&context);
		FMatrix mirroredViewMatrix = RenderParameters.ViewMatrix;

		//mirroredViewMatrix=mirroredViewMatrix.Inverse();
		FD3D11TextureBase * depthTex		= static_cast<FD3D11Texture2D*>(RenderParameters.DepthTexture);	
		FD3D11TextureBase * halfDepthTex	= static_cast<FD3D11Texture2D*>(RenderParameters.SmallDepthTexture);		
		
		Viewport v;
		v.x=RenderParameters.ViewportRect.Min.X;
		v.y=RenderParameters.ViewportRect.Min.Y;
		v.w=RenderParameters.ViewportRect.Width();
		v.h=RenderParameters.ViewportRect.Height();
		unsigned uid=((unsigned)v.w<<(unsigned)24)+((unsigned)v.h<<(unsigned)16)+((unsigned)View->StereoPass);
        int view_id = StaticGetOrAddView((void*)uid);		// RVK: really need a unique view ident to pass here..
		StaticRenderFrame( device,view_id, &(mirroredViewMatrix.M[0][0]), &(RenderParameters.ProjMatrix.M[0][0])
			,(ID3D11Texture2D*)depthTex->GetResource(),depthTex->GetShaderResourceView(),&v
							 ,UNREAL_STYLE);
		RenderCloudShadow();
	}
}

void FTrueSkyPlugin::OnDebugTrueSky(class UCanvas* Canvas, APlayerController*)
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

void FTrueSkyPlugin::ShutdownModule()
{
#if !UE_BUILD_SHIPPING && WITH_EDITOR
	// Unregister for debug drawing
	//UDebugDrawService::Unregister(FDebugDrawDelegate::CreateUObject(this, &FTrueSkyPlugin::OnDebugTrueSky));
#endif

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
		MenuBuilder.AddMenuEntry( FTrueSkyCommands::Get().AddSequence );
		MenuBuilder.AddMenuEntry( FTrueSkyCommands::Get().TriggerRecompileShaders);
		MenuBuilder.AddMenuEntry( FTrueSkyCommands::Get().TriggerShowDocumentation);
		try
		{
			FNewMenuDelegate d;
#ifdef SHARED_FROM_THIS
			TSharedRef< FTrueSkyPlugin,(ESPMode::Type)0 > ref=AsShared();
			d=FNewMenuDelegate::CreateSP(this, &FTrueSkyPlugin::FillOverlayMenu);
			MenuBuilder.AddSubMenu(FText::FromString("Overlays"),FText::FromString("TrueSKY overlays"),d);
#else
			MenuBuilder.AddSubMenu(FText::FromString("Overlays"),FText::FromString("TrueSKY overlays"),FNewMenuDelegate::CreateRaw(this,&FTrueSkyPlugin::FillOverlayMenu ));
#endif
		}
		catch(...)
		{
			UE_LOG(TrueSky, Warning, TEXT("Failed to add trueSKY submenu"), TEXT(""));
		}
	}
	MenuBuilder.EndSection();
	
}
	
void FTrueSkyPlugin::FillOverlayMenu(FMenuBuilder& MenuBuilder)
{		
	MenuBuilder.AddMenuEntry(FTrueSkyCommands::Get().ToggleFades);
	MenuBuilder.AddMenuEntry(FTrueSkyCommands::Get().ToggleShowCompositing);
	MenuBuilder.AddMenuEntry(FTrueSkyCommands::Get().ToggleShow3DCloudTextures);
	MenuBuilder.AddMenuEntry(FTrueSkyCommands::Get().ToggleShow2DCloudTextures);
}
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

bool FTrueSkyPlugin::InitRenderingInterface(  )
{
#if 0
	const TCHAR* const DllPath =L"TrueSkyPluginRender_MDd.dll";
#else
	const TCHAR* const DllPath =L"TrueSkyPluginRender_MD.dll";
#endif
	check( DllPath );

	void* const DllHandle = FPlatformProcess::GetDllHandle( DllPath );
	if(DllHandle==NULL)
	{
		UE_LOG(TrueSky, Warning, TEXT("Failed to load %s"), DllPath);
	}
	if ( DllHandle != NULL )
	{
		StaticInitInterface				=(FStaticInitInterface)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticInitInterface") );
		StaticPushPath					=(FStaticPushPath)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticPushPath") );
		StaticGetOrAddView				=(FStaticGetOrAddView)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticGetOrAddView") );
		StaticRenderFrame				=(FStaticRenderFrame)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticRenderFrame") );
		StaticOnDeviceChanged			=(FStaticOnDeviceChanged)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticOnDeviceChanged") );
		StaticTick						=(FStaticTick)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticTick") );
		StaticGetEnvironment			=(FStaticGetEnvironment)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticGetEnvironment"));
		StaticSetSequence				=(FStaticSetSequence)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticSetSequence"));
		StaticGetRenderInterfaceInstance=(FStaticGetRenderInterfaceInstance)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticGetRenderInterfaceInstance"));
		StaticSetRenderBool				=(FStaticSetRenderBool)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticSetRenderBool"));
		StaticGetRenderBool				=(FStaticGetRenderBool)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticGetRenderBool"));
		StaticSetRenderFloat			=(FStaticSetRenderFloat)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticSetRenderFloat"));
		StaticGetRenderFloat			=(FStaticGetRenderFloat)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticGetRenderFloat"));
		
		StaticGetRenderString			=(FStaticGetRenderString)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticGetRenderString"));
		StaticSetRenderString			=(FStaticSetRenderString)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticSetRenderString"));

		StaticTriggerAction				=(FStaticTriggerAction)FPlatformProcess::GetDllExport(DllHandle, TEXT("StaticTriggerAction"));
		if( StaticInitInterface == NULL ||StaticPushPath==NULL|| StaticRenderFrame == NULL || StaticGetOrAddView==NULL||
			StaticOnDeviceChanged == NULL || StaticTick == NULL  || 
			StaticGetEnvironment == NULL || StaticSetSequence == NULL||StaticGetRenderInterfaceInstance==NULL
			||StaticSetRenderBool==NULL
			||StaticGetRenderBool==NULL||StaticTriggerAction==NULL
			||StaticSetRenderFloat==NULL || StaticGetRenderFloat==NULL
			||StaticSetRenderString==NULL || StaticGetRenderString==NULL)
		{
			//missing dll functions... cancel initialization
			SetRenderingEnabled(false);
			return false;
		}

		StaticInitInterface(  );
		
		StaticPushPath("ShaderPath",(trueSkyPluginPath+"\\Resources\\Platform\\DirectX11\\HLSL").c_str());
		StaticPushPath("ShaderBinaryPath",(trueSkyPluginPath+"\\Resources\\Platform\\DirectX11\\shaderbin").c_str());
		StaticPushPath("TexturePath",(trueSkyPluginPath+"\\Resources\\Media\\Textures").c_str());
		
		// IF there's a "SIMUL" env variable, we can build shaders direct from there:
		wchar_t *SimulPath = GetEnvVariable(L"SIMUL");
		if(SimulPath)
			StaticPushPath("ShaderPath", ConstructPathUTF8( SimulPath, L"\\Platform\\DirectX11\\HLSL" ).c_str());
		delete [] SimulPath;
		ID3D11Device * device = (ID3D11Device*)GDynamicRHI->RHIGetNativeDevice();

		if( device != NULL )
		{
			StaticOnDeviceChanged(device);
		}

		RendererInitialized = true;
		return true;
	}
	return false;
}

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

#define warnf(expr, ...)				{ if(!(expr)) FDebug::AssertFailed( #expr, __FILE__, __LINE__, ##__VA_ARGS__ ); CA_ASSUME(expr); }
FTrueSkyPlugin::SEditorInstance* FTrueSkyPlugin::CreateEditorInstance(   void* Env )
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

void FTrueSkyPlugin::SaveAllEditorInstances()
{
	for (int i = 0; i < EditorInstances.Num(); ++i)
	{
		EditorInstances[i].SaveSequenceData();
	}
}

void FTrueSkyPlugin::SEditorInstance::SaveSequenceData()
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
		check( FTrueSkyPlugin::Instance );
		check( FTrueSkyPlugin::Instance->GetSequence );
		if ( char* const OutputText = FTrueSkyPlugin::Instance->GetSequence(EditorWindowHWND, Local::AllocString) )
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
void FTrueSkyPlugin::OnSequenceChangeCallback(HWND OwnerHWND,const char *txt)
{
	check( Instance );
	if ( SEditorInstance* const EditorInstance = Instance->FindEditorInstance(OwnerHWND) )
	{
		EditorInstance->SaveSequenceData();
	}
}
void FTrueSkyPlugin::OnTimeChangedCallback(HWND OwnerHWND,float t)
{
	check( Instance );
	if ( SEditorInstance* const EditorInstance = Instance->FindEditorInstance(OwnerHWND) )
	{
		//EditorInstance->SaveSequenceData();
	}
}


::UINT FTrueSkyPlugin::MessageId = 0;

LRESULT CALLBACK FTrueSkyPlugin::EditorWindowWndProc(HWND hWnd, ::UINT uMsg, WPARAM wParam, LPARAM lParam)
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


void FTrueSkyPlugin::InitPaths()
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

void FTrueSkyPlugin::OnToggleRendering()
{
	if ( UTrueSkySequenceAsset* const ActiveSequence = GetActiveSequence() )
	{
		InitPaths();
		if(RendererInitialized)
		{
			SetRenderingEnabled(!RenderingEnabled);
		}
		else
		{
			if(InitRenderingInterface())
				SetRenderingEnabled(true);
		}
		if(RenderingEnabled)
		{
			if(ActiveSequence->SequenceText.Num()>0)
			{
				std::string SequenceInputText;
				SequenceInputText = std::string( (const char*)ActiveSequence->SequenceText.GetData() );
				StaticSetSequence( SequenceInputText );
			}
		}
	}
	else if(RenderingEnabled)
	{
		// no active sequence, so disable rendering
		SetRenderingEnabled(false);
	}
}

IMPLEMENT_TOGGLE(ShowFades)
IMPLEMENT_TOGGLE(ShowCompositing)
IMPLEMENT_TOGGLE(Show3DCloudTextures)
IMPLEMENT_TOGGLE(Show2DCloudTextures)

bool FTrueSkyPlugin::IsToggleRenderingEnabled()
{
	if(GetActiveSequence())
	{
		return true;
	}
	// No active sequence found!
	SetRenderingEnabled(false);
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
		SequenceActor=GWorld->SpawnActor<ATrueSkySequenceActor>(ATrueSkySequenceActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	}
	else
	{
		// Sequence actor already exists -- error message?
	}
}

void FTrueSkyPlugin::OnSequenceDestroyed()
{
}

bool FTrueSkyPlugin::IsAddSequenceEnabled()
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

void FTrueSkyPlugin::UpdateFromActor()
{
	if(actorCrossThreadProperties.Destroyed)
		actorCrossThreadProperties.Visible=false;
	if(actorCrossThreadProperties.Visible!=RenderingEnabled)
	{
		OnToggleRendering();
	}
	if(RenderingEnabled)
	{
		SetRenderFloat("SimpleCloudShadowing",actorCrossThreadProperties.SimpleCloudShadowing);
		SetRenderFloat("SimpleCloudShadowSharpness",actorCrossThreadProperties.SimpleCloudShadowSharpness);
	}
}

UTrueSkySequenceAsset* FTrueSkyPlugin::GetActiveSequence()
{
	return actorCrossThreadProperties.activeSequence;
}


void FTrueSkyPlugin::OpenEditor(UTrueSkySequenceAsset* const TrueSkySequence)
{
	if ( TrueSkySequence == NULL )
		return;

	SEditorInstance* EditorInstance = FindEditorInstance(TrueSkySequence);
	if ( EditorInstance == NULL )
	{
		InitPaths();

		InitRenderingInterface();

		void* const Env = GetActiveSequence() == TrueSkySequence ? (StaticGetEnvironment?StaticGetEnvironment():NULL) : NULL;

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
}




