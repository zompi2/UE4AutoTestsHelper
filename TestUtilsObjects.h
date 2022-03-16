#pragma once

#include "CoreMinimal.h"
#include "Utils/TestUtilsCommon.h"
#include "Blueprint/WidgetTree.h"

class FTestUtilsObjects
{
public:
	
	template<typename T>
	static int32 CountActorsOfClass(UWorld* World)
	{
		int32 Result = 0;
		for (TActorIterator<T> It(World, T::StaticClass()); It; ++It)
		{
			Result++;			
		}
		return Result;
	}

	template<typename T>
	static T* GetFirstWidgetOfClass(UWorld* World, TSubclassOf<UUserWidget> Class)
	{
		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, FoundWidgets, Class, true);
		if (FoundWidgets.Num() > 0)
		{
			if (T* Widget = Cast<T>(FoundWidgets[0]))
			{
				return Widget;
			}
		}
		return nullptr;
	}

	template<typename T>
	static T* GetFirstWidgetOfClass(UWorld* World)
	{
		return GetFirstWidgetOfClass<T>(World, T::StaticClass());
	}

	template <typename T>
	static T* GetWidgetWithName(UUserWidget* ParentWidget, const FString& Name)
	{
		if (ParentWidget)
		{
			TArray<UWidget*> Widgets;
			ParentWidget->WidgetTree->GetAllWidgets(Widgets);
			for (UWidget* Widget : Widgets)
			{
				if (Widget->GetName() == Name)
				{
					if (T* CastedWidget = Cast<T>(Widget))
					{
						return CastedWidget;
					}
				}
			}
		}
		return nullptr;
	}
};

