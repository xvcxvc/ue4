#pragma once

#include "TrueSkyComponent.h"
#include "TrueSkySequenceActor.generated.h"


UCLASS(hideCategories=(Actor, Advanced, Display, Events, Object, Attachment, Movement, Collision, Rendering, Input), MinimalAPI, Blueprintable, notplaceable)
class ATrueSkySequenceActor : public AActor
{
	GENERATED_UCLASS_BODY()
	~ATrueSkySequenceActor();
	void Destroyed();
	//UPROPERTY(EditAnywhere, Category=TrueSky)
	//FString LicenceKey;

	UFUNCTION(BlueprintCallable, Category=TrueSky)
	void SetTime( float value );

	UFUNCTION(BlueprintCallable, Category=TrueSky)
	FRotator GetSunRotation() const;

	UFUNCTION(BlueprintCallable, Category=TrueSky)
	FLinearColor GetSunColor() const;

	UPROPERTY(EditAnywhere, Category=TrueSky)
	UTrueSkySequenceAsset* ActiveSequence;

	//UPROPERTY(EditAnywhere, Category=TrueSky)
	//UTextureRenderTarget2D* CloudShadowRenderTarget;

	UPROPERTY(EditAnywhere, Category=TrueSky,meta=(ClampMin = "0.0", ClampMax = "1.0"))
	float SimpleCloudShadowing;

	UPROPERTY(EditAnywhere, Category=TrueSky,meta=(ClampMin = "0.0", ClampMax = "1.0"))
	float SimpleCloudShadowSharpness;

	UPROPERTY(EditAnywhere, Category=TrueSky)
	bool Visible;
	void PostInitProperties();
	void TickActor( float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction ) override;
#if UE_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	UTrueSkyComponent *trueSkyComponent;
	void TransferProperties();
};
