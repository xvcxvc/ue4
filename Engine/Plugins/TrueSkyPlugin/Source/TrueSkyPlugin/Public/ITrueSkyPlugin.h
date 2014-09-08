// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"


/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules 
 * within this plugin.
 */
class ITrueSkyPlugin : public IModuleInterface
{

public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline ITrueSkyPlugin& Get()
	{
		return FModuleManager::LoadModuleChecked< ITrueSkyPlugin >( "TrueSkyPlugin" );
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "TrueSkyPlugin" );
	}

	virtual void	SetRenderFloat(const FString& name, float value) = 0;
	virtual float	GetRenderFloat(const FString& name) const = 0;
	virtual void	SetRenderInt(const FString& name, int value) = 0;
	virtual int		GetRenderInt(const FString& name) const = 0;

	virtual void	SetKeyframeFloat(unsigned,const FString& name, float value) = 0;
	virtual float	GetKeyframeFloat(unsigned,const FString& name) const = 0;
	virtual void	SetKeyframeInt(unsigned,const FString& name, int value) = 0;
	virtual int		GetKeyframeInt(unsigned,const FString& name) const = 0;

	virtual void	SetRenderBool(const FString& name, bool value) = 0;
	virtual bool	GetRenderBool(const FString& name) const = 0;
	virtual void	SetRenderString(const FString& name, const FString&  value)=0;
	virtual FString	GetRenderString(const FString& name) const =0;
	virtual void	TriggerAction(const FString& name) = 0;
	virtual void	SetRenderingEnabled(bool) = 0;
	
	virtual class	UTrueSkySequenceAsset* GetActiveSequence()=0;
	virtual void*	GetRenderEnvironment()=0;
	virtual void	OnToggleRendering() = 0;
};

