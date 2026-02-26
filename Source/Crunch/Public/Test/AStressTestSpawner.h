// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AStressTestSpawner.generated.h"

UCLASS()
class CRUNCH_API AAStressTestSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAStressTestSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// 你要在蓝图中指定的角色类（填入你的 ACCharacter 蓝图子类）
	UPROPERTY(EditAnywhere, Category = "Stress Test")
	TSubclassOf<class ACCharacter> CharacterClassToSpawn;

	// 生成数量（先从 1000 开始，再挑战 5000）
	UPROPERTY(EditAnywhere, Category = "Stress Test")
	int32 SpawnCount = 3000;

	// 生成范围的半径
	UPROPERTY(EditAnywhere, Category = "Stress Test")
	float SpawnRadius = 5000.f;

};
