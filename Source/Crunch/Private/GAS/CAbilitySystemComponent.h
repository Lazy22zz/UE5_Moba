// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class UCAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	void ApplyInitialEffects();
	void GrantAbilities();

private:
	UPROPERTY(EditDefaultsOnly, Category = "GamePlay Effect")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffects;

	UPROPERTY(EditDefaultsOnly, Category = "GamePlay Ability")
	TArray<TSubclassOf<UGameplayAbility>> BasicAbilities; // basic character's abilities.

	UPROPERTY(EditDefaultsOnly, Category = "GamePlay Ability")
	TArray<TSubclassOf<UGameplayAbility>> Abilities; // special character's own abilities.


};
