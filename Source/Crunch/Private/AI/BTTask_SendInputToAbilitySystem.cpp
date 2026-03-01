// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_SendInputToAbilitySystem.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

EBTNodeResult::Type UBTTask_SendInputToAbilitySystem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 1. 获取当前执行行为树的 AI 控制器
	AAIController* OwnerAIC = OwnerComp.GetAIOwner();
	if (OwnerAIC)
	{
		// 2. 获取该 AI 控制的 Pawn（角色），并提取它身上的能力系统组件 (ASC)
		UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerAIC->GetPawn());
		if (OwnerASC)
		{
			// 3. 核心：向 ASC 发送指定的 InputID（模拟玩家按下按键），从而激活绑定的技能
			OwnerASC->PressInputID((int32)InputID);

			// 任务执行成功
			return EBTNodeResult::Succeeded;
		}
	}

	// 如果找不到 AI 控制器或角色没有 ASC，则任务失败
	return EBTNodeResult::Failed;
}