#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"

class FTestUtilsFlow
{
public:
	static void Wait(float TimeInSeconds)
	{
		float StartTime = FPlatformTime::Seconds();
		while (FPlatformTime::Seconds() - StartTime < TimeInSeconds)
		{
			FPlatformProcess::Sleep(.1f);
		}
	}

	static void RunOnGameThreadAndWait(TUniqueFunction<void()> Function)
	{
		checkf(IsInGameThread() == false, TEXT("You are not supposed to run this on Game Thread. Use only in LatentIt."));

		bool bWaitForExecute = true;
		AsyncTask(ENamedThreads::GameThread, [&bWaitForExecute, &Function]()
		{
			Function();
			bWaitForExecute = false;
		});
		while (bWaitForExecute)
		{
			FPlatformProcess::Sleep(.1f);
		}
	}

	static void RunOnGameThreadLatentAndWait(TUniqueFunction<void(const FDoneDelegate)> Function)
	{
		checkf(IsInGameThread() == false, TEXT("You are not supposed to run this on Game Thread. Use only in LatentIt."));

		bool bWaitForExecute = true;
		AsyncTask(ENamedThreads::GameThread, [&bWaitForExecute, &Function]()
		{
			Function(FDoneDelegate::CreateLambda([&bWaitForExecute]()
			{
				bWaitForExecute = false;
			}));
		});
		while (bWaitForExecute)
		{
			FPlatformProcess::Sleep(.1f);
		}
	}

	static void WaitForPlayer(const UWorld* World, bool bEnsureInputMode)
	{
		checkf(IsInGameThread() == false, TEXT("You are not supposed to run this on Game Thread. Use only in LatentIt."));

		bool bWaitsForPlayer = true;
		while (bWaitsForPlayer)
		{
			RunOnGameThreadAndWait([World, bEnsureInputMode, &bWaitsForPlayer]()
			{
				if (APlayerController* PC = GEngine->GetFirstLocalPlayerController(World))
				{
					if (PC->GetPawn() != nullptr)
					{
						if (bEnsureInputMode)
						{
							PC->SetInputMode(FInputModeGameOnly());
						}
						bWaitsForPlayer = false;
					}
				}
			});
			FPlatformProcess::Sleep(.1f);
		}
	}
};
