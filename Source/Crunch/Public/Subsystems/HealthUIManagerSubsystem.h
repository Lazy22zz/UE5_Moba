// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include <Widgets/OverHeadStatsGauge.h>
#include "HealthUIManagerSubsystem.generated.h"


class UWidgetComponent;
/**
 * 
 */
UCLASS()
class CRUNCH_API UHealthUIManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Character 注册和注销的接口
	void RegisterWidget(UWidgetComponent* InWidgetComp, AActor* OwnerActor);
	void UnregisterWidget(UWidgetComponent* OffWidgetComp);

	// 统一更新的函数（可以在 GameMode 或专门的 Tick 中调用，甚至用一个全局 Timer）
	void BachUpdateWidgets();

private:

	FTimerHandle BatchUpdateTimerHandle;

	// 【DOD 核心：平行的结构体数组 SoA】
	UPROPERTY()
	TArray<UWidgetComponent*> WidgetArrays;

	UPROPERTY()
	TArray<AActor*> TargetActors;

	UPROPERTY()
	TArray<UOverHeadStatsGauge*> CachedGauges; // 提前缓存好的血条 UI 实例

	// 记录上一帧的状态，用来避免重复调用耗时函数
	TArray<float> PreviousScales;
	TArray<bool> PreviousVisibilities;

	TArray<FVector> TargetLocations;	   // 缓存的位置数据
	TArray<float> DistancesSquared;        // 距离平方数组
	TArray<float> Scales;                  // 算好的缩放数组
	TArray<bool> bVisibilities;            // 算好的可见性数组

	// 配置参数
	float VisibilityRangeSquared = 100000000.f;;
	float MaxScale = 1.0f;
	float MinScale = 0.2f;

	TArray<float> LastScales;			// dirty flag
	TArray<bool> LastVisibilities;		// dirty flag

private:


	
};
