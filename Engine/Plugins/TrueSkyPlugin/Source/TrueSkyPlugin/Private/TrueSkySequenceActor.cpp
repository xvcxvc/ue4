#include "TrueSkyPluginPrivatePCH.h"
#include "TrueSkySequenceActor.h"
#include "ActorCrossThreadProperties.h"

ATrueSkySequenceActor::ATrueSkySequenceActor(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP),SimpleCloudShadowing(0.5f),Visible(true)
{
	trueSkyComponent=ConstructObject<UTrueSkyComponent>(UTrueSkyComponent::StaticClass());
	// We register the TrueSkyComponent. This is created so the Actor (game thread) can talk to the plugin (render thread).
	AddOwnedComponent(trueSkyComponent);
	PrimaryActorTick.bTickEvenWhenPaused	=true;
	PrimaryActorTick.bCanEverTick			=true;
	PrimaryActorTick.bStartWithTickEnabled	=true;
	SetTickGroup( TG_PrePhysics);
	SetActorTickEnabled(true);
}

ATrueSkySequenceActor::~ATrueSkySequenceActor()
{
	ActorCrossThreadProperties *A	=GetActorCrossThreadProperties();
	if(A)
		A->Destroyed=true;
}

void ATrueSkySequenceActor::PostInitProperties()
{
	TransferProperties();
    Super::PostInitProperties();
}

void ATrueSkySequenceActor::Destroyed()
{
	ActorCrossThreadProperties *A	=GetActorCrossThreadProperties();
	if(!A)
		return;
	A->Destroyed=true;
	AActor::Destroyed();
}

void ATrueSkySequenceActor::SetTime( float value )
{
	ITrueSkyPlugin::Get().SetRenderFloat("time",value);
}

FRotator ATrueSkySequenceActor::GetSunRotation() const
{
	float azimuth	=ITrueSkyPlugin::Get().GetRenderFloat("SunAzimuthDegrees");
	float elevation	=ITrueSkyPlugin::Get().GetRenderFloat("SunElevationDegrees");
	FRotator sunRotation(-elevation,-azimuth,0.0f);
	return sunRotation;
}

FLinearColor ATrueSkySequenceActor::GetSunColor() const
{
	float r	=ITrueSkyPlugin::Get().GetRenderFloat("SunIrradianceRed");
	float g	=ITrueSkyPlugin::Get().GetRenderFloat("SunIrradianceGreen");
	float b	=ITrueSkyPlugin::Get().GetRenderFloat("SunIrradianceBlue");
	return 0.5f*FLinearColor( r, g, b );
}

void ATrueSkySequenceActor::TransferProperties()
{
	ActorCrossThreadProperties *A	=GetActorCrossThreadProperties();
	if(!A)
		return;
	A->Destroyed			=false;
	A->Visible				=Visible;
	A->SimpleCloudShadowing	=SimpleCloudShadowing;
	A->activeSequence		=ActiveSequence;
	A->SimpleCloudShadowSharpness=SimpleCloudShadowSharpness;
}
	
void ATrueSkySequenceActor::TickActor(float DeltaTime,enum ELevelTick TickType,FActorTickFunction& ThisTickFunction)
{
	TransferProperties();
}

#if UE_EDITOR
void ATrueSkySequenceActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	TransferProperties();
}
#endif