#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

class FTestUtilsCommon
{
public:
	static const int32 CommonTestFlags =
		EAutomationTestFlags::EditorContext |
		EAutomationTestFlags::ClientContext |
		EAutomationTestFlags::ProductFilter;

	static UWorld* GetWorld()
	{
		if (GEngine)
		{
			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				if (UWorld* World = WorldContext.World())
				{
					if (World->WorldType == EWorldType::PIE || World->WorldType == EWorldType::Game)
					{
						return World;
					}
				}
			}
		}
		return nullptr;
	}

	static void Exit(const UWorld* World)
	{
		if (APlayerController* PC = GEngine->GetFirstLocalPlayerController(World))
		{
			PC->ConsoleCommand(TEXT("Exit"), true);
		}
	}
};
