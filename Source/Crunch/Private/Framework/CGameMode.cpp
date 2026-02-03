// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CGameMode.h"
#include "EngineUtils.h"          // 引擎工具类，用于遍历场景中的Actor
#include "GameFramework/PlayerStart.h" // 玩家出生点相关头文件

// 重写SpawnPlayerController方法：生成玩家控制器时，为玩家分配队伍和出生点
APlayerController* ACGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& options)
{
    // 先调用父类方法，生成默认的玩家控制器
    APlayerController* NewPlayercontroller = Super::SpawnPlayerController(InRemoteRole, options);

    // 将玩家控制器转换为"通用队伍代理接口"（用于处理队伍相关逻辑）
    IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayercontroller);

    // 获取该玩家对应的队伍ID
    FGenericTeamId TeamId = GetTeamIDforPlayer(NewPlayercontroller);

    // 如果转换接口成功（玩家控制器支持队伍功能）
    if (NewPlayerTeamInterface)
    {
        // 给玩家设置分配好的队伍ID
        NewPlayerTeamInterface->SetGenericTeamId(TeamId);
    }

    // 根据队伍ID为玩家找对应的出生点，并赋值给玩家控制器的StartSpot
    NewPlayercontroller->StartSpot = FindNextStartSpotForTeam(TeamId);

    // 返回设置好的玩家控制器
    return NewPlayercontroller;
}

// 核心逻辑：为玩家分配队伍ID（简单的奇偶分配，0/1两队）
FGenericTeamId ACGameMode::GetTeamIDforPlayer(const APlayerController* PlayerController) const
{
    // static变量：仅初始化一次，记录已分配的玩家总数（重启游戏才会重置）
    static int playercount = 0;
    // 每调用一次，玩家数+1
    ++playercount;
    // 取模2：实现玩家轮流分配到0队（偶数）和1队（奇数）
    return FGenericTeamId(playercount % 2);
}

// 根据队伍ID查找该队伍专属的出生点
AActor* ACGameMode::FindNextStartSpotForTeam(const FGenericTeamId& TeamID) const
{
    // 从队伍-出生点标签映射表中，找到该队伍对应的出生点标签
    const FName* StartSpotTag = TeamStartSpotTeamMap.Find(TeamID);
    // 如果没找到对应标签，返回空（玩家会用默认出生点）
    if (!StartSpotTag)
    {
        return nullptr;
    }

    // 获取当前游戏世界的指针
    UWorld* World = GetWorld();

    // 遍历场景中所有的APlayerStart（玩家出生点）Actor
    for (TActorIterator<APlayerStart> It(World); It; ++It)
    {
        // 如果当前出生点的标签和队伍对应的标签匹配
        if (It->PlayerStartTag == *StartSpotTag)
        {
            // 将该出生点标记为"已占用"（避免重复分配）
            It->PlayerStartTag = FName("Taken");
            // 返回这个匹配的出生点
            return *It;
        }
    }

    // 遍历完所有出生点都没找到匹配的，返回空
    return nullptr;
}