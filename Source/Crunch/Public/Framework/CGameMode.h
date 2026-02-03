// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GenericTeamAgentInterface.h"
#include "CGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ACGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& options) override;

private:
	FGenericTeamId GetTeamIDforPlayer(const APlayerController* PlayerController) const;

	AActor* FindNextStartSpotForTeam(const FGenericTeamId& TeamID) const;

	UPROPERTY(EditDefaultsOnly, Category = "Team")
	TMap<FGenericTeamId, FName> TeamStartSpotTeamMap;
};
