#pragma once
#include "TrueSkyPluginPrivatePCH.h"

struct ActorCrossThreadProperties
{
	ActorCrossThreadProperties()
		:Visible(false)
		,SimpleCloudShadowing(0.0f)
		,activeSequence(NULL)
	{
	}
	bool Visible;
	float SimpleCloudShadowing;
	float SimpleCloudShadowSharpness;
	class UTrueSkySequenceAsset *activeSequence;
};
extern ActorCrossThreadProperties *GetActorCrossThreadProperties();