// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_Combo.h"
#include "UCAbilitySystemStatics.h"
#include "CrunchGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameplayTagsManager.h"

UGA_Combo::UGA_Combo()
{
	/* Uisng Two different Ways to sort the GameplayTags, 
	   If you choose blueprint to input your prefer gameplaytags name, using the first one.
	   (Remember change the rerturn to your customize gameplaytags name!!!)
	   If you would rather save memory and speed up the game, you can direct to use the second one.
	*/

	AbilityTags.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
	BlockAbilitiesWithTag.AddTag(CrunchGameplayTags::Ability_Basicattack);
}

void UGA_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo)) //serve predict actor actions
	{
		UAbilityTask_PlayMontageAndWait* PlayComboMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ComboMontage);

		PlayComboMontageTask->OnBlendOut.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnCancelled.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnCompleted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnInterrupted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->ReadyForActivation();
		
		UAbilityTask_WaitGameplayEvent* WaitComboChangeEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboChangedEventTag(), nullptr, false, false );
		WaitComboChangeEventTask->EventReceived.AddDynamic(this, &UGA_Combo::ComboChangeEventReceived);
		WaitComboChangeEventTask->ReadyForActivation();
	}
}

FGameplayTag UGA_Combo::GetComboChangedEventTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Combo.Change");
}

FGameplayTag UGA_Combo::GetComboChangedEndTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Combo.Change.end");
}

void UGA_Combo::ComboChangeEventReceived(FGameplayEventData Data)
{
	FGameplayTag EventTag = Data.EventTag;

	if (EventTag == GetComboChangedEndTag())
	{
		NextComboName = NAME_None;
		UE_LOG(LogTemp, Warning, TEXT("Next combo is end."));
		return;
	}

	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
	NextComboName = TagNames.Last();

	UE_LOG(LogTemp, Warning, TEXT("Next combo is now : %s"), *NextComboName.ToString());
}
