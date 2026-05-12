// AngelscriptGASGameplayTagScriptTests.cpp
//
// CQTest coverage for gameplay tag query operations on UAngelscriptAbilitySystemComponent:
//   - HasGameplayTag
//   - HasAllGameplayTags
//   - HasAnyGameplayTags
//
// Automation IDs: Angelscript.GAS.Bindings.GameplayTagOps.*

#include "AngelscriptAbilitySystemComponent.h"
#include "Components/ActorTestSpawner.h"
#include "CQTest.h"
#include "GameplayTagsManager.h"

#if WITH_DEV_AUTOMATION_TESTS

TEST_CLASS_WITH_FLAGS(FAngelscriptGASGameplayTagScriptTests,
	"Angelscript.GAS.Bindings.GameplayTagOps",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
{
	FActorTestSpawner Spawner;

	UAngelscriptAbilitySystemComponent* CreateTestASC()
	{
		AActor& TestActor = Spawner.SpawnActor<AActor>();
		UAngelscriptAbilitySystemComponent* ASC = NewObject<UAngelscriptAbilitySystemComponent>(&TestActor);
		TestActor.AddInstanceComponent(ASC);
		ASC->RegisterComponent();
		ASC->InitAbilityActorInfo(&TestActor, &TestActor);
		return ASC;
	}

	FGameplayTag RequestTag(const TCHAR* TagName)
	{
		return FGameplayTag::RequestGameplayTag(FName(TagName), false);
	}

	TEST_METHOD(HasGameplayTagReturnsFalseWhenEmpty)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();
		FGameplayTag TestTag = RequestTag(TEXT("Ability.Test"));
		if (!TestTag.IsValid()) { TestRunner->AddInfo(TEXT("Tag not registered, skipping")); return; }

		TestRunner->TestFalse(
			TEXT("HasGameplayTag should return false when ASC has no tags"),
			ASC->HasGameplayTag(TestTag));
	}

	TEST_METHOD(HasGameplayTagReturnsTrueWhenPresent)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();
		FGameplayTag TestTag = RequestTag(TEXT("Ability.Test"));
		if (!TestTag.IsValid()) { TestRunner->AddInfo(TEXT("Tag not registered, skipping")); return; }

		ASC->AddLooseGameplayTag(TestTag);
		TestRunner->TestTrue(
			TEXT("HasGameplayTag should return true when the tag is present"),
			ASC->HasGameplayTag(TestTag));
	}

	TEST_METHOD(HasAllGameplayTagsReturnsTrueWhenAllPresent)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();
		FGameplayTag TagA = RequestTag(TEXT("Ability.Test"));
		FGameplayTag TagB = RequestTag(TEXT("Ability.Cooldown"));
		if (!TagA.IsValid() || !TagB.IsValid()) { TestRunner->AddInfo(TEXT("Required tags not registered, skipping")); return; }

		ASC->AddLooseGameplayTag(TagA);
		ASC->AddLooseGameplayTag(TagB);

		FGameplayTagContainer QueryContainer;
		QueryContainer.AddTag(TagA);
		QueryContainer.AddTag(TagB);

		TestRunner->TestTrue(
			TEXT("HasAllGameplayTags should return true when all queried tags are present"),
			ASC->HasAllGameplayTags(QueryContainer));
	}

	TEST_METHOD(HasAllGameplayTagsReturnsFalseWhenPartialMatch)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();
		FGameplayTag TagA = RequestTag(TEXT("Ability.Test"));
		FGameplayTag TagB = RequestTag(TEXT("Ability.Cooldown"));
		if (!TagA.IsValid() || !TagB.IsValid()) { TestRunner->AddInfo(TEXT("Required tags not registered, skipping")); return; }

		ASC->AddLooseGameplayTag(TagA);

		FGameplayTagContainer QueryContainer;
		QueryContainer.AddTag(TagA);
		QueryContainer.AddTag(TagB);

		TestRunner->TestFalse(
			TEXT("HasAllGameplayTags should return false when only some queried tags are present"),
			ASC->HasAllGameplayTags(QueryContainer));
	}

	TEST_METHOD(HasAnyGameplayTagsReturnsTrueOnPartialMatch)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();
		FGameplayTag TagA = RequestTag(TEXT("Ability.Test"));
		FGameplayTag TagB = RequestTag(TEXT("Ability.Cooldown"));
		if (!TagA.IsValid() || !TagB.IsValid()) { TestRunner->AddInfo(TEXT("Required tags not registered, skipping")); return; }

		ASC->AddLooseGameplayTag(TagA);

		FGameplayTagContainer QueryContainer;
		QueryContainer.AddTag(TagA);
		QueryContainer.AddTag(TagB);

		TestRunner->TestTrue(
			TEXT("HasAnyGameplayTags should return true when at least one queried tag is present"),
			ASC->HasAnyGameplayTags(QueryContainer));
	}

	TEST_METHOD(HasAnyGameplayTagsReturnsFalseWhenNoneMatch)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();
		FGameplayTag TagA = RequestTag(TEXT("Ability.Test"));
		FGameplayTag TagB = RequestTag(TEXT("Ability.Cooldown"));
		if (!TagA.IsValid() || !TagB.IsValid()) { TestRunner->AddInfo(TEXT("Required tags not registered, skipping")); return; }

		FGameplayTagContainer QueryContainer;
		QueryContainer.AddTag(TagA);
		QueryContainer.AddTag(TagB);

		TestRunner->TestFalse(
			TEXT("HasAnyGameplayTags should return false when no queried tags are present"),
			ASC->HasAnyGameplayTags(QueryContainer));
	}

	TEST_METHOD(HasAllGameplayTagsWithEmptyContainerReturnsTrue)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();
		FGameplayTagContainer EmptyContainer;

		TestRunner->TestTrue(
			TEXT("HasAllGameplayTags with empty container should return true (UE convention)"),
			ASC->HasAllGameplayTags(EmptyContainer));
	}

	TEST_METHOD(HasAnyGameplayTagsWithEmptyContainerReturnsFalse)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();
		FGameplayTagContainer EmptyContainer;

		TestRunner->TestFalse(
			TEXT("HasAnyGameplayTags with empty container should return false (UE convention)"),
			ASC->HasAnyGameplayTags(EmptyContainer));
	}

	TEST_METHOD(HasGameplayTagReturnsFalseAfterRemoval)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();
		FGameplayTag TestTag = RequestTag(TEXT("Ability.Test"));
		if (!TestTag.IsValid()) { TestRunner->AddInfo(TEXT("Tag not registered, skipping")); return; }

		ASC->AddLooseGameplayTag(TestTag);
		ASC->RemoveLooseGameplayTag(TestTag);

		TestRunner->TestFalse(
			TEXT("HasGameplayTag should return false after the tag is removed"),
			ASC->HasGameplayTag(TestTag));
	}

	TEST_METHOD(AddLooseGameplayTagMultipleTimesStillReturnsTrue)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();
		FGameplayTag TestTag = RequestTag(TEXT("Ability.Test"));
		if (!TestTag.IsValid()) { TestRunner->AddInfo(TEXT("Tag not registered, skipping")); return; }

		ASC->AddLooseGameplayTag(TestTag);
		ASC->AddLooseGameplayTag(TestTag);

		TestRunner->TestTrue(
			TEXT("HasGameplayTag should return true after adding the same tag multiple times"),
			ASC->HasGameplayTag(TestTag));
	}

	TEST_METHOD(RemoveLooseGameplayTagOnlyRemovesOneCount)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();
		FGameplayTag TestTag = RequestTag(TEXT("Ability.Test"));
		if (!TestTag.IsValid()) { TestRunner->AddInfo(TEXT("Tag not registered, skipping")); return; }

		ASC->AddLooseGameplayTag(TestTag);
		ASC->AddLooseGameplayTag(TestTag);
		ASC->RemoveLooseGameplayTag(TestTag);

		TestRunner->TestTrue(
			TEXT("HasGameplayTag should still return true after removing one count of a double-added tag"),
			ASC->HasGameplayTag(TestTag));
	}

	TEST_METHOD(HasAllGameplayTagsWithSingleTagWorks)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();
		FGameplayTag TestTag = RequestTag(TEXT("Ability.Test"));
		if (!TestTag.IsValid()) { TestRunner->AddInfo(TEXT("Tag not registered, skipping")); return; }

		ASC->AddLooseGameplayTag(TestTag);

		FGameplayTagContainer SingleContainer;
		SingleContainer.AddTag(TestTag);

		TestRunner->TestTrue(
			TEXT("HasAllGameplayTags with a single-tag container should return true when that tag is present"),
			ASC->HasAllGameplayTags(SingleContainer));
	}

	TEST_METHOD(HasAnyGameplayTagsWithSingleTagWorks)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();
		FGameplayTag TestTag = RequestTag(TEXT("Ability.Test"));
		if (!TestTag.IsValid()) { TestRunner->AddInfo(TEXT("Tag not registered, skipping")); return; }

		ASC->AddLooseGameplayTag(TestTag);

		FGameplayTagContainer SingleContainer;
		SingleContainer.AddTag(TestTag);

		TestRunner->TestTrue(
			TEXT("HasAnyGameplayTags with a single-tag container should return true when that tag is present"),
			ASC->HasAnyGameplayTags(SingleContainer));
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
