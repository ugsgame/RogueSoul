// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#pragma once
#include "Asset/UIParticleEmitterAsset.h"
#include "UIParticleAsset.generated.h"

USTRUCT(BlueprintType)
struct FUIParticleEmitterInfo
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Emitter)
        bool Disable = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Emitter)
        float    ActiveDelay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Emitter)
		int32    ZOrder;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Emitter)
        UUIParticleEmitterAsset* Asset;
};


UCLASS(BlueprintType)
class UIPARTICLE_API  UUIParticleAsset : public UObject
{
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(EditAnywhere, Category = Root)
		bool AutoPlay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Emitter)
		TArray<FUIParticleEmitterInfo> Emitters;
};
