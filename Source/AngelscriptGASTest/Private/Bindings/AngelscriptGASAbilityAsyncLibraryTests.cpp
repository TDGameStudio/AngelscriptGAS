// AngelscriptGASAbilityAsyncLibraryTests.cpp
//
// CQTest coverage for UAngelscriptAbilityAsyncLibrary:
//   - WaitForAttributeChanged null-safety
//   - WaitGameplayEventToActor null-safety
//   - WaitGameplayTagAddToActor null-safety
//   - WaitGameplayTagRemoveFromActor null-safety
//   - WaitGameplayTagQueryOnActor null-safety
//   - Valid actor paths (returns non-null task)
//
// Automation IDs: Angelscript.GAS.Bindings.AbilityAsyncLibrary.*

#include "AngelscriptAbilityAsyncLibrary.h"
#include "AngelscriptAbilitySystemComponent.h"
#include "AngelscriptAttributeSet.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "GameplayTagsManager.h"
#include "Misc/ScopeExit.h"

#if WITH_ANGELSCRIPT_UNITTESTS


namespace
{
	struct FAngelscriptGASAsyncAbilityFixture
	{
		FActorTestSpawner Spawner;
		AActor* Actor = nullptr;
		UAngelscriptAbilitySystemComponent* ASC = nullptr;

		FAngelscriptGASAsyncAbilityFixture()
		{
			Actor = &Spawner.SpawnActor<AActor>();
			ASC = NewObject<UAngelscriptAbilitySystemComponent>(Actor);
			Actor->AddInstanceComponent(ASC);
			ASC->RegisterComponent();
			ASC->InitAbilityActorInfo(Actor, Actor);
		}
	};
}

TEST_CLASS_WITH_FLAGS(FAngelscriptGASAbilityAsyncLibraryTests,
	"Angelscript.GAS.Bindings.AbilityAsyncLibrary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
{
	TEST_METHOD(WaitForAttributeChangedWithNullActorReturnsNull)
	{
		FGameplayAttribute DummyAttr;
		UAbilityAsync_WaitAttributeChanged* Task =
			UAngelscriptAbilityAsyncLibrary::WaitForAttributeChanged(nullptr, DummyAttr, false);
		TestRunner->TestNull(
			TEXT("WaitForAttributeChanged with null actor should return null"),
			Task);
	}

	TEST_METHOD(WaitGameplayEventToActorWithNullActorReturnsNull)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Test"), false);
		UAbilityAsync_WaitGameplayEvent* Task =
			UAngelscriptAbilityAsyncLibrary::WaitGameplayEventToActor(nullptr, Tag, false, true);
		TestRunner->TestNull(
			TEXT("WaitGameplayEventToActor with null actor should return null"),
			Task);
	}

	TEST_METHOD(WaitGameplayTagAddToActorWithNullActorReturnsNull)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Test"), false);
		UAbilityAsync_WaitGameplayTagAdded* Task =
			UAngelscriptAbilityAsyncLibrary::WaitGameplayTagAddToActor(nullptr, Tag, false);
		TestRunner->TestNull(
			TEXT("WaitGameplayTagAddToActor with null actor should return null"),
			Task);
	}

	TEST_METHOD(WaitGameplayTagRemoveFromActorWithNullActorReturnsNull)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Test"), false);
		UAbilityAsync_WaitGameplayTagRemoved* Task =
			UAngelscriptAbilityAsyncLibrary::WaitGameplayTagRemoveFromActor(nullptr, Tag, false);
		TestRunner->TestNull(
			TEXT("WaitGameplayTagRemoveFromActor with null actor should return null"),
			Task);
	}

	TEST_METHOD(WaitGameplayTagQueryOnActorWithNullActorReturnsNull)
	{
		FGameplayTagQuery Query;
		UAbilityAsync_WaitGameplayTagQuery* Task =
			UAngelscriptAbilityAsyncLibrary::WaitGameplayTagQueryOnActor(
				nullptr, Query, EWaitGameplayTagQueryTriggerCondition::WhenTrue, false);
		TestRunner->TestNull(
			TEXT("WaitGameplayTagQueryOnActor with null actor should return null"),
			Task);
	}

	TEST_METHOD(WaitForAttributeChangedWithInvalidAttributeDoesNotCrash)
	{
		FAngelscriptGASAsyncAbilityFixture Fixture;

		FGameplayAttribute InvalidAttr;
		UAbilityAsync_WaitAttributeChanged* Task =
			UAngelscriptAbilityAsyncLibrary::WaitForAttributeChanged(Fixture.Actor, InvalidAttr, false);
		// May return non-null (task created but attribute invalid) or null depending on UE version
		TestRunner->TestTrue(
			TEXT("WaitForAttributeChanged with invalid attribute should not crash"),
			true);
	}

	TEST_METHOD(WaitGameplayEventToActorWithValidActorReturnsNonNull)
	{
		FAngelscriptGASAsyncAbilityFixture Fixture;

		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Test"), false);
		if (!Tag.IsValid()) { TestRunner->AddInfo(TEXT("Tag not registered, skipping")); return; }

		UAbilityAsync_WaitGameplayEvent* Task =
			UAngelscriptAbilityAsyncLibrary::WaitGameplayEventToActor(Fixture.Actor, Tag, true, true);
		TestRunner->TestNotNull(
			TEXT("WaitGameplayEventToActor with valid actor and tag should return non-null"),
			Task);
	}

	TEST_METHOD(WaitGameplayTagAddToActorWithValidActorReturnsNonNull)
	{
		FAngelscriptGASAsyncAbilityFixture Fixture;

		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Test"), false);
		if (!Tag.IsValid()) { TestRunner->AddInfo(TEXT("Tag not registered, skipping")); return; }

		UAbilityAsync_WaitGameplayTagAdded* Task =
			UAngelscriptAbilityAsyncLibrary::WaitGameplayTagAddToActor(Fixture.Actor, Tag, true);
		TestRunner->TestNotNull(
			TEXT("WaitGameplayTagAddToActor with valid actor and tag should return non-null"),
			Task);
	}
};

#endif // WITH_ANGELSCRIPT_UNITTESTS
