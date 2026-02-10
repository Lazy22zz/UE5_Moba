// Fill out your copyright notice in the Description page of Project Settings.

// 定位：GAS 体系中 “单个能力 / 技能的逻辑载体”，是具体技能的实现类（如 “火球术”“闪现”“治疗”）
// 核心职责：定义技能的完整逻辑流程 —— 触发条件（如标签判定、属性阈值）、激活逻辑（如目标筛选、GE 应用）、持续 / 中断逻辑、结束回收逻辑

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GenericTeamAgentInterface.h"
#include "CGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class UCGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
protected:
	class UAnimInstance* GetOwnerAnimInstance() const;
	TArray<FHitResult> GetHitResultFromSweepLocationTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle, float SphereSweepRadius = 30.f, ETeamAttitude::Type TeamType = ETeamAttitude::Hostile , bool bDrawDebug = false, bool bIgnoreItself = true)const;
};
