#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"

class FTestUtilsMouseTicker
{
public:

	void MoveMouse(UWorld* InWorld, float InTargetX, float InTargetY, float InTime, TUniqueFunction<void()>&& InCallback);

private:

	float CurrentPosX, CurrentPosY;
	float TargetX, TargetY;
	float XStep, YStep;
	float TimeLeft;
	FTimerHandle TH;
	TUniqueFunction<void()> Callback;
	UWorld* World;
};