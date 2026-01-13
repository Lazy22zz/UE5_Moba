// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

// ==============================================================
// 【核心函数1】PreAttributeChange 前置属性修改拦截器
// ★★★ 核心作用（你复习重点）★★★
// 1. 拦截范围：只管控【所有非GameplayEffect(GE)驱动的属性修改】
//    - 蓝图里手动调用SetHealth/SetMana修改血蓝
//    - C++代码里直接赋值：如 SetHealth(GetHealth()-5)
//    - 角色自然回血、吃药回血、手动扣血等 无GE参与的逻辑
// 2. 拦截不到：【所有通过GE的修改都不走这里】，包括：
//    - GE的普通静态Modifier（如中毒自动扣血）
//    - GE的Execution执行器（你的碰撞触碰扣血 就是这个！完全绕开此函数）
// 3. 函数特性：NewValue是引用传递，修改后的值就是最终写入属性的值，一步到位
// 4. 解决的BUG：没有此函数 → 手动改血蓝会溢出（血到负数、蓝超上限）
// ==============================================================
void UCAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

// ==============================================================
// 【核心函数2】PostGameplayEffectExecute 后置GE执行完成回调
// ★★★ 核心作用（你复习重点，解决你碰撞扣血负数的关键）★★★
// 1. 拦截范围：管控【所有通过GameplayEffect(GE)驱动的属性修改】，无任何例外！
//    - GE的普通静态Modifier（中毒自动扣血、加攻击力buff、护盾等）
//    - GE的Execution执行计算（你的【碰撞体积触碰扣血】就是这个，100%触发此函数）
// 2. 触发原理：GAS引擎底层强制规则 → 所有GE的属性修改（不管哪种Modifier）
//    完成数值计算、并把数值写入属性后，必然触发这个回调！
// 3. 函数特性：属性值已经被修改完成，这里是【后置修正】，需要重新赋值
// 4. 解决的BUG：没有此函数 → GE的Execution碰撞扣血会无限扣到负数，静态Modifier中毒也会扣到负数
// ==============================================================
void UCAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0, GetMaxHealth()));
	}

	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0, GetMaxMana()));
	}
}


void UCAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCAttributeSet, Health, OldValue);
}

void UCAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCAttributeSet, MaxHealth, OldValue);
}

void UCAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCAttributeSet, Mana, OldValue);
}

void UCAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCAttributeSet, MaxMana, OldValue);
}

void UCAttributeSet::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UCAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
}
