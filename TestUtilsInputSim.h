#pragma once

#include "CoreMinimal.h"
#include "TestUtilsMouseTicker.h"
#include "TestUtilsInputSim.h"
#include "TestUtilsFlow.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/GenericWindow.h"
#include "InputCoreTypes.h"

class FTestUtilsInputSim
{
public:
	
	static bool SlateMousePress(const EMouseButtons::Type MouseButton)
	{
		FSlateApplication& SlateApp = FSlateApplication::Get();
		TSharedPtr<FGenericWindow> GenWindow;
		return SlateApp.OnMouseDown(GenWindow, MouseButton);
	}

	static bool SlateMouseRelease(const EMouseButtons::Type MouseButton)
	{
		FSlateApplication& SlateApp = FSlateApplication::Get();
		return SlateApp.OnMouseUp(MouseButton);
	}

	static void SetMousePosition(UWorld* World, const int32 MouseX, const int32 MouseY)
	{
		if (APlayerController* PC = GEngine->GetFirstLocalPlayerController(World))
		{
			PC->SetMouseLocation(MouseX, MouseY);
		}
	}

	static void MoveMouse(UWorld* World, float TargetX, float TargetY, float Time)
	{
		FTestUtilsFlow::RunOnGameThreadLatentAndWait([World, TargetX, TargetY, Time](const FDoneDelegate Done)
		{
			FTestUtilsMouseTicker* MouseTicker = new FTestUtilsMouseTicker;
			MouseTicker->MoveMouse(World, TargetX, TargetY, Time, [Done, MouseTicker]()
			{
				Done.Execute();
				delete MouseTicker;
			});
		});
	}

	static bool SlateKeyPress(const FKey Key)
	{
		const uint32* KeyCode = nullptr;
		const uint32* CharCode = nullptr;
		FInputKeyManager::Get().GetCodesFromKey(Key, KeyCode, CharCode);
		return FSlateApplication::Get().OnKeyDown(KeyCode ? *KeyCode : 0, CharCode ? *CharCode : 0, false);
	}

	static bool SlateKeyReleased(const FKey Key)
	{
		const uint32* KeyCode = nullptr;
		const uint32* CharCode = nullptr;
		FInputKeyManager::Get().GetCodesFromKey(Key, KeyCode, CharCode);
		return FSlateApplication::Get().OnKeyUp(KeyCode ? *KeyCode : 0, CharCode ? *CharCode : 0, false);
	}

	static bool KeyEvent(FKey Key, EInputEvent InputEvent)
	{
		if (GEngine)
		{
			if (GEngine->GameViewport)
			{
				if (FViewport* Viewport = GEngine->GameViewport->Viewport)
				{
					if (FViewportClient* ViewportClient = Viewport->GetClient())
					{
						return ViewportClient->InputKey(FInputKeyEventArgs(Viewport, 0, EKeys::LeftMouseButton, EInputEvent::IE_Pressed));
					}
				}
			}
		}
		return false;
	}
};

