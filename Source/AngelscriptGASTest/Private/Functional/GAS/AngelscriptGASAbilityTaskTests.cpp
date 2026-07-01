// AngelscriptGASAbilityTaskTests.cpp
//
// CQTest coverage for UAngelscriptAbilityTask:
//   - Property accessors (IsTickingTask, IsPausable, IsSimulatedTask)
//   - CreateAbilityTask static factory (null-safety)
//   - Script subclass compilation
//
// Automation IDs: Angelscript.GAS.Functional.AbilityTask.*

#include "AngelscriptAbilityTask.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "Misc/ScopeExit.h"

#if WITH_ANGELSCRIPT_UNITTESTS


TEST_CLASS_WITH_FLAGS(FAngelscriptGASAbilityTaskTests,
	"Angelscript.GAS.Functional.AbilityTask",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
{
	// Acquire-once / reset-once: see AngelscriptGASAttributeSetOverrideTests.cpp
	// pilot for rationale.
	BEFORE_ALL()
	{
		ASTEST_CREATE_ENGINE();
	}

	AFTER_ALL()
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		ASTEST_RESET_ENGINE(Engine);
	}

	TEST_METHOD(DefaultTaskIsNotTicking)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>();
		TestRunner->TestFalse(
			TEXT("Default UAngelscriptAbilityTask should not be ticking"),
			Task->GetIsTickingTask());
	}

	TEST_METHOD(SetIsTickingTaskRoundTrips)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>();
		Task->SetIsTickingTask(true);
		TestRunner->TestTrue(
			TEXT("SetIsTickingTask(true) should be reflected by GetIsTickingTask"),
			Task->GetIsTickingTask());
		Task->SetIsTickingTask(false);
		TestRunner->TestFalse(
			TEXT("SetIsTickingTask(false) should be reflected by GetIsTickingTask"),
			Task->GetIsTickingTask());
	}

	TEST_METHOD(DefaultTaskIsPausable)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>();
		// UAbilityTask defaults bTickingTask=false, bSimulatedTask=false
		// but IsPausable defaults depend on base class — verify the getter works
		bool bPausable = Task->GetIsPausable();
		// Just verify it doesn't crash and returns a consistent value
		Task->SetIsPausable(!bPausable);
		TestRunner->TestEqual(
			TEXT("SetIsPausable should toggle the pausable state"),
			Task->GetIsPausable(), !bPausable);
	}

	TEST_METHOD(DefaultTaskIsNotSimulated)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>();
		TestRunner->TestFalse(
			TEXT("Default UAngelscriptAbilityTask should not be simulated"),
			Task->GetIsSimulatedTask());
	}

	TEST_METHOD(SetIsSimulatedTaskRoundTrips)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>();
		Task->SetIsSimulatedTask(true);
		TestRunner->TestTrue(
			TEXT("SetIsSimulatedTask(true) should be reflected by GetIsSimulatedTask"),
			Task->GetIsSimulatedTask());
		Task->SetIsSimulatedTask(false);
		TestRunner->TestFalse(
			TEXT("SetIsSimulatedTask(false) should be reflected by GetIsSimulatedTask"),
			Task->GetIsSimulatedTask());
	}

	TEST_METHOD(GetIsSimulatingDefaultFalse)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>();
		TestRunner->TestFalse(
			TEXT("Default UAngelscriptAbilityTask should not be simulating"),
			Task->GetIsSimulating());
	}

	TEST_METHOD(CreateAbilityTaskWithNullAbilityReturnsNull)
	{
		UAngelscriptAbilityTask* Task = UAngelscriptAbilityTask::CreateAbilityTask(
			UAngelscriptAbilityTask::StaticClass(), nullptr, NAME_None);
		TestRunner->TestNull(
			TEXT("CreateAbilityTask with null ability should return null"),
			Task);
	}

	TEST_METHOD(ScriptSubclassCompiles)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityTaskScript"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* TaskClass = CompileScriptModule(
			*TestRunner,
			Engine,
			ModuleName,
			TEXT("AbilityTaskScript.as"),
			TEXT(R"AS(
UCLASS()
class UTestCustomAbilityTask : UAngelscriptAbilityTask
{
	UPROPERTY()
	float CustomDuration = 2.0f;
}
)AS"),
			TEXT("UTestCustomAbilityTask"));
		if (TaskClass == nullptr) { return; }

		TestRunner->TestTrue(
			TEXT("Script subclass should derive from UAngelscriptAbilityTask"),
			TaskClass->IsChildOf(UAngelscriptAbilityTask::StaticClass()));

		FProperty* DurationProp = TaskClass->FindPropertyByName(TEXT("CustomDuration"));
		TestRunner->TestNotNull(
			TEXT("Script subclass should expose CustomDuration property"),
			DurationProp);
	}

	TEST_METHOD(BP_GetAbilitySystemComponentNullSafe)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>();
		UAbilitySystemComponent* ASC = Task->BP_GetAbilitySystemComponent();
		TestRunner->TestNull(
			TEXT("BP_GetAbilitySystemComponent on uninitialized task should return null"),
			ASC);
	}
};

#endif // WITH_ANGELSCRIPT_UNITTESTS
