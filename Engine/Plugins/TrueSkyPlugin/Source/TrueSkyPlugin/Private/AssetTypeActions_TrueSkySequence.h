#pragma once

#include "AssetToolsPrivatePCH.h"
#include "Toolkits/IToolkitHost.h"
#include "TrueSkySequenceAsset.h"

class FAssetTypeActions_TrueSkySequence : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions Implementation
	virtual FText GetName() const OVERRIDE { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_TrueSkySequence", "TrueSky Sequence Asset"); }
	virtual FColor GetTypeColor() const OVERRIDE { return FColor(255,192,128); }
	virtual UClass* GetSupportedClass() const OVERRIDE { return UTrueSkySequenceAsset::StaticClass(); }
	virtual bool HasActions ( const TArray<UObject*>& InObjects ) const OVERRIDE { return true; }
	virtual void GetActions( const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder ) OVERRIDE {}
	virtual void OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>() ) OVERRIDE;
	virtual uint32 GetCategories() OVERRIDE { return EAssetTypeCategories::Misc; }

private:
	/** Handler for when Edit is selected */
	void ExecuteEdit(TArray<TWeakObjectPtr<UTrueSkySequenceAsset>> Objects);
};