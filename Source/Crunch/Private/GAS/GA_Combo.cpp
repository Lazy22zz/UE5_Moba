// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_Combo.h"
#include "UCAbilitySystemStatics.h"
#include "CrunchGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagsManager.h"

// 给普攻技能加「身份标识」和「互斥约束」
UGA_Combo::UGA_Combo()
{
	AbilityTags.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag()); // or AbilityTags.AddTag(CrunchGameplayTags::Ability_Basicattack);
	BlockAbilitiesWithTag.AddTag(CrunchGameplayTags::Ability_Basicattack);
}

//GAS 的核心重写函数，所有技能的逻辑都是从这个函数开始执行
void UGA_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 技能合法性校验：提交技能 + 失败兜底
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	// 「客户端 + 服务端」双端同步逻辑（网络安全）
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo)) //server predict actor actions
	{
		UAbilityTask_PlayMontageAndWait* PlayComboMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ComboMontage);

		// 任务1：播放连招蒙太奇动画，监听所有动画异常状态
		PlayComboMontageTask->OnBlendOut.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnCancelled.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnCompleted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnInterrupted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->ReadyForActivation();
		
		// 任务2：监听连招切换事件
		UAbilityTask_WaitGameplayEvent* WaitComboChangeEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboChangedEventTag(), nullptr, false, false );
		WaitComboChangeEventTask->EventReceived.AddDynamic(this, &UGA_Combo::ComboChangeEventReceived);
		WaitComboChangeEventTask->ReadyForActivation();
	}

	//「纯服务端」伤害结算逻辑（防作弊核心）
	if (K2_HasAuthority())
	{
		UAbilityTask_WaitGameplayEvent* WaitTargetingEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboEventTargetTag());
		WaitTargetingEventTask->EventReceived.AddDynamic(this, &UGA_Combo::DoDamage);
		WaitTargetingEventTask->ReadyForActivation();
	}

	//初始化连招按键监听
	SetupInputComboPress();
}

// 标签封装函数
FGameplayTag UGA_Combo::GetComboChangedEventTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Combo.Change");
}

FGameplayTag UGA_Combo::GetComboChangedEndTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Combo.Change.end");
}

FGameplayTag UGA_Combo::GetComboEventTargetTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Combo.Damage");
}

// 连招按键监听
void UGA_Combo::SetupInputComboPress()
{
	UAbilityTask_WaitInputPress* WaitInputPress = UAbilityTask_WaitInputPress::WaitInputPress(this);
	WaitInputPress->OnPress.AddDynamic(this, &UGA_Combo::HandleInputPress); //GAS asynchronous task
	WaitInputPress->ReadyForActivation();
}

// 连招按键响应
void UGA_Combo::HandleInputPress(float TimeWaited)
{
	SetupInputComboPress();
	TryCommitCombo();
}

// 连招切换核心逻辑
void UGA_Combo::TryCommitCombo()
{
	if (NextComboName == NAME_None)
		return;

	UAnimInstance* OwnerAnimInstnce = GetOwnerAnimInstance();
	if (!OwnerAnimInstnce)
		return;

	// The core API for switching combo animations
	OwnerAnimInstnce->Montage_SetNextSection(OwnerAnimInstnce->Montage_GetCurrentSection(ComboMontage), NextComboName, ComboMontage);
}

TSubclassOf<UGameplayEffect> UGA_Combo::GetDamageEffectforcurrentCombo() const
{
	// 1. 获取技能拥有者的动画实例（比如玩家角色/AI的UAnimInstance）
	UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();
	if (OwnerAnimInstance) // 动画实例有效，才继续匹配
	{
		// 2. 获取【连击蒙太奇ComboMontage】当前正在播放的动画分段名称（FName）
		// 比如播放到Combo_1，就返回FName("Combo_1")，播放到重击就返回FName("Heavy")
		FName CurrentSectionName = OwnerAnimInstance->Montage_GetCurrentSection(ComboMontage);

		// 3. TMap的核心API：Find(Key) → 根据Key查找对应的Value，返回【常量指针】
		// 为什么返回指针？因为Find可能查不到，查不到时返回 nullptr，查到则返回指向对应Value的指针
		const TSubclassOf<UGameplayEffect>* FoundEffectPtr = DamageEffectMap.Find(CurrentSectionName);

		if (FoundEffectPtr) // 查到了！指针非空
		{
			// 4. 解引用指针，返回查到的「对应连击段的伤害效果类」
			return *FoundEffectPtr;
		}
	}
	// 5. 兜底逻辑：任何失败情况（动画实例无效/查不到对应伤害效果），返回默认伤害效果
	return DefaultDamageEffectMap;
}

// Use "GameplayTag" to pass "the segment name of the next combo"
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

void UGA_Combo::DoDamage(FGameplayEventData GameplayData)
{
	// 核心API：从事件数据的「目标数据」中，获取所有命中的目标（FHitResult数组）
	// 参数说明：
	// - GameplayData.TargetData：GAS的目标数据，包含本次连击的碰撞检测结果
	// - true,true：是否忽略自己/是否忽略友好单位，GAS标准写法，避免打自己/队友
	TArray<FHitResult> HitResults = GetHitResultFromSweepLocationTargetData(GameplayData.TargetData, HitTargetSphereRadius, false, true);

	// 遍历所有命中的目标，逐个施加伤害效果（多段连击可以同时打多个敌人）
	for (const FHitResult& HitResult : HitResults)
	{
		// 调用上面的查询函数，拿到「当前连击段对应的伤害效果类」
		TSubclassOf<UGameplayEffect> GameplayEffect = GetDamageEffectforcurrentCombo();

		// FGameplayEffectSpecHandle： 效果「本体」-> 伤害数值、效果规则、等级、标签
		// MakeOutgoingGameplayEffectSpec：把「伤害效果类」实例化为「可执行的效果蓝图Spec」
		// 第二个参数：获取当前技能的等级，伤害数值会根据技能等级缩放（比如2级普攻伤害更高）
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(GameplayEffect, GetAbilityLevel(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo()));

		// FGameplayEffectContextHandle: 效果「快递盒」-> 命中位置、HitResult、施法者、受击者、自定义信息和决定效果的附加信息是什么、蓝图能拿到什么数据
		FGameplayEffectContextHandle GameplayEffectContextHandle = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
		GameplayEffectContextHandle.AddHitResult(HitResult);

		// 让效果带着附加信息走
		EffectSpecHandle.Data->SetContext(GameplayEffectContextHandle);

		//  最终核心：把伤害效果蓝图，施加给命中的目标 → 完成伤害结算！
		// ApplyGameplayEffectSpecToTarget：GAS的伤害施加API，把Spec句柄应用到目标身上
		// UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor：把命中的Actor转为GAS能识别的目标数据
		ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor()));
	}
}
