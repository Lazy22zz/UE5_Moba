// Fill out your copyright notice in the Description page of Project Settings.

//负责 游戏的所有业务逻辑 + 表现逻辑：角色移动、动画播放、特效音效、输入响应、死亡动作、复活表现、碰撞检测、相机跟随。
//它是游戏的 “肉身”，是玩家能看到、能操作的实体，不管 GAS 的底层逻辑。
#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h" 
#include "CCharacter.generated.h"

UCLASS()
class ACCharacter : public ACharacter, public IAbilitySystemInterface 
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACCharacter();

	// Init base on which size:serve or client.
	void ServerSideInit();
	void ClientSideInit();
	bool IsLocallyControlledbyPlayer() const;

	//Only For Client, To check Is It AICharacter.
	virtual void PossessedBy(AController* NewController) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/*********************************************************************************************/
	/*                                     Gameplay Ability                                      */
	/*********************************************************************************************/

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
	void BindGASChangedDelegate();
	void DeathTagUpdated(const FGameplayTag Tag, int32 NewCount); 

	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	class UCAbilitySystemComponent* CAbilitySystemComponent;

	UPROPERTY()
	class UCAttributeSet* CAttributeSet;

	/*********************************************************************************************/
	/*                                     UI                                                    */
	/*********************************************************************************************/

private:
	UPROPERTY(VisibleDefaultsOnly, Category = "UI")
	class UWidgetComponent* OverHeadWidgetComponent;

	void ConfigureOverHeadWidget();

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HeadStatGaugeVisiblittyCheckUpdateGap = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HeadStatGaugeVisibilityRangeSquared = 100000000.f;

	UPROPERTY(EditAnywhere, Category = "UI Scaling")
	float MinScale = 0.2f;

	UPROPERTY(EditAnywhere, Category = "UI Scaling")
	float MaxScale = 1.0f;

	FTimerHandle HeadStatGaugeVisibilityUpdateTimerHandle;

	void UpdateHeadGaugeVisibility();

	/*********************************************************************************************/
	/*                                  Death                                                    */
	/*********************************************************************************************/
private:
	void StartDeathSequence();
	void Respawn();
};
