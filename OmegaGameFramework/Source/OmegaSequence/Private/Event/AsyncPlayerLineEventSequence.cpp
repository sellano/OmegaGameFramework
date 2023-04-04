﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Event/AsyncPlayLinearEventSequence.h"
#include "Event/OmegaLinearEventInstance.h"
#include "Engine/World.h"

void UAsyncPlayLinearEventSequence::Local_Finish(const FString& Flag)
{
	OnFinished.Broadcast(Flag);
	SetReadyToDestroy();
}

void UAsyncPlayLinearEventSequence::Local_NewEvent(int32 Index, UOmegaLinearEvent* EventRef)
{
	NewEvent.Broadcast(Index,EventRef);
}

void UAsyncPlayLinearEventSequence::Activate()
{
	if(EventData.Events.Num()<=0)
	{
		Local_Finish("Empty");
	}
	UOmegaLinearEventInstance* TempInst = SubsystemRef->PlayLinearEvent(EventData, Local_StartingIndex);
	TempInst->OnEventSequenceFinish.AddDynamic(this, &UAsyncPlayLinearEventSequence::Local_Finish);
	TempInst->OnEventUpdated.AddDynamic(this, &UAsyncPlayLinearEventSequence::Local_NewEvent);
}

UAsyncPlayLinearEventSequence* UAsyncPlayLinearEventSequence::PlayLinearEventSequence(UObject* WorldContextObject, FLinearEventSequence Events, int32 StartingIndex)
{
	UAsyncPlayLinearEventSequence* NewEvent = NewObject<UAsyncPlayLinearEventSequence>();
	NewEvent->SubsystemRef = WorldContextObject->GetWorld()->GetSubsystem<UOmegaLinearEventSubsystem>();
	NewEvent->EventData = Events;
	NewEvent->Local_StartingIndex = StartingIndex;
	return  NewEvent;
}
