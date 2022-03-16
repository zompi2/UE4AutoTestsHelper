#include "TestUtilsMouseTicker.h"
#include "TestUtilsCommon.h"
#include "TestUtilsInputSim.h"

void FTestUtilsMouseTicker::MoveMouse(UWorld* InWorld, float InTargetX, float InTargetY, float InTime, TUniqueFunction<void()>&& InCallback)
{
	static float TickValue = .1f;

	World = InWorld;

	FTestUtilsInputSim::SetMousePosition(World, 0, 0);

	TargetX = InTargetX;
	TargetY = InTargetY;
	TimeLeft = InTime;
	Callback = MoveTemp(InCallback);

	FTestUtilsInputSim::SetMousePosition(World, CurrentPosX, CurrentPosY);
	XStep = ((TargetX - CurrentPosX) / InTime) * TickValue;
	YStep = ((TargetY - CurrentPosY) / InTime) * TickValue;

	World->GetTimerManager().SetTimer(TH, [this]()
	{
		TimeLeft -= TickValue;

		if (TimeLeft <= 0)
		{
			FTestUtilsInputSim::SetMousePosition(World, TargetX, TargetY);
			World->GetTimerManager().ClearTimer(TH);
			Callback();
		}
		else
		{
			CurrentPosX += XStep;
			CurrentPosY += YStep;
			FTestUtilsInputSim::SetMousePosition(World, CurrentPosX, CurrentPosY);
		}
	}, TickValue, true);
}