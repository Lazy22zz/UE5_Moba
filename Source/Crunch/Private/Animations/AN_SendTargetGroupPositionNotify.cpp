// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/AN_SendTargetGroupPositionNotify.h"
#include "AbilitySystemBlueprintLibrary.h"

// 动画通知的核心重写功能
void UAN_SendTargetGroupPositionNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
		return;

	if (TargetSocketNames.Num() <= 1) //在逻辑执行之前，必须至少配置两个SocketNames。
		return;

	if (!MeshComp->GetOwner() || !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner()))
		return;

	FGameplayEventData EventData; //Standard Event Data Structure for the GAS System

	for (int i = 1; i < TargetSocketNames.Num(); ++i)
	{
		// 1. 创建一个「位置信息」对象，存储单组「开始+结束」位置
		FGameplayAbilityTargetData_LocationInfo* LocationInfo = new FGameplayAbilityTargetData_LocationInfo();

		// 2. 获取【上一个Socket】的世界坐标 = 本次扫描的「开始位置」
		FVector StartLocation = MeshComp->GetSocketLocation(TargetSocketNames[i - 1]);
		// 3. 获取【当前Socket】的世界坐标 = 本次扫描的「结束位置」
		FVector EndLocation = MeshComp->GetSocketLocation(TargetSocketNames[i]);

		// 4. 把起止坐标赋值给位置信息对象
		LocationInfo->SourceLocation.LiteralTransform.SetLocation(StartLocation);
		LocationInfo->TargetLocation.LiteralTransform.SetLocation(EndLocation);

		// 5. 将这组「起止位置信息」添加到 事件数据的「位置数组」中
		EventData.TargetData.Add(LocationInfo);
	}

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag, EventData);

}
