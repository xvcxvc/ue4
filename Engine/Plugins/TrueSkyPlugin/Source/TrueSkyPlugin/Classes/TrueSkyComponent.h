#pragma once

#include "Components/ActorComponent.h"
#include "TrueSkyComponent.generated.h"
UCLASS(ClassGroup=Rendering,hidecategories=(Object, ActorComponent))
class UTrueSkyComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:
	// Begin UActorComponent interface.
	virtual void OnRegister() override;
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void OnUnregister() override;
	// Begin UActorComponent interface.

protected:
};

