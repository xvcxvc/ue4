#include "TrueSkyPluginPrivatePCH.h"
#if UE_EDITOR
#include "TrueSkySequenceFactory.h"
#include "TrueSkySequenceAsset.h"
UTrueSkySequenceFactory::UTrueSkySequenceFactory(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UTrueSkySequenceAsset::StaticClass();
}

UObject* UTrueSkySequenceFactory::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	UTrueSkySequenceAsset* NewSequence = CastChecked<UTrueSkySequenceAsset>(StaticConstructObject(UTrueSkySequenceAsset::StaticClass(),InParent,Name,Flags));

	return NewSequence;
}

FName UTrueSkySequenceFactory::GetNewAssetThumbnailOverride() const
{
	return TEXT("ClassThumbnail.TrueSkySequenceAsset");
}
#endif