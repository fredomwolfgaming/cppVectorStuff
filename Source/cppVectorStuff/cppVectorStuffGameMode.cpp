// Copyright Epic Games, Inc. All Rights Reserved.

#include "cppVectorStuffGameMode.h"
#include "cppVectorStuffHUD.h"
#include "cppVectorStuffCharacter.h"
#include "UObject/ConstructorHelpers.h"

AcppVectorStuffGameMode::AcppVectorStuffGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AcppVectorStuffHUD::StaticClass();
}
