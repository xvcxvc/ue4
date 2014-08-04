#pragma once

#include "TrueSkySequenceActor.generated.h"

UCLASS(hideCategories=(Actor, Advanced, Display, Events, Object, Attachment, Movement, Collision, Rendering, Input), MinimalAPI, Blueprintable, notplaceable)
class ATrueSkySequenceActor : public AActor
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, Category=Defaults)
	void SetTime( float value );

	UFUNCTION(BlueprintCallable, Category=Defaults)
	FRotator GetSunRotation();

	UFUNCTION(BlueprintCallable, Category=Defaults)
	FLinearColor GetSunColor();

	UPROPERTY(EditAnywhere, Category=TrueSky)
	UTrueSkySequenceAsset* ActiveSequence;

	UPROPERTY(EditAnywhere, Category=TrueSky)
	UTextureRenderTarget2D* CloudShadowRenderTarget;

	UPROPERTY(EditAnywhere, Category=TrueSky)
	float SimpleCloudShadowing;
	
	virtual void Tick(float DeltaTime) override;
};
