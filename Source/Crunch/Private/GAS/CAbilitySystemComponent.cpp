// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CAbilitySystemComponent.h"

void UCAbilitySystemComponent::ApplyInitialEffects()
{
	for (const TSubclassOf<UGameplayEffect>& Effects: InitialEffects)
	{
		FGameplayEffectSpecHandle InitialEffectsSpecHandle = MakeOutgoingSpec(Effects, 1, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*InitialEffectsSpecHandle.Data.Get());
	}
}
