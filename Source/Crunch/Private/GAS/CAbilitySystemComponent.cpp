// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CAbilitySystemComponent.h"

void UCAbilitySystemComponent::ApplyInitialEffects()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	for (const TSubclassOf<UGameplayEffect>& Effects: InitialEffects)
	{
		FGameplayEffectSpecHandle InitialEffectsSpecHandle = MakeOutgoingSpec(Effects, 1, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*InitialEffectsSpecHandle.Data.Get());
	}
}

void UCAbilitySystemComponent::GrantAbilities()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : Abilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityClass, 0, -1, nullptr)); // 0 means not learned yet.
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : BasicAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, -1, nullptr)); // 1 means learned it yet.
	}
}
