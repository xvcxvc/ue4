#pragma once

#include "TrueSkySequenceAsset.generated.h"

UCLASS(MinimalAPI)
class UTrueSkySequenceAsset : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY()
	TArray<uint8> SequenceText;
};
