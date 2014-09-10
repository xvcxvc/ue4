#if UE_EDITOR
#include "TrueSkyEditorPluginPrivatePCH.h"
#include "AssetTypeActions_TrueSkySequence.h"
#include "TrueSkySequenceAsset.h"

void FAssetTypeActions_TrueSkySequence::OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor )
{
	if ( InObjects.Num() > 0 )
	{
		if ( UTrueSkySequenceAsset* const TrueSkyAsset = Cast<UTrueSkySequenceAsset>(InObjects[0]) )
		{
			ITrueSkyEditorPlugin::Get().OpenEditor( TrueSkyAsset );
		}
	}
}
#endif