// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_Combo.h"
#include "UCAbilitySystemStatics.h"
#include "CrunchGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

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
	}
}
