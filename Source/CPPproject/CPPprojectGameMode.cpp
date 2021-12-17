// Copyright Epic Games, Inc. All Rights Reserved.

#include "CPPprojectGameMode.h"
#include "CPPprojectCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACPPprojectGameMode::ACPPprojectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
