// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CGameplayAbility.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// 安全的获取「当前技能拥有者（比如玩家角色 / 怪物）」的动画实例对象
UAnimInstance* UCGameplayAbility::GetOwnerAnimInstance() const
{
	USkeletalMeshComponent* OwnerSkeletalMeshComp = GetOwningComponentFromActorInfo();
	if (OwnerSkeletalMeshComp)
		return OwnerSkeletalMeshComp->GetAnimInstance();

	return nullptr;
}


// 从 TargetDataHandle 里提取出位置数据，并以这些位置为起点和终点，进行球形扫线检测，返回所有命中的结果
TArray<FHitResult> UCGameplayAbility::GetHitResultFromSweepLocationTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle, float SphereSweepRadius, ETeamAttitude::Type TeamType, bool bDrawDebug, bool bIgnoreItself) const
{
	TArray<FHitResult>OutResults; // 最终要返回的「去重后」所有命中结果
	TSet<AActor*> HitActors; // 用TSet存储已经命中过的Actor，用于去重，保证一个Actor只命中一次

	IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());// 技能持有者的teamtype

	// 遍历「技能的所有目标数据」（一个技能可能有多个瞄准点/目标点）
	for (const TSharedPtr<FGameplayAbilityTargetData> TargetData : TargetDataHandle.Data)
	{
		// 1. 从目标数据中提取：检测的起点位置（比如技能释放者的位置/准星起点）
		FVector StartLoc = TargetData->GetOrigin().GetLocation();
		// 2. 从目标数据中提取：检测的终点位置（比如瞄准的位置/技能的最远判定点）
		FVector EndLoc = TargetData->GetEndPoint();

		// 3. 设置：只检测「Pawn类型」的对象（玩家/AI角色，ECC_Pawn是碰撞通道）
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

		// 4. 设置：需要忽略的Actor列表
		TArray<AActor*> ActorsToIgnore;
		if (bIgnoreItself)
		{
			// 如果需要忽略自身 → 把技能的释放者加入忽略列表
			ActorsToIgnore.Add(GetOwningActorFromActorInfo());
		}


		// 5. 设置：是否绘制调试信息（开发调试用，上线关闭）
		EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

		// 6. 临时数组：存储本次球形检测的「所有命中结果（含重复）」
		TArray<FHitResult> HitResults;

		// 7. UE封装的核心API：执行【球形扫线检测】
		// 效果：从StartLoc到EndLoc，画一个半径为SphereSweepRadius的移动球体，检测所有Pawn类型的碰撞
		UKismetSystemLibrary::SphereTraceMultiForObjects(
			this,                // 上下文对象
			StartLoc,            // 检测起点
			EndLoc,              // 检测终点
			SphereSweepRadius,   // 球体半径
			ObjectTypes,         // 要检测的对象类型（只检测Pawn）
			false,               // 是否忽略复杂碰撞（骨骼碰撞），默认false
			ActorsToIgnore,      // 要忽略的Actor（自身）
			DrawDebugType,       // 调试绘制类型
			HitResults,          // 输出：本次检测的所有命中结果
			false                // 是否检测物理物体，默认false
		);

		// 8. 核心去重逻辑 
		for (const FHitResult& Result : HitResults)
		{
			// 如果这个Actor已经被命中过 → 跳过，去重
			if (HitActors.Contains(Result.GetActor()))
			{
				continue;
			}

			// 判断是否同teamtype
			if (OwnerTeamInterface)
			{
				ETeamAttitude::Type OtherTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*Result.GetActor());
				if (OtherTeamAttitude != TeamType)
				{
					continue;
				}
			}

			HitActors.Add(Result.GetActor());  // 记录该Actor，标记为「已命中」
			OutResults.Add(Result);            // 把去重后的有效命中结果，加入最终数组
		}
	}

	// 9. 返回所有「去重后」的命中结果（一个Actor只出现一次）
	return OutResults;
}



