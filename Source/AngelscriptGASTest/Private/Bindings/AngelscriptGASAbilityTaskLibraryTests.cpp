// AngelscriptGASAbilityTaskLibraryTests.cpp
//
// CQTest coverage for UAngelscriptAbilityTaskLibrary:
//   - Null-safety for task creation methods with null OwningAbility
//   - Verifies library class is properly registered
//   - Covers representative subset of 30+ wrapper methods
//
// Automation IDs: Angelscript.GAS.Bindings.AbilityTaskLibrary.*

#include "AngelscriptAbilityTaskLibrary.h"
#include "CQTest.h"
#include "GameplayTagsManager.h"

#if WITH_ANGELSCRIPT_UNITTESTS

TEST_CLASS_WITH_FLAGS(FAngelscriptGASAbilityTaskLibraryTests,
	"Angelscript.GAS.Bindings.AbilityTaskLibrary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
{
	TEST_METHOD(LibraryClassIsRegistered)
	{
		UClass* LibClass = UAngelscriptAbilityTaskLibrary::StaticClass();
		TestRunner->TestNotNull(
			TEXT("UAngelscriptAbilityTaskLibrary should be a valid registered class"),
			LibClass);
		TestRunner->TestTrue(
			TEXT("UAngelscriptAbilityTaskLibrary should derive from UBlueprintFunctionLibrary"),
			LibClass->IsChildOf(UBlueprintFunctionLibrary::StaticClass()));
	}

	TEST_METHOD(WaitDelayWithNullAbilityReturnsNull)
	{
		UAbilityTask_WaitDelay* Task = UAngelscriptAbilityTaskLibrary::WaitDelay(nullptr, 1.0f);
		TestRunner->TestNull(
			TEXT("WaitDelay with null ability should return null"),
			Task);
	}

	TEST_METHOD(WaitNetSyncWithNullAbilityReturnsNull)
	{
		UAbilityTask_NetworkSyncPoint* Task =
			UAngelscriptAbilityTaskLibrary::WaitNetSync(nullptr, EAbilityTaskNetSyncType::OnlyServerWait);
		TestRunner->TestNull(
			TEXT("WaitNetSync with null ability should return null"),
			Task);
	}

	TEST_METHOD(RepeatActionWithNullAbilityReturnsNull)
	{
		UAbilityTask_Repeat* Task = UAngelscriptAbilityTaskLibrary::RepeatAction(nullptr, 0.5f, 3);
		TestRunner->TestNull(
			TEXT("RepeatAction with null ability should return null"),
			Task);
	}

	TEST_METHOD(WaitForCancelInputWithNullAbilityReturnsNull)
	{
		UAbilityTask_WaitCancel* Task = UAngelscriptAbilityTaskLibrary::WaitForCancelInput(nullptr);
		TestRunner->TestNull(
			TEXT("WaitForCancelInput with null ability should return null"),
			Task);
	}

	TEST_METHOD(WaitForConfirmInputWithNullAbilityReturnsNull)
	{
		UAbilityTask_WaitConfirm* Task = UAngelscriptAbilityTaskLibrary::WaitForConfirmInput(nullptr);
		TestRunner->TestNull(
			TEXT("WaitForConfirmInput with null ability should return null"),
			Task);
	}

	TEST_METHOD(WaitConfirmCancelWithNullAbilityReturnsNull)
	{
		UAbilityTask_WaitConfirmCancel* Task = UAngelscriptAbilityTaskLibrary::WaitConfirmCancel(nullptr);
		TestRunner->TestNull(
			TEXT("WaitConfirmCancel with null ability should return null"),
			Task);
	}

	TEST_METHOD(WaitInputPressWithNullAbilityReturnsNull)
	{
		UAbilityTask_WaitInputPress* Task = UAngelscriptAbilityTaskLibrary::WaitInputPress(nullptr, false);
		TestRunner->TestNull(
			TEXT("WaitInputPress with null ability should return null"),
			Task);
	}

	TEST_METHOD(WaitInputReleaseWithNullAbilityReturnsNull)
	{
		UAbilityTask_WaitInputRelease* Task = UAngelscriptAbilityTaskLibrary::WaitInputRelease(nullptr, false);
		TestRunner->TestNull(
			TEXT("WaitInputRelease with null ability should return null"),
			Task);
	}

	TEST_METHOD(WaitForOverlapWithNullAbilityReturnsNull)
	{
		UAbilityTask_WaitOverlap* Task = UAngelscriptAbilityTaskLibrary::WaitForOverlap(nullptr);
		TestRunner->TestNull(
			TEXT("WaitForOverlap with null ability should return null"),
			Task);
	}

	TEST_METHOD(WaitGameplayEventWithNullAbilityReturnsNull)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Test"), false);
		UAbilityTask_WaitGameplayEvent* Task =
			UAngelscriptAbilityTaskLibrary::WaitGameplayEvent(nullptr, Tag, nullptr, false, true);
		TestRunner->TestNull(
			TEXT("WaitGameplayEvent with null ability should return null"),
			Task);
	}

	TEST_METHOD(WaitGameplayTagAddWithNullAbilityReturnsNull)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Test"), false);
		UAbilityTask_WaitGameplayTagAdded* Task =
			UAngelscriptAbilityTaskLibrary::WaitGameplayTagAdd(nullptr, Tag, nullptr, false);
		TestRunner->TestNull(
			TEXT("WaitGameplayTagAdd with null ability should return null"),
			Task);
	}

	TEST_METHOD(WaitGameplayTagRemoveWithNullAbilityReturnsNull)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Test"), false);
		UAbilityTask_WaitGameplayTagRemoved* Task =
			UAngelscriptAbilityTaskLibrary::WaitGameplayTagRemove(nullptr, Tag, nullptr, false);
		TestRunner->TestNull(
			TEXT("WaitGameplayTagRemove with null ability should return null"),
			Task);
	}
};

#endif // WITH_ANGELSCRIPT_UNITTESTS
