#pragma once
#include "TrueSkyPluginPrivatePCH.h"

struct ActorCrossThreadProperties
{
	ActorCrossThreadProperties()
		:Destroyed(false)
		,Visible(false)
		,SimpleCloudShadowing(0.0f)
		,activeSequence(NULL)
	{
	}
	bool Destroyed;
	bool Visible;
	float SimpleCloudShadowing;
	float SimpleCloudShadowSharpness;
	class UTrueSkySequenceAsset *activeSequence;
};
extern ActorCrossThreadProperties *GetActorCrossThreadProperties();