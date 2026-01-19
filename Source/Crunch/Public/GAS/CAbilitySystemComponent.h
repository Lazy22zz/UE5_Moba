// Fill out your copyright notice in the Description page of Project Settings.

//GAS 体系内的所有核心逻辑：属性 (Attribute)、标签 (GameplayTag)、能力 (GA)、效果 (GE)、属性计算、权限校验、网络同步。
//它是纯逻辑组件，无任何表现 / 业务行为、无动画、无输入、无角色动作。
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GAS/CAbilityInputID.h"
#include "CAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class UCAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	UCAbilitySystemComponent();

	void ApplyInitialEffects();
	void GrantAbilities();

private:
	void HealthUpdated(const FOnAttributeChangeData& Data);

	UPROPERTY(EditDefaultsOnly, Category = "GamePlay Effect")
	TSubclassOf<UGameplayEffect> DeathEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GamePlay Effect")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffects;

	UPROPERTY(EditDefaultsOnly, Category = "GamePlay Ability")
	TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>> BasicAbilities; // basic character's abilities.

	UPROPERTY(EditDefaultsOnly, Category = "GamePlay Ability")
	TMap<ECAbilityInputID ,TSubclassOf<UGameplayAbility>> LearnedAbilities; // special character's own abilities.


};
