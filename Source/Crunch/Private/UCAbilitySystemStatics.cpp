// Fill out your copyright notice in the Description page of Project Settings.


#include "UCAbilitySystemStatics.h"
#include "CrunchGameplayTags.h"

FGameplayTag UCAbilitySystemStatics::GetBasicAttackAbilityTag()
{
	return CrunchGameplayTags::Ability_Basicattack;
	
	/*
	   If you want to customize in blueprint!
	*/

	// return FGameplayTag::RequestGameplayTag("Ability.BasicAttack");
}
