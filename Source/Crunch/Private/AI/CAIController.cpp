// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CAIController.h"
#include "Character/CCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"

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
		// 如果感知丢失，且丢失的目标是当前记录的目标，清空目标
		if (GetCurrentSeenTarget() == TargetActor)
			SetCurrentSeenTarget(nullptr);
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
