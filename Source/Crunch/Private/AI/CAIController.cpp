// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CAIController.h"
#include "Character/CCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CrunchGameplayTags.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"

ACAIController::ACAIController()
{
	// --- 1. 初始化感知组件 ---
	// 创建感知组件（相当于 AI 的“大脑”负责处理感官信息）
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("AI Perception Component");
	// 创建视觉配置（相当于给 AI 装上“眼睛”）
	AISenseconfig_sight = CreateDefaultSubobject<UAISenseConfig_Sight>("AI sight config");

	// --- 2. 配置敌我识别 ---
	// 设置只检测“敌人”，忽略友军和中立单位
	AISenseconfig_sight->DetectionByAffiliation.bDetectEnemies = true;
	AISenseconfig_sight->DetectionByAffiliation.bDetectFriendlies = false;
	AISenseconfig_sight->DetectionByAffiliation.bDetectNeutrals = false;

	// --- 3. 配置视野距离 ---
	AISenseconfig_sight->SightRadius = 1000.f;      // 视野范围：1000单位内能看见
	AISenseconfig_sight->LoseSightRadius = 1200.f;  // 丢失范围：目标跑出1200单位才算彻底看不见（防止在边缘反复横跳）

	// --- 4. 其他视觉参数 ---
	AISenseconfig_sight->SetMaxAge(5.f);                       // 记忆时间：目标消失后，AI 还能“记得”它 5 秒
	AISenseconfig_sight->PeripheralVisionAngleDegrees = 180.f; // 视野角度：180度（前方半圆区域）

	// --- 5. 应用配置 ---
	// 将配置好的视觉系统注册到感知组件中
	AIPerceptionComponent->ConfigureSense(*AISenseconfig_sight);

	// --- 6. 绑定感知回调 ---
	// 注册感知更新事件：当AI看到/丢失目标时，自动调用TargetPerceptionUpdated函数处理逻辑
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACAIController::TargetPerceptionUpdated);

	// 注册“遗忘目标”事件：当AI的感知记忆超时（5秒）彻底遗忘目标时，自动调用TargetForgotten函数切换追踪目标
	AIPerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &ACAIController::TargetForgotten);
}

void ACAIController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

	// --- 设置队伍 ID ---
	// 1. 将 AI 控制器本身的队伍设为 ID 0
	SetGenericTeamId(FGenericTeamId(0));

	// 2. 尝试将这个队伍 ID 同步给被控制的角色 (Pawn)
	// 这样确保 AI 和它控制的身体属于同一个阵营
	IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(NewPawn);
	if (TeamAgentInterface)
	{
		TeamAgentInterface->SetGenericTeamId(GetGenericTeamId());
	}

	// 3. 使用delegate绑定gameplaytag和对应的行为
	UAbilitySystemComponent* AIPawnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(NewPawn);
	if (AIPawnASC)
	{
		AIPawnASC->RegisterGameplayTagEvent(CrunchGameplayTags::Status_Dead).AddUObject(this, &ACAIController::AIPawnDeadTagUpdate);
	}

}

// AI控制器开始运行时的初始化函数
void ACAIController::BeginPlay()
{
	// 调用父类的BeginPlay（UE必备，保证父类逻辑执行）
	Super::BeginPlay();
	// 启动行为树，AI开始执行预设的行为逻辑
	RunBehaviorTree(BehaviorTree);
}

// AI感知到目标更新时的回调函数（感知系统触发）
// TargetActor：被感知的目标Actor  Stimulus：感知刺激信息（是否看到/听到等）
void ACAIController::TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
	// 如果成功感知到目标（比如看到了敌人）
	if (Stimulus.WasSuccessfullySensed())
	{
		// 如果当前黑板中没有记录目标，就设置新目标
		if (!GetCurrentSeenTarget())
		{
			SetCurrentSeenTarget(TargetActor);
		}
	}
	else
	{
		// 如果感知丢失，且死亡的目标是当前记录的目标，清空目标
		ForgetIfTargetIsDead(TargetActor);
		
	}
}

/**
 * @brief 当AI忘记某个感知目标时执行的处理函数
 * @param ForgottonActor 被AI遗忘的目标Actor
 */
void ACAIController::TargetForgotten(AActor* ForgottonActor)
{
	// 安全检查：如果感知组件为空，直接返回，避免空指针访问
	if (!AIPerceptionComponent)
		return;

	// 检查当前AI正在追踪的可见目标是否就是被遗忘的这个Actor
	if (GetCurrentSeenTarget() == ForgottonActor)
	{
		// 如果是，则将当前追踪目标切换为"被遗忘的感知目标"（通常是备用/下一个目标）
		SetCurrentSeenTarget(GetFirstSensedPerceptionTarget());
	}
}

// 读取黑板中当前感知到的目标（只读，不修改数据）
const UObject* ACAIController::GetCurrentSeenTarget() const
{
	// 获取黑板组件（只读，因为仅读取数据）
	const UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
	// 黑板组件不为空时，读取指定名称的对象值并返回
	if (BlackboardComp)
	{
		return BlackboardComp->GetValueAsObject(TargetBlackBoardName);
	}

	// 黑板组件为空时返回空指针（安全处理）
	return nullptr;
}

// 设置/清空黑板中的感知目标（修改黑板数据）
// NewTarget：要设置的新目标（传nullptr则清空）
void ACAIController::SetCurrentSeenTarget(AActor* NewTarget)
{
	// 获取黑板组件（非只读，因为要修改数据）
	UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
	// 黑板组件为空则直接返回（避免空指针崩溃）
	if (!BlackboardComp)
		return;

	// 如果传空，清空黑板中对应的目标值
	if (!NewTarget)
	{
		BlackboardComp->ClearValue(TargetBlackBoardName);
	}
	else
	{
		// 否则将新目标设置到黑板中
		BlackboardComp->SetValueAsObject(TargetBlackBoardName, NewTarget);
	}
}

/**
 * @brief 获取AI感知组件中第一个敌对感知目标（用于遗忘当前目标后切换）
 * @return 返回第一个感知到的敌对Actor；若无感知组件/无敌对目标，返回nullptr
 */
AActor* ACAIController::GetFirstSensedPerceptionTarget() const
{
	// 安全检查：感知组件存在时才执行后续逻辑
	if (AIPerceptionComponent)
	{
		// 定义数组存储感知到的所有敌对Actor
		TArray<AActor*> TargetLists;
		// 从感知组件中获取所有被标记为“敌对”的感知目标，存入数组
		AIPerceptionComponent->GetPerceivedHostileActors(TargetLists);

		// 检查数组中是否有有效的敌对目标
		if (TargetLists.Num() != 0)
		{
			// 返回数组中第一个敌对目标（作为遗忘当前目标后的备选目标）
			return TargetLists[0];
		}
	}

	// 无感知组件/无敌对目标时，返回空指针
	return nullptr;
}

// AI遗忘已经感知到的死亡actor
void ACAIController::ForgetIfTargetIsDead(AActor* TargetToForgot)
{
	// 1. 获取目标的技能系统组件(ASC)，这是判断生死状态的前提
	const UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetToForgot);
	if (!TargetASC)
	{
		return; // 如果目标连ASC都没有（比如是个没有生命值的石头），直接跳过
	}

	// 2. 检查目标身上是否有“死亡”的GameplayTag
	if (TargetASC->HasMatchingGameplayTag(CrunchGameplayTags::Status_Dead))
	{
		/* 
		
		老师推荐的通过age 最大化让系统忘记
		//3. 打开AI脑海里的“感知记事本”（类似C++的 TMap<AActor*, 综合感知档案>）
		//Iter->Key 是被感知的具体Actor
		// Iter->Value 是该Actor留下的所有感知情报
		for (UAIPerceptionComponent::TActorPerceptionContainer::TIterator Iter = AIPerceptionComponent->GetPerceptualDataIterator(); Iter; ++Iter)
		{
			// 如果这一页记录的不是我们要找的死者，翻到下一页
			if (Iter->Key != TargetToForgot)
			{
				continue;
			}

			// 4. 找到了死者的档案！遍历他留下的所有“感官刺激原件”（视觉、听觉等）
			// 注意：必须用 &（引用）来直接修改底层真实数据，否则改的只是复印件
			for (FAIStimulus& Stimulus : Iter->Value.LastSensedStimuli)
			{
				// 5. 核心黑科技：把这些感官记忆的“存在时间”强行改成浮点数最大值
				// 下一帧引擎检测时，会发现这些记忆严重“超时/过期 (Expired)”
				// 从而触发底层的 MarkExpired 流程，让AI彻底忘掉他
				Stimulus.SetStimulusAge(TNumericLimits<float>::Max());
			}
		}
		*/

		// 这里利用了GetFirstSensedPerceptionTarget()的AIPerceptionComponent->GetPerceivedHostileActors(TargetLists)
		// 里的GetHostileActors(), 因为ForgetActor()有自清理功能。

		if (AIPerceptionComponent)
		{
			// 官方 API：直接让感知组件遗忘这个 Actor
			AIPerceptionComponent->ForgetActor(TargetToForgot);
			
			// 2. 【核心修复】手动检查并清理黑板！
			// 如果当前黑板里锁定的目标，正好是这个刚死的 Actor，就立刻清空或切换
			if (GetCurrentSeenTarget() == TargetToForgot)
			{
			// 尝试寻找下一个活着的敌人，如果没有就会自动传入 nullptr 清空黑板
				SetCurrentSeenTarget(GetFirstSensedPerceptionTarget());
			}
		}
	}
}

/**
 * @brief 清空AI感知信息并禁用所有感知能力
 * @details 该函数会将所有已感知的刺激信息标记为过期，并禁用AI的所有感知类型（如视觉、听觉等）
 */
void ACAIController::ClearAndDisableSenses()
{
	// 检查AI感知组件是否有效
	if (AIPerceptionComponent)
	{
		// 将所有已感知的刺激信息设置为最大过期时间，使其立即失效
		AIPerceptionComponent->AgeStimuli(TNumericLimits<float>::Max());

		// 遍历AI感知组件中所有已配置的感知类型
		for (auto SenseConfigIt = AIPerceptionComponent->GetSensesConfigIterator(); SenseConfigIt; ++SenseConfigIt)
		{
			// 禁用当前遍历到的感知类型（如视觉、听觉、嗅觉等）
			AIPerceptionComponent->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(), false);
		}
	}

	// 获取黑板组件并检查有效性
	if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
	{
		// 清空黑板上的目标变量（通常用于存储AI追踪的目标）
		BlackboardComp->ClearValue(TargetBlackBoardName);
	}
}

/**
 * @brief 启用AI所有已配置的感知能力
 * @details 重新激活AI的所有感知类型，恢复其感知环境的能力
 */
void ACAIController::EnableAllSenses()
{
	// 遍历AI感知组件中所有已配置的感知类型
	for (auto SenseConfigIt = AIPerceptionComponent->GetSensesConfigIterator(); SenseConfigIt; ++SenseConfigIt)
	{
		// 启用当前遍历到的感知类型
		AIPerceptionComponent->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(), true);
	}
}

/**
 * @brief 根据AI死亡标签的计数更新AI行为状态
 * @param gameplayTag 触发的游戏标签（此处为AI死亡标签）
 * @param count 标签计数（非0表示死亡，0表示复活/未死亡）
 */
void ACAIController::AIPawnDeadTagUpdate(FGameplayTag gameplayTag, int32 count)
{
	// 如果计数非0，说明AI已死亡
	if (count != 0)
	{
		// 停止AI的行为逻辑，并添加调试说明
		GetBrainComponent()->StopLogic("AI Pawn is dead!");
		// 清空感知信息并禁用所有感知能力
		ClearAndDisableSenses();
	}
	else
	{
		// 计数为0，说明AI复活/未死亡，重新启动AI行为逻辑
		GetBrainComponent()->StartLogic();
		// 重新启用所有感知能力
		EnableAllSenses();
	}
}


