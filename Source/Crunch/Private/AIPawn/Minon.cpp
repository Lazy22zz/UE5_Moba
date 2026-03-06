// Fill out your copyright notice in the Description page of Project Settings.


#include "AIPawn/Minon.h"

void AMinon::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	Super::SetGenericTeamId(NewTeamID);
	PickSkinBasedOnTeamID();
}

void AMinon::PickSkinBasedOnTeamID()
{
	USkeletalMesh** Skin = SkinMap.Find(GetGenericTeamId());
	if (Skin)
	{
		GetMesh()->SetSkeletalMesh(*Skin);
	}
}

void AMinon::OnRep_TeamID()
{
	PickSkinBasedOnTeamID();
}
