#pragma once
#include "TrueSkyPluginPrivatePCH.h"
#include "TrueSkySequenceFactory.generated.h"

UCLASS()
class UTrueSkySequenceFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

	virtual UObject* FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn) override;
	virtual FName GetNewAssetThumbnailOverride() const;
};
#if UE_EDITOR
#endif