// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/MinionBarrack.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

AMinionBarrack::AMinionBarrack()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMinionBarrack::BeginPlay()
{
	Super::BeginPlay();

	// 游戏开始时，立即生成 5 个 Minion
	SpawnMinons(5);
}

void AMinionBarrack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 批量生成 Minion，Num 为本次生成数量
void AMinionBarrack::SpawnMinons(int Num)
{
	// 未配置 Minion 蓝图类时，提前报错退出
	if (!MinonClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("MinionBarrack: MinonClass is not set!"));
		return;
	}

	for (int i = 0; i < Num; ++i)
	{
		// 默认使用兵营自身位置；若有指定生成点则优先使用
		FTransform SpawnActorTransform = GetActorTransform();
		if (const APlayerStart* SpawnSpot = GetNextSpawnSpot())
		{
			SpawnActorTransform = SpawnSpot->GetActorTransform();
		}

		// Deferred 生成：先创建对象，暂不执行 BeginPlay
		// 这样可以在 BeginPlay 触发前完成初始化赋值
		AMinon* NewMinon = GetWorld()->SpawnActorDeferred<AMinon>(
			MinonClass, SpawnActorTransform, this, nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
		);

		if (NewMinon)
		{
			// 在 BeginPlay 之前设置队伍 ID，确保 AI 感知系统初始化时能读到正确阵营
			NewMinon->SetGenericTeamId(TeamBarrackId);

			// 完成生成，触发 Minion 的 BeginPlay
			UGameplayStatics::FinishSpawningActor(NewMinon, SpawnActorTransform);

			// 记录到池子，方便后续统一管理（清场、计数等）
			MinionPool.Add(NewMinon);
		}
	}
}

// 轮询返回下一个生成点（循环使用 SpawnSpots 数组）
const APlayerStart* AMinionBarrack::GetNextSpawnSpot()
{
	if (SpawnSpots.Num() == 0) return nullptr;

	// 索引超出数组范围时，回绕到第一个
	++NextSpawnSpotIndex;
	if (NextSpawnSpotIndex >= SpawnSpots.Num())
	{
		NextSpawnSpotIndex = 0;
	}

	return SpawnSpots[NextSpawnSpotIndex];
}