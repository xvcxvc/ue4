#pragma once

#include "TrueSkySequenceActor.generated.h"

UCLASS(hideCategories=(Actor, Advanced, Display, Events, Object, Attachment, Movement, Collision, Rendering, Input), MinimalAPI, NotBlueprintable, notplaceable)
class ATrueSkySequenceActor : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category=TrueSky)
	UTrueSkySequenceAsset* ActiveSequence;
};
