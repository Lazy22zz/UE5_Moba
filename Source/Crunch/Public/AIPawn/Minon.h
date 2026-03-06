// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CCharacter.h"
#include "Minon.generated.h"

/**
 * 
 */
UCLASS()
class AMinon : public ACCharacter
{
	GENERATED_BODY()
	
public:
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
private:
	void PickSkinBasedOnTeamID();

	virtual void OnRep_TeamID() override;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TMap<FGenericTeamId, USkeletalMesh*> SkinMap;
};
