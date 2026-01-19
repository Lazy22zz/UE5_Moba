// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/CCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"
#include "CrunchGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/OverHeadStatsGauge.h"

// 构造函数：设置默认值
ACCharacter::ACCharacter()
{
 	// 开启每帧Tick（可关闭以提升性能）
	PrimaryActorTick.bCanEverTick = true;
	// 网格体关闭碰撞
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 创建GAS核心组件：技能系统组件
	CAbilitySystemComponent = CreateDefaultSubobject<UCAbilitySystemComponent>("CAbility System Component");
	// 创建GAS属性集组件
	CAttributeSet = CreateDefaultSubobject<UCAttributeSet>("CAttribute Set");

	// 创建头顶Widget组件并挂载到根组件
	OverHeadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Over Head Widget Component");
	OverHeadWidgetComponent->SetupAttachment(GetRootComponent());

	// 绑定GAS属性/标签变更委托
	BindGASChangedDelegate();
}

// 服务端GAS初始化：初始化技能Actor信息、应用初始效果、授予技能
void ACCharacter::ServerSideInit()
{
	CAbilitySystemComponent->InitAbilityActorInfo(this, this);
	CAbilitySystemComponent->ApplyInitialEffects();
	CAbilitySystemComponent->GrantAbilities();
}

// 客户端GAS初始化：仅初始化技能Actor信息
void ACCharacter::ClientSideInit()
{
	CAbilitySystemComponent->InitAbilityActorInfo(this, this);
}

// 判断是否被本地玩家控制器控制
bool ACCharacter::IsLocallyControlledbyPlayer() const
{
	return GetController() && GetController()->IsLocalPlayerController();
}

// 被控制器接管时触发（服务端逻辑）
void ACCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 非本地玩家控制器时，执行服务端初始化
	if (NewController && !NewController->IsLocalPlayerController())
	{
		ServerSideInit();
	}
}

// 游戏开始/生成时触发
void ACCharacter::BeginPlay()
{
	Super::BeginPlay();
	// 配置头顶Widget（显示/隐藏、绑定数据等）
	ConfigureOverHeadWidget();
}

// 每帧Tick（可根据需求关闭）
void ACCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 绑定输入（此处为默认实现，需自定义补充）
void ACCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// GAS接口：获取技能系统组件（供引擎/其他系统调用）
UAbilitySystemComponent* ACCharacter::GetAbilitySystemComponent() const
{
	return CAbilitySystemComponent;
}


// 绑定GAS标签变更委托（此处绑定死亡标签）
void ACCharacter::BindGASChangedDelegate()
{
	if (CAbilitySystemComponent)
		CAbilitySystemComponent->RegisterGameplayTagEvent(CrunchGameplayTags::Status_Dead).AddUObject(this, &ACCharacter::DeathTagUpdated);
}

// 死亡标签更新回调：标签存在则触发死亡流程，否则触发重生
void ACCharacter::DeathTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount != 0)
		StartDeathSequence();
	else
		Respawn();
}

// 配置头顶Widget：本地玩家隐藏，其他玩家显示并绑定血条数据
void ACCharacter::ConfigureOverHeadWidget()
{
	if (!OverHeadWidgetComponent) 
	{
		return;
	}

	// 本地玩家控制时隐藏头顶Widget
	if (IsLocallyControlledbyPlayer())
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
		return;
	}

	// 绑定血条Widget与GAS组件，开启距离检测定时器
	UOverHeadStatsGauge* OverHeadStatsGauge = Cast<UOverHeadStatsGauge>(OverHeadWidgetComponent->GetUserWidgetObject());
	if (OverHeadStatsGauge)
	{
		OverHeadStatsGauge->ConfigureWithASC(GetAbilitySystemComponent());
		OverHeadWidgetComponent->SetHiddenInGame(false);
		GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdateTimerHandle);
		GetWorldTimerManager().SetTimer(HeadStatGaugeVisibilityUpdateTimerHandle, this, &ACCharacter::UpdateHeadGaugeVisibility, HeadStatGaugeVisiblittyCheckUpdateGap, true);
	}
}

// 更新头顶血条的缩放和可见性（根据与本地玩家的距离）
void ACCharacter::UpdateHeadGaugeVisibility()
{
	APawn* LocalPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (LocalPlayerPawn)
	{
		// 计算与本地玩家的距离平方（减少开方运算，提升性能）
		float DistSquared = FVector::DistSquared(GetActorLocation(), LocalPlayerPawn->GetActorLocation());

		// 根据距离映射缩放值（距离越远，缩放越小）
		float NewScale = FMath::GetMappedRangeValueClamped(
			FVector2D(0.0f, HeadStatGaugeVisibilityRangeSquared),
			FVector2D(MaxScale, MinScale), 
			DistSquared
		);

		// 设置血条Widget缩放
		FVector2D ScaleVector2D(NewScale);
		Cast<UOverHeadStatsGauge>(OverHeadWidgetComponent->GetUserWidgetObject())->SetRenderScale(ScaleVector2D);

		// 距离超过阈值则隐藏Widget
		OverHeadWidgetComponent->SetHiddenInGame(DistSquared > HeadStatGaugeVisibilityRangeSquared);
	}
}

// 死亡流程（仅日志，需补充具体逻辑：播放动画、禁用控制等）
void ACCharacter::StartDeathSequence()
{
	UE_LOG(LogTemp, Warning, TEXT("DEAD"));
}

// 重生逻辑（仅日志，需补充具体逻辑：重置属性、刷新位置等）
void ACCharacter::Respawn()
{
	UE_LOG(LogTemp, Warning, TEXT("Respawn"));
}