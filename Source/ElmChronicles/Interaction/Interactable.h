#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

class APlayerCharacter;

// UHT용 껍데기. 실제 함수는 아래 IInteractable에 적는다.
// C++에서만 구현한다. (Lyra의 IInteractableTarget과 같은 방식)
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

// 플레이어가 가까이 가서 E로 상호작용할 수 있는 대상. (NPC, 나중에 상자 / 문 등)
// 대상을 찾는 쪽(AElmPlayerController)은 이 인터페이스만 알고, NPC인지 상자인지는 모른다.
class ELMCHRONICLES_API IInteractable
{
	GENERATED_BODY()

public:
	// 안내 문구에 들어갈 동작 이름. 예: "대화하기" → 화면에는 "[E] 대화하기"
	virtual FText GetInteractText() const = 0;
	// 지금 상호작용할 수 있는지. false면 안내 문구도 뜨지 않는다.
	virtual bool CanInteract(const APlayerCharacter* Player) const = 0;
	virtual void Interact(APlayerCharacter* Player) = 0;
};
