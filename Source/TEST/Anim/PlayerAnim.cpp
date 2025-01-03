
#include "../Anim/PlayerAnim.h"
#include "PlayerCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UPlayerAnim::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	playerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	if (playerCharacter)
	{
		playerCharacterMovement = playerCharacter->GetCharacterMovement();
	}
}

void UPlayerAnim::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	if (playerCharacterMovement)
	{
		speed = UKismetMathLibrary::VSizeXY(playerCharacterMovement->Velocity);
	}
}
