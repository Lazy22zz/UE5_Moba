// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIPawn/Minon.h"
#include "GameFramework/Actor.h"
#include "GenericTeamAgentInterface.h"
#include "MinionBarrack.generated.h"

UCLASS()
class CRUNCH_API AMinionBarrack : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMinionBarrack();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, Category = "Spawn")
	FGenericTeamId TeamBarrackId;

	UPROPERTY()
	TArray<class AMinon*> MinionPool;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<class AMinon> MinonClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<class APlayerStart*> SpawnSpots;

	void SpawnMinons(int Num);

	int NextSpawnSpotIndex = -1;

	const APlayerStart* GetNextSpawnSpot();
};
