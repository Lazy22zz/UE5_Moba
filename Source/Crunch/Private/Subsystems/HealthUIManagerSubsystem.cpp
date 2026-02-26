// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/HealthUIManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/OverHeadStatsGauge.h"
#include "Components/WidgetComponent.h"
#include "Stats/Stats.h"

// 声明一个名为 MyDOD 性能统计组
DECLARE_STATS_GROUP(TEXT("MyDOD_Stats"), STATGROUP_MyDOD, STATCAT_Advanced);
// 声明一个具体的计时器，用来测算你的批量更新函数
DECLARE_CYCLE_STAT(TEXT("Subsystem DOD BachUpdate"), STAT_BachUpdate, STATGROUP_MyDOD);


// 注册血条UI组件并初始化配套状态数据
void UHealthUIManagerSubsystem::RegisterWidget(UWidgetComponent* InWidgetComp, AActor* OwnerActor)
{

	if (!InWidgetComp || !OwnerActor)
	{
		return;
	}

	WidgetArrays.Add(InWidgetComp);
	TargetActors.Add(OwnerActor);
	TargetLocations.Add(OwnerActor->GetActorLocation());

	// 提前进行 Cast 并缓存下来，以后再也不用 Cast 了！
	UOverHeadStatsGauge* Gauge = Cast<UOverHeadStatsGauge>(InWidgetComp->GetUserWidgetObject());
	CachedGauges.Add(Gauge);

	// 初始化占位数据，保持数组长度一致
	DistancesSquared.Add(0.0f);
	Scales.Add(1.0f);
	bVisibilities.Add(true);

	PreviousScales.Add(-1.0f); // 初始给个负数，确保第一帧一定更新
	PreviousVisibilities.Add(false);

	LastScales.Add(-1.0f);         // 故意给个负数，确保第一帧一定不相等，强制更新UI
	LastVisibilities.Add(false);   // 假装上一帧是隐藏的，确保第一帧能正常显示

	if (WidgetArrays.Num() == 1)
	{
		GetWorld()->GetTimerManager().SetTimer(
			BatchUpdateTimerHandle,
			this,
			&UHealthUIManagerSubsystem::BachUpdateWidgets,
			0.1f, // 同样是 0.1s 更新一次
			true
		);
	}
}

void UHealthUIManagerSubsystem::UnregisterWidget(UWidgetComponent* OffWidgetComp)
{

	int32 Index = WidgetArrays.Find(OffWidgetComp);

	if (Index != INDEX_NONE)
	{
		// 移除时，为了防止数组大规模内存移动，可以用 RemoveAtSwap
		WidgetArrays.RemoveAtSwap(Index);
		TargetActors.RemoveAtSwap(Index);
		TargetLocations.RemoveAtSwap(Index);
		CachedGauges.RemoveAtSwap(Index);
		DistancesSquared.RemoveAtSwap(Index);
		Scales.RemoveAtSwap(Index);
		bVisibilities.RemoveAtSwap(Index);

		LastScales.RemoveAtSwap(Index);
		LastVisibilities.RemoveAtSwap(Index);

	}
}

void UHealthUIManagerSubsystem::BachUpdateWidgets()
{
	SCOPE_CYCLE_COUNTER(STAT_BachUpdate);

	if (WidgetArrays.Num() == 0)
	{
		return;
	}

	APawn* LocalPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (LocalPawn)
	{
		FVector LocalPawnLoc = LocalPawn->GetActorLocation();
		int32 NumWidgets = WidgetArrays.Num();

		// 步骤 0：实时更新PawnLoc
		for (int32 i = 0; i < NumWidgets; ++i)
		{
			if (TargetActors[i])
			{
				TargetLocations[i] = TargetActors[i]->GetActorLocation();
			}
		}


		// 步骤 1：纯数据计算循环（DOD 甜点区，无任何外部指针解引用，Cache 极度友好）
		for (int32 i = 0; i < NumWidgets; ++i)
		{
			DistancesSquared[i] = FVector::DistSquared(TargetLocations[i], LocalPawnLoc);
			Scales[i] = FMath::GetMappedRangeValueClamped(
				FVector2D(0.0f, VisibilityRangeSquared),
				FVector2D(MaxScale, MinScale),
				DistancesSquared[i]
			);

			bVisibilities[i] = (DistancesSquared[i] <= VisibilityRangeSquared);

		}

		// 步骤 2：UI 表现应用循环（这里操作了对象，会打破缓存，但因为计算已经提前完成，依然比以前快得多）
		for (int32 i = 0; i < NumWidgets; ++i)
		{
			UWidgetComponent* WidgetComp = WidgetArrays[i];
			if (!WidgetComp)
			{
				continue;
			}

			if (WidgetComp->GetOwner() == LocalPawn)
			{
				WidgetComp->SetHiddenInGame(true);
				continue;
			}

			// 如果可见性发生变化，才调用 SetHiddenInGame
			if (LastVisibilities[i] != bVisibilities[i])
			{
				WidgetArrays[i]->SetHiddenInGame(!bVisibilities[i]);
				LastVisibilities[i] = bVisibilities[i]; // 记录当前状态
			}

			// 如果血条是显示的，且缩放值发生了变化（比如差值大于 0.01），才更新 UI
			if (bVisibilities[i] && !FMath::IsNearlyEqual(LastScales[i], Scales[i], 0.01f))
			{
				if(UOverHeadStatsGauge* Gauge = CachedGauges[i])
				{
					Gauge->SetRenderScale(FVector2D(Scales[i]));
					LastScales[i] = Scales[i]; // 记录当前状态
				}
			}
		}
	}

	
}
