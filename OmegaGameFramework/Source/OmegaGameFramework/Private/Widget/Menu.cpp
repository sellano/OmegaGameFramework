// Copyright Studio Syndicat 2021. All Rights Reserved.


#include "Widget/Menu.h"

#include "OmegaStyle_Slate.h"
#include "Engine/GameInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Subsystems/OmegaSubsystem_GameManager.h"
#include "Subsystems/OmegaSubsystem_Gameplay.h"
#include "Subsystems/OmegaSubsystem_Player.h"


void UMenu::OpenMenu(FGameplayTagContainer Tags, UObject* Context, APlayerController* PlayerRef, const FString& Flag)
{
	
	SetOwningPlayer(PlayerRef);
	if (!bIsOpen)
	{
		//BIND EVENTS
		GetGameInstance()->GetSubsystem<UOmegaMessageSubsystem>()->OnGameplayMessage.AddDynamic(this, &UMenu::OnGameplayMessage);
		GetOwningLocalPlayer()->GetSubsystem<UOmegaPlayerSubsystem>()->OnInputDeviceChanged.AddDynamic(this, &UMenu::OnInputMethodChanged);
		Local_BindGlobalEvent();

		
		
		TempTags = Tags;
		PrivateInputBlocked = true;
		InputBlock_Remaining=InputBlockDelay;
		SetIsEnabled(true);
		SetVisibility(VisibilityOnOpen);
		
		OnOpened.Broadcast(Tags, Flag);
		AddToPlayerScreen(200);
		MenuOpened(Tags, Context, Flag);
		//ANIMATION

		if(CustomInputMode)
		{
			GetOwningLocalPlayer()->GetSubsystem<UOmegaPlayerSubsystem>()->SetCustomInputMode(CustomInputMode);
		}
		
		if(ParallelGameplaySystem)
		{
			GetWorld()->GetSubsystem<UOmegaGameplaySubsystem>()->ActivateGameplaySystem(ParallelGameplaySystem, this);
		}

		
		if(OpenSound)
		{
			PlaySound(OpenSound);
		}
		else if(UOmegaSlateFunctions::GetCurrentSlateStyle() && UOmegaSlateFunctions::GetCurrentSlateStyle()->Sound_Menu_Open)
		{
			PlaySound(UOmegaSlateFunctions::GetCurrentSlateStyle()->Sound_Menu_Open);
		}
		
		if(GetOpenAnimation())
		{
			if(ReverseOpenAnimation)
			{
				PlayAnimationReverse(GetOpenAnimation());
			}
			else
			{
				PlayAnimationForward(GetOpenAnimation());
			}
		}
		else
		{
			Native_CompleteOpen();
		}
	}
}

void UMenu::CloseMenu(FGameplayTagContainer Tags, UObject* Context, const FString& Flag)
{
	if (bIsOpen && CanCloseMenu(Tags,Context,Flag))
	{
		bIsOpen = false;
		PrivateInputBlocked = true;
		
		TempTags = Tags;
		MenuClosed(TempTags, Flag);
		OnClosed.Broadcast(TempTags, Context, Flag);

		//Handle Subsystem
		UOmegaPlayerSubsystem* SubsystemRef = GetOwningLocalPlayer()->GetSubsystem<UOmegaPlayerSubsystem>();
		SubsystemRef->RemoveMenuFromActiveList(this);
		const bool LastMenu = !SubsystemRef->OpenMenus.IsValidIndex(0);
		SubsystemRef->OnMenuClosed.Broadcast(this, TempTags, LastMenu);

		if(SubsystemRef->FocusMenu && SubsystemRef->FocusMenu==this)
		{
			SubsystemRef->ClearControlWidget();
		}

		SetVisibility(ESlateVisibility::HitTestInvisible);

		if(CloseSound)
		{
			PlaySound(CloseSound);
		}
		else if(UOmegaSlateFunctions::GetCurrentSlateStyle() && UOmegaSlateFunctions::GetCurrentSlateStyle()->Sound_Menu_Close)
		{
			PlaySound(UOmegaSlateFunctions::GetCurrentSlateStyle()->Sound_Menu_Close);
		}
		
		//ANIMATION

		bIsClosing = true;
		if(GetCloseAnimation())
		{
			if(ReverseCloseAnimation)
			{
				PlayAnimationReverse(GetCloseAnimation());
			}
			else
			{
				PlayAnimationForward(GetCloseAnimation());
			}
		}
		else
		{
			Native_CompleteClose();
		}
	}
}

void UMenu::OnAnimationFinished_Implementation(const UWidgetAnimation* MovieSceneBlends)
{
	
	if(MovieSceneBlends==GetOpenAnimation() && !bIsClosing)
	{
		UE_LOG(LogTemp, Warning, TEXT("Menu CLOSE Complete") );
		Native_CompleteOpen();
	}
	else if (MovieSceneBlends==GetCloseAnimation() && bIsClosing)
	{
		Native_CompleteClose();
	}
	Super::OnAnimationFinished_Implementation(MovieSceneBlends);
}


void UMenu::NativeConstruct()
{
	/*
	//Try Set Close Anim
	if(GetCloseAnimation())
	{
		CloseDelegate.BindUFunction(this, "Native_CompleteClose");
		BindToAnimationFinished(GetCloseAnimation(), CloseDelegate);
	}
	
	//Try Set Open Anim
	if(GetOpenAnimation())
	{
		OpenDelegate.BindUFunction(this, "Native_CompleteOpen");
		BindToAnimationFinished(GetOpenAnimation(), OpenDelegate);
		
	}*/
	
	Super::NativeConstruct();
}

void UMenu::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	if(InputBlock_Remaining > 0.0)
	{
		InputBlock_Remaining=InputBlock_Remaining-InDeltaTime;
	}
	
	float target_val=0.0;
	if(bIsOpen) { target_val=1.0; }
	if(target_val != OpenCloseInterp_Value)
	{
		isPlayingOpenCloseInterp=true;
		OpenCloseInterp_Value=UKismetMathLibrary::FInterpTo_Constant(OpenCloseInterp_Value,target_val,InDeltaTime,1.0/OpenCloseInterpTime);
		if(AutoInterpOpacityOnOpenClose)
		{
			SetRenderOpacity(OpenCloseInterp_Value);
		}
		UpdateOpenCloseInterp(OpenCloseInterp_Value);
	}
	else
	{
		isPlayingOpenCloseInterp=false;
		if(bIsClosing)
		{
			Native_CompleteClose();
		}
	}
	
	Super::NativeTick(MyGeometry, InDeltaTime);
}


void UMenu::Native_CompleteOpen()
{
	bIsOpen = true;
	PrivateInputBlocked = false;
}

bool UMenu::CanCloseMenu_Implementation(FGameplayTagContainer Tags, UObject* Context, const FString& Flag)
{
	return true;
}

void UMenu::Native_CompleteClose()
{
	if(!isPlayingOpenCloseInterp)
	{
		bIsClosing = false;
		PrivateInputBlocked = true;
		SetIsEnabled(false);
		SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogTemp, Warning, TEXT("Menu CLOSE Complete") );
	
		if(ParallelGameplaySystem)
		{
			GetWorld()->GetSubsystem<UOmegaGameplaySubsystem>()->ShutdownGameplaySystem(ParallelGameplaySystem, this);
		}
	
		RemoveFromParent();
	}
}

bool UMenu::InputBlocked_Implementation()
{
	return IsInputBlocked();
}

void UMenu::Local_BindGlobalEvent()
{
	GetWorld()->GetGameInstance()->GetSubsystem<UOmegaGameManager>()->OnGlobalEvent.AddDynamic(this, &UMenu::OnGlobalEvent);
}
