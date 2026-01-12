// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "GA_Combo.generated.h"


UCLASS()
class UGA_Combo : public UCGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Combo();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	static FGameplayTag GetComboChangedEventTag();
	static FGameplayTag GetComboChangedEndTag();
	static FGameplayTag GetComboEventTargetTag();

private:
	void SetupInputComboPress();

	UFUNCTION()
	void HandleInputPress(float TimeWaited);

	void TryCommitCombo();

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
	TSubclassOf<UGameplayEffect> DefaultDamageEffectMap;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
	TMap<FName, TSubclassOf<UGameplayEffect>> DamageEffectMap;

	TSubclassOf<UGameplayEffect> GetDamageEffectforcurrentCombo() const;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ComboMontage;

	UFUNCTION()
	void ComboChangeEventReceived(FGameplayEventData GameplayData);

	UFUNCTION()
	void DoDamage(FGameplayEventData GameplayData);

	FName NextComboName;
};
