	// Fill out your copyright notice in the Description page of Project Settings.
#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <EnhancedInputSubsystems.h>
#include <EnhancedInputComponent.h>


// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 200.l;
	CameraBoom->SetRelativeRotation(FRotator(-40.0f, 20.0f, 0.0f));

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>("Player Camera");
	PlayerCamera->SetupAttachment(CameraBoom);
	
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	
	//UE_LOG(LogTemp, Warning, TEXT("hello"));
	//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, FString::Printf(TEXT("TargetArmLength: %f"), CameraBoom->TargetArmLength));

	if (const ULocalPlayer* player = (GEngine && GetWorld()) ? GEngine->GetFirstGamePlayer(GetWorld()) : nullptr)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(player);
		if (DefaultMapping)
		{
			Subsystem->AddMappingContext(DefaultMapping, 0);
		}
	}
	if (PlayerCamera)  // 确保你的摄像机组件不为空
	{
		InitialCameraDirection = PlayerCamera->GetForwardVector();
		InitialCameraDirection.Z = 0; // 确保方向在平面上，忽略Z轴
		InitialCameraDirection.Normalize(); // 归一化方向向量
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();
	if (Controller)
	{
		// 获取当前的俯仰角
		FRotator CurrentRotation = Controller->GetControlRotation();
		float NewPitch = FMath::Clamp(CurrentRotation.Pitch + LookVector.Y, -80.0f, 80.0f); // 限制俯仰角

		// 应用旋转
		Controller->SetControlRotation(FRotator(NewPitch, CurrentRotation.Yaw + LookVector.X, 0));

		// 记录鼠标的横向移动值（左右偏移）
		MouseInputX = LookVector.X; // 存储横向输入
	}
}

void APlayerCharacter::Move(const FInputActionValue& value)
{
	PlayerMoveInput = value.Get<FVector2D>();
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 获取当前摄像机的前向向量
	FVector CurrentCameraDirection = PlayerCamera->GetForwardVector();
	CurrentCameraDirection.Normalize();

	// 前进方向基于摄像机的当前方向
	FVector ForwardMovement = CurrentCameraDirection * AutoForwardSpeed * DeltaTime;

	// 计算右向量（基于摄像机的前向向量和世界 Z 轴）
	FVector RightVector = FVector::CrossProduct(FVector(0, 0, 1), CurrentCameraDirection).GetSafeNormal();

	// 玩家输入的左右移动
	FVector RightMovement = RightVector * PlayerMoveInput.Y * AutoForwardSpeed * MoveRightSpeedMultiplier * DeltaTime;

	// 应用运动
	AddActorWorldOffset(ForwardMovement + RightMovement, true);

	// 目标旋转（基于摄像机方向）
	FRotator TargetRotation = CurrentCameraDirection.Rotation();

	// 插值平滑旋转
	FRotator SmoothRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 4.0f);
	SetActorRotation(SmoothRotation);

	// **飞船倾斜效果**
	// 根据鼠标和键盘输入调整目标倾斜角度
	float TargetTiltAngle = (PlayerMoveInput.Y + MouseInputX) * 15.0f; // 合并键盘和鼠标输入，限制倾斜范围
	CurrentTiltAngle = FMath::FInterpTo(CurrentTiltAngle, TargetTiltAngle, DeltaTime, 2.0f); // 平滑插值

	// 应用平滑后的倾斜角度
	FRotator LocalRotation = GetActorRotation();
	LocalRotation.Roll = CurrentTiltAngle; // 设置倾斜角度
	SetActorRotation(FRotator(LocalRotation.Pitch, LocalRotation.Yaw, LocalRotation.Roll));
}


// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 绑定视角控制
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		// 绑定左右移动
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
	}
}
