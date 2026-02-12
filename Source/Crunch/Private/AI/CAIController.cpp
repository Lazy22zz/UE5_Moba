// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CAIController.h"
#include "Character/CCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

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

