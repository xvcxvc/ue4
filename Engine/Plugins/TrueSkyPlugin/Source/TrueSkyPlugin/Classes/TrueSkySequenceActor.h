#pragma once

#include "Components/PrimitiveComponent.h"
#include "TrueSkySequenceActor.generated.h"

UCLASS(ClassGroup=Rendering, hidecategories=Object, hidecategories=Physics, hidecategories=Collision, showcategories=Trigger, editinlinenew, meta=(BlueprintSpawnableComponent))
class ENGINE_API UTrueSkyComponent : public UPrimitiveComponent
{
	GENERATED_UCLASS_BODY()

};


UCLASS(hideCategories=(Actor, Advanced, Display, Events, Object, Attachment, Movement, Collision, Rendering, Input), MinimalAPI, Blueprintable, notplaceable)
class ATrueSkySequenceActor : public AActor
{
	GENERATED_UCLASS_BODY()
		
	UPROPERTY(EditAnywhere, Category=TrueSky)
	FString LicenceKey;

	UFUNCTION(BlueprintCallable, Category=TrueSky)
	void SetTime( float value );

	UFUNCTION(BlueprintCallable, BlueprintPure,Category=TrueSky)
	FRotator GetSunRotation();

	UFUNCTION(BlueprintCallable, BlueprintPure,Category=TrueSky)
	FLinearColor GetSunColor();

	UPROPERTY(EditAnywhere, Category=TrueSky)
	UTrueSkySequenceAsset* ActiveSequence;

	//UPROPERTY(EditAnywhere, Category=TrueSky)
	//UTextureRenderTarget2D* CloudShadowRenderTarget;

	UPROPERTY(EditAnywhere, Category=TrueSky,meta=(ClampMin = "0.0", ClampMax = "1.0"))
	float SimpleCloudShadowing;

	UPROPERTY(EditAnywhere, Category=TrueSky)
	bool Visible;
	
	virtual void Tick(float DeltaTime) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};
