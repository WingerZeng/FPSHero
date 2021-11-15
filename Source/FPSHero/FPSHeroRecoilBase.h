#pragma once

#include "FPSHero.h"
#include "CoreMinimal.h"
#include "UObject\Object.h"
#include "FPSHeroRecoilBase.generated.h"

class APlayerController;

UCLASS(BlueprintType, Blueprintable, Abstract, EditInlineNew, Category = "Weapon")
class FPSHERO_API UFPSHeroRecoilBase : public UObject
{
	GENERATED_BODY()
public:
	UFPSHeroRecoilBase();

	// 获取随着射击过程的准心偏移量
	UFUNCTION(BlueprintImplementableEvent, Category = "Recoil")
		void GetCameraMovement(float AmmoIndex, float& pitch, float& yaw);

	// 获取随着射击过程的方向性准心扩散
	UFUNCTION(BlueprintImplementableEvent, Category = "Recoil")
		//Spread value equal to 1 means 45°spread
		void GetDirectionalSpread(float AmmoIndex, float& SpreadUp, float& SpreadRight);

	// 获取随着射击过程的随机准心扩散
	UFUNCTION(BlueprintImplementableEvent, Category = "Recoil")
		//Spread value equal to 1 means 45°spread
		void GetRandomSpread(float AmmoIndex, float& scale);

	// 获取射击结束后的镜头随时间回弹量
	UFUNCTION(BlueprintImplementableEvent, Category = "Recoil")
		//From 0 to 1 with TimeSinceStop. Totally restored when return 1.
		void GetCameraRestoreRatio(float TimeSinceStop, float& RestoreRatio);

	// 触发镜头抖动
	UFUNCTION(BlueprintImplementableEvent, Category = "Recoil")
		void ApplyCameraShake(float AmmoIndex, APlayerController* controller);

	// 后坐力恢复时间
	UPROPERTY(EditAnywhere, Category = "Recoil")
		float RecoilRestoreTime = 0.5;
};