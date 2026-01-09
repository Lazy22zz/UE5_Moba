// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_Combo.h"
#include "UCAbilitySystemStatics.h"
#include "CrunchGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
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

// 伤害结算核心逻辑
void UGA_Combo::DoDamage(FGameplayEventData GameplayData)
{
	TArray<FHitResult> HitResults = GetHitResultFromSweepLocationTargetData(GameplayData.TargetData, 30.f, true, true);
}
