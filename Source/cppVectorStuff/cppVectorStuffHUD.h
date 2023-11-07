// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "cppVectorStuffHUD.generated.h"

UCLASS()
class AcppVectorStuffHUD : public AHUD
{
	GENERATED_BODY()

public:
	AcppVectorStuffHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

