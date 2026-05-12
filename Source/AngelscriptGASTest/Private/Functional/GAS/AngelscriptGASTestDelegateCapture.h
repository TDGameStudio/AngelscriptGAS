#pragma once

#include "AngelscriptAbilitySystemComponent.h"
#include "AngelscriptAttributeSet.h"
#include "CoreMinimal.h"

#include "AngelscriptGASTestDelegateCapture.generated.h"

UCLASS()
class UAngelscriptGASTestDelegateCapture : public UObject
{
	GENERATED_BODY()

public:
	bool bFired = false;
	int32 FireCount = 0;

	AActor* CapturedOwner = nullptr;
	AActor* CapturedAvatar = nullptr;

	int32 CapturedAbilityLevel = 0;
	int32 CapturedAbilityInputID = 0;

	FGameplayTag FilterTag;
	bool bUseTagFilter = false;
	bool bCapturedTagExists = false;

	FAngelscriptModifiedAttribute CapturedAttributeChange;
	TArray<FName> AttributeChangedNames;

	UAngelscriptAttributeSet* CapturedAttributeSet = nullptr;

	UFUNCTION()
	void HandleInitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
	{
		bFired = true;
		FireCount++;
		CapturedOwner = InOwnerActor;
		CapturedAvatar = InAvatarActor;
	}

	UFUNCTION()
	void HandleAbilityGiven(const FGameplayAbilitySpec& AbilitySpec)
	{
		bFired = true;
		FireCount++;
		CapturedAbilityLevel = AbilitySpec.Level;
		CapturedAbilityInputID = AbilitySpec.InputID;
	}

	UFUNCTION()
	void HandleAbilityRemoved(const FGameplayAbilitySpec& AbilitySpec)
	{
		bFired = true;
		FireCount++;
		CapturedAbilityLevel = AbilitySpec.Level;
		CapturedAbilityInputID = AbilitySpec.InputID;
	}

	UFUNCTION()
	void HandleOwnedTagUpdated(const FGameplayTag& Tag, bool bTagExists)
	{
		if (bUseTagFilter && Tag != FilterTag)
		{
			return;
		}

		bFired = true;
		FireCount++;
		bCapturedTagExists = bTagExists;
	}

	UFUNCTION()
	void HandleAttributeChanged(const FAngelscriptModifiedAttribute& AttributeChangeData)
	{
		bFired = true;
		FireCount++;
		CapturedAttributeChange = AttributeChangeData;
		AttributeChangedNames.Add(AttributeChangeData.Name);
	}

	UFUNCTION()
	void HandleGameplayAttributeChanged(const FAngelscriptAttributeChangedData& AttributeChangeData)
	{
		const FName AttributeName(*AttributeChangeData.WrappedData.Attribute.AttributeName);
		bFired = true;
		FireCount++;
		CapturedAttributeChange = FAngelscriptModifiedAttribute(
			AttributeName,
			AttributeChangeData.WrappedData.OldValue,
			AttributeChangeData.WrappedData.NewValue);
		AttributeChangedNames.Add(AttributeName);
	}

	UFUNCTION()
	void HandleAttributeSetRegistered(UAngelscriptAttributeSet* AttributeSet)
	{
		bFired = true;
		FireCount++;
		CapturedAttributeSet = AttributeSet;
	}
};
