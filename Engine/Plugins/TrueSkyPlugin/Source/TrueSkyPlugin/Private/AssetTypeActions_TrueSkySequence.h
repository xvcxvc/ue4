#pragma once
#if UE_EDITOR
#include "AssetToolsPrivatePCH.h"
#include "Toolkits/IToolkitHost.h"
#include "TrueSkySequenceAsset.h"

class FAssetTypeActions_TrueSkySequence : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_TrueSkySequence", "TrueSky Sequence Asset"); }
	virtual FColor GetTypeColor() const override { return FColor(255,192,128); }
	virtual UClass* GetSupportedClass() const override { return UTrueSkySequenceAsset::StaticClass(); }
	virtual bool HasActions ( const TArray<UObject*>& InObjects ) const override { return true; }
	virtual void GetActions( const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder ) override {}
	virtual void OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>() ) override;
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Misc; }

private:
	/** Handler for when Edit is selected */
	void ExecuteEdit(TArray<TWeakObjectPtr<UTrueSkySequenceAsset>> Objects);
};
#endif