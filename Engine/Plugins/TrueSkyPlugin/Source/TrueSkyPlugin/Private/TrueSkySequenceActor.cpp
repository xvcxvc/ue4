#include "TrueSkyPluginPrivatePCH.h"
#include "TrueSkySequenceActor.h"

ATrueSkySequenceActor::ATrueSkySequenceActor(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP),SimpleCloudShadowing(0.5f)
{

}

void ATrueSkySequenceActor::SetTime( float value )
{
	ITrueSkyPlugin::Get().SetRenderFloat( "time", value );
	ITrueSkyPlugin::Get().SetRenderFloat( "SimpleCloudShadowing", SimpleCloudShadowing);
}

FRotator ATrueSkySequenceActor::GetSunRotation()
{
	float azimuth = ITrueSkyPlugin::Get().GetRenderFloat( "SunAzimuthDegrees" );
	float elevation = ITrueSkyPlugin::Get().GetRenderFloat( "SunElevationDegrees" );
	ITrueSkyPlugin::Get().SetRenderFloat( "SimpleCloudShadowing", SimpleCloudShadowing);

	FRotator sunRotation( -elevation, -azimuth, 0.0f );
	return sunRotation;
}

FLinearColor ATrueSkySequenceActor::GetSunColor()
{
	float r = ITrueSkyPlugin::Get().GetRenderFloat( "SunIrradianceRed" );
	float g = ITrueSkyPlugin::Get().GetRenderFloat( "SunIrradianceGreen" );
	float b = ITrueSkyPlugin::Get().GetRenderFloat( "SunIrradianceBlue" );
	ITrueSkyPlugin::Get().SetRenderFloat( "SimpleCloudShadowing", SimpleCloudShadowing);

	return 0.5f * FLinearColor( r, g, b );
}
	
void ATrueSkySequenceActor::Tick(float DeltaTime)
{
	ITrueSkyPlugin::Get().SetRenderFloat( "SimpleCloudShadowing", SimpleCloudShadowing);
}

/*
void ATrueSkySequenceActor::SetCloudShadowRenderTarget(class FRenderTarget *t)
{
	ITrueSkyPlugin::Get().SetCloudShadowRenderTarget(t);
}*/