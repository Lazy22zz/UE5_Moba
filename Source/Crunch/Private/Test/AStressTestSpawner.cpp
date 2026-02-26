// Fill out your copyright notice in the Description page of Project Settings.


#include "Test/AStressTestSpawner.h"
#include "Character/CCharacter.h"

// Sets default values
AAStressTestSpawner::AAStressTestSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AAStressTestSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (!CharacterClassToSpawn || !GetWorld()) return;

	for (int32 i = 0; i < SpawnCount; i++)
	{
		// 在给定的半径内随机生成一个平面位置
		FVector RandomOffset = FMath::VRand();
		RandomOffset.Z = 0.f;
		FVector SpawnLocation = GetActorLocation() + RandomOffset * FMath::FRandRange(0.f, SpawnRadius);
		FRotator SpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; // 无视碰撞强制生成

		GetWorld()->SpawnActor<ACCharacter>(CharacterClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
	}
}



