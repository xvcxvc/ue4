#include "TrueSkyPluginPrivatePCH.h"
#include "TrueSkySequenceActor.h"

ATrueSkySequenceActor::ATrueSkySequenceActor(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP),SimpleCloudShadowing(0.5f),Visible(true)
{

}

void ATrueSkySequenceActor::SetTime( float value )
{
	ITrueSkyPlugin::Get().SetRenderFloat( "time", value );
}

FRotator ATrueSkySequenceActor::GetSunRotation()
{
	float azimuth = ITrueSkyPlugin::Get().GetRenderFloat( "SunAzimuthDegrees" );
	float elevation = ITrueSkyPlugin::Get().GetRenderFloat( "SunElevationDegrees" );

	FRotator sunRotation( -elevation, -azimuth, 0.0f );
	return sunRotation;
}

FLinearColor ATrueSkySequenceActor::GetSunColor()
{
	float r = ITrueSkyPlugin::Get().GetRenderFloat( "SunIrradianceRed" );
	float g = ITrueSkyPlugin::Get().GetRenderFloat( "SunIrradianceGreen" );
	float b = ITrueSkyPlugin::Get().GetRenderFloat( "SunIrradianceBlue" );

	return 0.5f * FLinearColor( r, g, b );
}
	
void ATrueSkySequenceActor::Tick(float DeltaTime)
{
}

void ATrueSkySequenceActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	ITrueSkyPlugin::Get().PropertiesChanged(this);
}