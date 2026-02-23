// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CAIController.generated.h"

/**
 * 
 */
UCLASS()
class CRUNCH_API ACAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ACAIController();

	virtual void OnPossess(APawn* NewPawn) override;

	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "AI Behavior")
	FName TargetBlackBoardName = "Target";

	UPROPERTY(EditDefaultsOnly, Category = "AI Behavior")
	class UBehaviorTree* BehaviorTree;

	UPROPERTY(VisibleAnywhere, Category = "Perception")
	class UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Perception")
	class UAISenseConfig_Sight* AISenseconfig_sight;

	UFUNCTION()
	void TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus);

	UFUNCTION()
	void TargetForgotten(AActor* ForgottonActor);

	const UObject* GetCurrentSeenTarget() const;

	void SetCurrentSeenTarget(AActor* NewTarget);

	AActor* GetFirstSensedPerceptionTarget() const;

	void ForgetIfTargetIsDead(AActor* TargetToForgot);
};
