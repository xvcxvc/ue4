// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "ITrueSkyPlugin.h"

// You should place include statements to your module's private header files here.  You only need to
// add includes for headers that are used in most of your module's source files though.

#include "RenderResource.h"
#if UE_EDITOR
#include "UnrealEd.h"
#else
#include "Engine.h"
#endif

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