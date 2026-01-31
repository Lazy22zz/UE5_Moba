// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/CCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"
#include "CrunchGameplayTags.h"
#include "Net/UnrealNetwork.h"
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

void ACCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCharacter, OwningTeamId);
}

// 游戏开始/生成时触发
void ACCharacter::BeginPlay()
{
	Super::BeginPlay();
	// 配置头顶Widget（显示/隐藏、绑定数据等）
	ConfigureOverHeadWidget();

	// 记录角色Mesh的初始相对变换（用于重生时恢复位置/旋转）
	MeshRelativeTransform = GetMesh()->GetRelativeTransform();
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

// 设置状态血条（状态栏）的启用/禁用状态
// 参数 bIsEnable: true表示启用显示，false表示禁用隐藏
void ACCharacter::SetStatusGaugeEnabled(bool bIsEnable)
{
	// 清除血条可见性更新的定时器（防止定时器重复触发导致状态异常）
	GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdateTimerHandle);

	// 如果启用状态栏
	if (bIsEnable)
	{
		// 配置头顶的UI组件（比如初始化血条、设置位置等）
		ConfigureOverHeadWidget();
	}
	// 如果禁用状态栏
	else
	{
		// 将头顶UI组件在游戏中隐藏
		OverHeadWidgetComponent->SetHiddenInGame(true);
	}
}

// 死亡动画蒙太奇播放完成后的回调函数
void ACCharacter::DeathMontageFinshed()
{
	// 启用布娃娃物理效果（角色死亡后物理接管）
	SetRagdollEnabled(true);
}

// 启用/禁用角色布娃娃(Ragdoll)物理效果
void ACCharacter::SetRagdollEnabled(bool bIsEnabled)
{
	if (bIsEnabled)
	{
		// 将Mesh从根组件分离（解除动画控制），保留世界空间变换
		GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		// 开启Mesh物理模拟（布娃娃核心：让物理引擎控制角色）
		GetMesh()->SetSimulatePhysics(true);
		// 设置碰撞为仅物理碰撞（响应物理效果，不响应其他碰撞检测）
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}
	else
	{
		// 关闭Mesh物理模拟（恢复动画控制）
		GetMesh()->SetSimulatePhysics(false);
		// 关闭Mesh碰撞（避免重生后碰撞异常）
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// 将Mesh重新附加到根组件，保留相对变换
		GetMesh()->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		// 恢复Mesh到角色初始的相对位置/旋转（重生后回到正常姿态）
		GetMesh()->SetRelativeTransform(MeshRelativeTransform);
	}
}

// 播放角色死亡动画
void ACCharacter::PlayDeathAnimation()
{
	// 检查死亡动画蒙太奇是否有效（避免空指针访问）
	if (DeathMontage)
	{
		// 播放死亡动画蒙太奇：由GE决定。
		float MontageDuration = PlayAnimMontage(DeathMontage);
		// 设置定时器：动画播放完成后（加偏移时间）触发布娃娃效果
		// DeathMontageTimeShift：动画与物理衔接的时间偏移（避免衔接生硬）
		GetWorldTimerManager().SetTimer(DeathMontageTimerHandle, this, &ACCharacter::DeathMontageFinshed, MontageDuration + DeathMontageTimeShift);

	}
}

// 角色死亡流程的核心函数
// 注：当前仅实现基础逻辑，可扩展：播放死亡音效、触发死亡特效、通知GameMode等
void ACCharacter::StartDeathSequence()
{
	// 触发死亡回调（供子类重写，实现个性化死亡逻辑）
	OnDeath();

	// 播放死亡动画
	PlayDeathAnimation();

	SetRagdollEnabled(false);

	// 禁用头顶的状态血条（死亡后不再显示）
	SetStatusGaugeEnabled(false);

	// 停止角色移动（将移动模式设为无，角色不再受物理和输入控制）
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	// 关闭胶囊体碰撞（避免死亡后角色被碰撞、阻挡）
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// 角色重生逻辑
// 注：当前仅打印日志，需补充：重置血量/属性、刷新角色位置、恢复碰撞/移动等
void ACCharacter::Respawn()
{
	// 触发重生回调（供子类重写，实现个性化重生逻辑）
	OnRespawn();

	SetRagdollEnabled(false);

	// 开启胶囊体碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 允许角色移动
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

	// 启用头顶的状态血条
	SetStatusGaugeEnabled(true);

	// 终止所有正在播放的动画蒙太奇
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);

	// 启用初始状态的GE
	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->ApplyFullStatEffect();
	}
}

// 死亡回调函数（虚函数，供子类重写）
// 基类空实现，由子类（如玩家角色）自定义死亡逻辑
void ACCharacter::OnDeath()
{
}

// 重生回调函数（虚函数，供子类重写）
// 基类空实现，由子类（如玩家角色）自定义重生逻辑
void ACCharacter::OnRespawn()
{
}

void ACCharacter::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	OwningTeamId = TeamID;
}

FGenericTeamId ACCharacter::GetGenericTeamId() const
{
	return OwningTeamId;
}
