// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"

UCAbilitySystemComponent::UCAbilitySystemComponent()
{
	// 绑定生命值属性变更的委托，值变化时触发HealthUpdated函数
	GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetHealthAttribute()).AddUObject(this, &UCAbilitySystemComponent::HealthUpdated);
}

// 应用初始游戏效果（仅服务端执行）
void UCAbilitySystemComponent::ApplyInitialEffects()
{
	// 非拥有者/非服务端则直接返回
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	// 遍历初始效果列表，逐个应用到自身
	for (const TSubclassOf<UGameplayEffect>& Effects: InitialEffects)
	{
		FGameplayEffectSpecHandle InitialEffectsSpecHandle = MakeOutgoingSpec(Effects, 1, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*InitialEffectsSpecHandle.Data.Get());
	}
}

// 授予技能（仅服务端执行）
void UCAbilitySystemComponent::GrantAbilities()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	// 授予基础技能（参数0表示未解锁状态）
	for (const TPair<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : BasicAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 0, (int32)AbilityPair.Key, nullptr)); 
	}

	// 授予已学习技能（参数1表示已解锁状态）
	for (const TPair<ECAbilityInputID ,TSubclassOf<UGameplayAbility>>& AbilityPair : LearnedAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 1, (int32)AbilityPair.Key, nullptr)); 
	}
}

// 生命值更新回调函数
void UCAbilitySystemComponent::HealthUpdated(const FOnAttributeChangeData& Data)
{
	if (!GetOwner()) return;

	// 生命值≤0、服务端、死亡效果有效时，应用死亡效果
	if (Data.OldValue > 0.f && Data.NewValue <= 0.f && GetOwner()->HasAuthority() && DeathEffect)
	{
		FGameplayEffectSpecHandle InitialEffectsSpecHandle = MakeOutgoingSpec(DeathEffect, 1, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*InitialEffectsSpecHandle.Data.Get());
	}
}
