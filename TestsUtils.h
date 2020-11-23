#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"

class FTestTicker
{
public:



	void MoveMouse(float InTargetX, float InTargetY, float InTime, TUniqueFunction<void()>&& InCallback)
	{
		static float TickValue = .1f;

		UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetMouseLocation(0, 0);

		TargetX = InTargetX;
		TargetY = InTargetY;
		TimeLeft = InTime;
		Callback = MoveTemp(InCallback);

		UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetMousePosition(CurrentPosX, CurrentPosY);
		XStep = ((TargetX - CurrentPosX) / InTime) * TickValue;
		YStep = ((TargetY - CurrentPosY) / InTime) * TickValue;

		GetWorld()->GetTimerManager().SetTimer(TH, [this]()
		{
			TimeLeft -= TickValue;

			if (TimeLeft <= 0)
			{
				UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetMouseLocation(TargetX, TargetY);
				GetWorld()->GetTimerManager().ClearTimer(TH);
				Callback();
			}
			else
			{
				CurrentPosX += XStep;
				CurrentPosY += YStep;
				UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetMouseLocation(CurrentPosX, CurrentPosY);
			}
		},
		TickValue, true);
	}

private:

	UWorld* GetWorld()
	{
		if (GEngine)
		{
			if (FWorldContext* WorldContext = GEngine->GetWorldContextFromPIEInstance(0))
			{
				return WorldContext->World();
			}
		}

		return nullptr;
	}

	float CurrentPosX, CurrentPosY;
	float TargetX, TargetY;
	float XStep, YStep;
	float TimeLeft;
	FTimerHandle TH;
	TUniqueFunction<void()> Callback;
};

class FTestsUtils
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
			if (FWorldContext* WorldContext = GEngine->GetWorldContextFromPIEInstance(0))
			{
				return WorldContext->World();
			}
		}

		return nullptr;
	}

	static void Exit()
	{
		if (UWorld* World = FTestsUtils::GetWorld())
		{
			if (APlayerController* TargetPC = UGameplayStatics::GetPlayerController(World, 0))
			{
				TargetPC->ConsoleCommand(TEXT("Exit"), true);
			}
		}
	}

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

	static void RunOnGameThreadLatentAndWait(TUniqueFunction<void(const FDoneDelegate TestDone)> Function)
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

	template<typename T>
	static TSubclassOf<T> GetObjectClass(const TCHAR* ObjectPath)
	{
		UClass* LoadedClass = StaticLoadClass(T::StaticClass(), NULL, ObjectPath);
		if (LoadedClass)
		{
			LoadedClass->AddToRoot();
			return LoadedClass;
		}
		return nullptr;
	}

	static bool InputKey(const FName& KeyName, EInputEvent InputEvent)
	{
		if (GEngine)
		{
			if (GEngine->GameViewport)
			{
				if (FViewport* Viewport = GEngine->GameViewport->Viewport)
				{
					if (FViewportClient* ViewportClient = Viewport->GetClient())
					{
						return ViewportClient->InputKey(FInputKeyEventArgs(Viewport, 0, KeyName, InputEvent));
					}
				}
			}
		}
		return false;
	}

	template<typename T>
	static T* GetFirstActorOfClass(TSubclassOf<AActor> Class)
	{
		if (UWorld* World = FTestsUtils::GetWorld())
		{
			TArray<AActor*> OutActors;
			UGameplayStatics::GetAllActorsOfClass(World, Class, OutActors);
			if (OutActors.Num() > 0)
			{
				if (T* Result = Cast<T>(OutActors[0]))
				{
					return Result;
				}
			}
		}
		return nullptr;
	}

	template<typename T>
	static T* GetFirstWidgetOfClass(TSubclassOf<UUserWidget> Class)
	{
		if (UWorld* World = FTestsUtils::GetWorld())
		{
			TArray<UUserWidget*> FoundWidgets;
			UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, FoundWidgets, Class, true);
			if (FoundWidgets.Num() > 0)
			{
				if (T* Result = Cast<T>(FoundWidgets[0]))
				{
					return Result;
				}
			}
		}
		return nullptr;
	}


	static void MoveMouse(float TargetX, float TargetY, float Time)
	{
		RunOnGameThreadLatentAndWait([TargetX, TargetY, Time](const FDoneDelegate Done)
		{
			FTestTicker* MouseTicker = new FTestTicker;
			MouseTicker->MoveMouse(TargetX, TargetY, Time, [Done, MouseTicker]()
			{
				Done.Execute();
				delete MouseTicker;
			});
		});
	}

	static void SlateMousePressed(FKey MouseKey)
	{
		// Get our slate application
		FSlateApplication& SlateApp = FSlateApplication::Get();

		// Create a pointer event
		FPointerEvent MouseDownEvent(
			0,
			SlateApp.CursorPointerIndex,
			SlateApp.GetCursorPos(),
			SlateApp.GetLastCursorPos(),
			SlateApp.GetPressedMouseButtons(),
			MouseKey,
			0,
			SlateApp.GetPlatformApplication()->GetModifierKeys()
		);

		// Send the mouse event to the slate handler
		TSharedPtr<FGenericWindow> GenWindow;
		SlateApp.ProcessMouseButtonDownEvent(GenWindow, MouseDownEvent);
	}

	static void SlateMouseReleased(FKey MouseKey)
	{
		// Get our slate application
		FSlateApplication& SlateApp = FSlateApplication::Get();

		// Create a pointer event
		FPointerEvent MouseUpEvent(
			0,
			SlateApp.CursorPointerIndex,
			SlateApp.GetCursorPos(),
			SlateApp.GetLastCursorPos(),
			SlateApp.GetPressedMouseButtons(),
			MouseKey,
			0,
			SlateApp.GetPlatformApplication()->GetModifierKeys()
		);

		// Send the mouse event to the slate handler
		SlateApp.ProcessMouseButtonUpEvent(MouseUpEvent);
	}

	template <typename T>
	static T* GetWidgetOfName(UUserWidget* InUserWidget, const FString& InName)
	{
		if (InUserWidget)
		{
			TArray<UWidget*> Widgets;
			InUserWidget->WidgetTree->GetAllWidgets(Widgets);
			for (UWidget* Widget : Widgets)
			{
				if (T* W = Cast<T>(Widget))
				{
					if (W->GetName() == TEXT("CatImg"))
					{
						return W;
					}
				}
			}
		}
		return nullptr;
	}


};

