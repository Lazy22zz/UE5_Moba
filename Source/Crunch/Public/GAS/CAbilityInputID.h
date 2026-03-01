// Fill out your copyright notice in the Description page of Project Settings.

/*
简单来说，设计 InputID 就是为了在**玩家的手（物理按键）和角色的动作（技能系统）**之间加一个“中间缓冲层”。

其核心作用可以简洁地总结为以下四点：

迎合底层框架：GAS 的原生函数强制要求使用枚举（Enum）来绑定输入，以此实现“按下自动施法，松手自动停止”。

彻底解耦按键：物理按键不再和技能死死绑定。这样不仅极大地简化了玩家的“自定义改键”，也方便在角色眩晕或过场动画时直接切断输入。

支撑连招派生：在连招的判定窗口期内，系统通过读取收到的 InputID（比如是普攻还是技能1），来决定下一个动作派生出什么招式。

处理施法确认：枚举中的 Confirm 和 Cancel 专门对接 GAS 的指示器（TargetActor）底层，用于处理需要“划定范围 -> 左键确认/右键取消”的复杂技能。
*/

#pragma once

#include "CoreMinimal.h"
#include "CAbilityInputID.generated.h" 

UENUM(BlueprintType)
enum class ECAbilityInputID : uint8
{
	None				UMETA(DisplayName = "None"),
	BasicAttack			UMETA(DisplayName = "Basic Attack"),
	AbilityOne			UMETA(DisplayName = "Ability One"),
	AbilityTwo			UMETA(DisplayName = "Ability Two"),
	AbilityThree		UMETA(DisplayName = "Ability Three"),
	AbilityFour			UMETA(DisplayName = "Ability Four"),
	AbilityFive			UMETA(DisplayName = "Ability Five"),
	AbilitySix			UMETA(DisplayName = "Ability Six"),
	Confirm				UMETA(DisplayName = "Confirm"),
	Cancel				UMETA(DisplayName = "Cancel")
};
