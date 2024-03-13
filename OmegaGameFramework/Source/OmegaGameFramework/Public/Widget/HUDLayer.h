// Copyright Studio Syndicat 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"

#include "HUDLayer.generated.h"

class UOmegaPlayerSubsystem;

/**
 * 
 */
UCLASS(HideFunctions = (Construct, Destruct))
class OMEGAGAMEFRAMEWORK_API UHUDLayer : public UUserWidget
{
	GENERATED_BODY()

private:

	UPROPERTY()
	bool bRemoving;

public:
	virtual void NativeConstruct() override;
	
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void LayerAdded(class APlayerController* OwningPlayer, UObject* Context);

	//Will run the removal animation, and then clea the HUD Layer once finished.
	UFUNCTION(BlueprintCallable, Category="HUD")
	void RemoveHUDLayer();

	//UFUNCTION(BlueprintImplementableEvent)
	//void LayerRemoved();

	UPROPERTY(EditDefaultsOnly, Category = "HUD Layer")
	FGameplayTagContainer HUDTags;
	
	UPROPERTY(EditDefaultsOnly, Category = "HUD Layer")
	int32 SlateLayerIndex;
	
	UPROPERTY(EditDefaultsOnly, Category = "HUD Layer")
	bool bReverseCloseAnim=true;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnGlobalEvent(FName Event, UObject* Context);
	UFUNCTION()
	void Local_BindGlobalEvent();

	UFUNCTION(BlueprintImplementableEvent)
	void OnGameplayMessage(UOmegaGameplayMessage* Message, FGameplayTag MessageCategory);
	UFUNCTION(BlueprintImplementableEvent)
	void OnInputMethodChanged(bool bIsGamepad);

	UFUNCTION(BlueprintImplementableEvent)
	UWidgetAnimation* GetAppearAnimation();
	UFUNCTION(BlueprintImplementableEvent)
	UWidgetAnimation* GetRemovedAnimation();

private:

	UFUNCTION()
	void Local_RemoveAnimFinished();


};
