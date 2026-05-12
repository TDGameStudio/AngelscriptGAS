// AngelscriptGASAbilityTaskLifecycleTests.cpp
//
// CQTest coverage for UAngelscriptAbilityTask lifecycle:
//   - Property accessors (IsTickingTask, IsPausable, IsSimulatedTask)
//   - BP_SetAbilitySystemComponent / BP_GetAbilitySystemComponent
//   - BP_IsWaitingOnRemotePlayerdata / BP_IsWaitingOnAvatar defaults
//   - BP_SetWaitingOnRemotePlayerData / BP_ClearWaitingOnRemotePlayerData
//   - BP_SetWaitingOnAvatar / BP_ClearWaitingOnAvatar
//   - CreateAbilityTask null-safety
//   - CreateAbilityTask with valid ability returns non-null
//
// Automation IDs: Angelscript.GAS.Functional.AbilityTaskLifecycle.*

#include "AngelscriptAbilityTask.h"
#include "AngelscriptAbilitySystemComponent.h"
#include "AngelscriptGASAbility.h"
#include "Components/ActorTestSpawner.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS

using namespace AngelscriptTestSupport;

TEST_CLASS_WITH_FLAGS(FAngelscriptGASAbilityTaskLifecycleTests,
	"Angelscript.GAS.Functional.AbilityTaskLifecycle",
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

	TEST_METHOD(SetIsTickingTaskModifiesState)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>(GetTransientPackage());
		TestRunner->TestFalse(TEXT("Default IsTickingTask should be false"), Task->GetIsTickingTask());

		Task->SetIsTickingTask(true);
		TestRunner->TestTrue(TEXT("SetIsTickingTask(true) should make GetIsTickingTask return true"), Task->GetIsTickingTask());

		Task->SetIsTickingTask(false);
		TestRunner->TestFalse(TEXT("SetIsTickingTask(false) should revert"), Task->GetIsTickingTask());
	}

	TEST_METHOD(SetIsPausableModifiesState)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>(GetTransientPackage());

		Task->SetIsPausable(true);
		TestRunner->TestTrue(TEXT("SetIsPausable(true) should make GetIsPausable return true"), Task->GetIsPausable());

		Task->SetIsPausable(false);
		TestRunner->TestFalse(TEXT("SetIsPausable(false) should revert"), Task->GetIsPausable());
	}

	TEST_METHOD(SetIsSimulatedTaskModifiesState)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>(GetTransientPackage());
		TestRunner->TestFalse(TEXT("Default IsSimulatedTask should be false"), Task->GetIsSimulatedTask());

		Task->SetIsSimulatedTask(true);
		TestRunner->TestTrue(TEXT("SetIsSimulatedTask(true) should make GetIsSimulatedTask return true"), Task->GetIsSimulatedTask());
	}

	TEST_METHOD(BP_SetAndGetAbilitySystemComponent)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>(GetTransientPackage());
		Task->BP_SetAbilitySystemComponent(ASC);

		TestRunner->TestEqual(
			TEXT("BP_GetAbilitySystemComponent should return the ASC set via BP_SetAbilitySystemComponent"),
			Task->BP_GetAbilitySystemComponent(), static_cast<UAbilitySystemComponent*>(ASC));
	}

	TEST_METHOD(BP_GetAbilitySystemComponentReturnsNullByDefault)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>(GetTransientPackage());
		TestRunner->TestNull(
			TEXT("BP_GetAbilitySystemComponent should return null when not set"),
			Task->BP_GetAbilitySystemComponent());
	}

	TEST_METHOD(BP_IsWaitingOnRemotePlayerdataDefaultReturnsFalse)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>(GetTransientPackage());
		TestRunner->TestFalse(
			TEXT("BP_IsWaitingOnRemotePlayerdata should return false by default"),
			Task->BP_IsWaitingOnRemotePlayerdata());
	}

	TEST_METHOD(BP_IsWaitingOnAvatarDefaultReturnsFalse)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>(GetTransientPackage());
		TestRunner->TestFalse(
			TEXT("BP_IsWaitingOnAvatar should return false by default"),
			Task->BP_IsWaitingOnAvatar());
	}

	TEST_METHOD(SetAndClearWaitingOnRemotePlayerDataRequiresOwningAbility)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>(GetTransientPackage());

		Task->BP_SetWaitingOnRemotePlayerData();
		TestRunner->TestFalse(
			TEXT("BP_SetWaitingOnRemotePlayerData without an owning ability should not mark the task waiting"),
			Task->BP_IsWaitingOnRemotePlayerdata());

		Task->BP_ClearWaitingOnRemotePlayerData();
		TestRunner->TestFalse(
			TEXT("After BP_ClearWaitingOnRemotePlayerData, IsWaitingOnRemotePlayerdata should return false"),
			Task->BP_IsWaitingOnRemotePlayerdata());
	}

	TEST_METHOD(SetAndClearWaitingOnAvatarRequiresOwningAbility)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>(GetTransientPackage());

		Task->BP_SetWaitingOnAvatar();
		TestRunner->TestFalse(
			TEXT("BP_SetWaitingOnAvatar without an owning ability should not mark the task waiting"),
			Task->BP_IsWaitingOnAvatar());

		Task->BP_ClearWaitingOnAvatar();
		TestRunner->TestFalse(
			TEXT("After BP_ClearWaitingOnAvatar, IsWaitingOnAvatar should return false"),
			Task->BP_IsWaitingOnAvatar());
	}

	TEST_METHOD(CreateAbilityTaskWithNullAbilityDoesNotCrash)
	{
		UAngelscriptAbilityTask* Task = UAngelscriptAbilityTask::CreateAbilityTask(
			UAngelscriptAbilityTask::StaticClass(), nullptr, NAME_None);
		TestRunner->TestNull(
			TEXT("CreateAbilityTask with null ability should return null"),
			Task);
	}

	TEST_METHOD(CreateAbilityTaskWithNullTaskTypeDoesNotCrash)
	{
		UAngelscriptAbilityTask* Task = UAngelscriptAbilityTask::CreateAbilityTask(
			nullptr, nullptr, NAME_None);
		TestRunner->TestNull(
			TEXT("CreateAbilityTask with null task type should return null"),
			Task);
	}

	TEST_METHOD(GetIsSimulatingDefaultReturnsFalse)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>(GetTransientPackage());
		TestRunner->TestFalse(
			TEXT("GetIsSimulating should return false by default"),
			Task->GetIsSimulating());
	}

	TEST_METHOD(BP_GetAbilityReturnsNullByDefault)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>(GetTransientPackage());
		TestRunner->TestNull(
			TEXT("BP_GetAbility should return null by default when no ability is set"),
			Task->BP_GetAbility(false));
	}

	TEST_METHOD(BP_IsPredictingClientReturnsFalseByDefault)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>(GetTransientPackage());
		TestRunner->TestFalse(
			TEXT("BP_IsPredictingClient should return false by default"),
			Task->BP_IsPredictingClient());
	}

	TEST_METHOD(BP_IsForRemoteClientReturnsFalseByDefault)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>(GetTransientPackage());
		TestRunner->TestFalse(
			TEXT("BP_IsForRemoteClient should return false by default"),
			Task->BP_IsForRemoteClient());
	}

	TEST_METHOD(BP_IsLocallyControlledReturnsFalseByDefault)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>(GetTransientPackage());
		TestRunner->TestFalse(
			TEXT("BP_IsLocallyControlled should return false by default"),
			Task->BP_IsLocallyControlled());
	}

	TEST_METHOD(BP_ShouldBroadcastAbilityTaskDelegatesReturnsFalseByDefault)
	{
		UAngelscriptAbilityTask* Task = NewObject<UAngelscriptAbilityTask>(GetTransientPackage());
		TestRunner->TestFalse(
			TEXT("BP_ShouldBroadcastAbilityTaskDelegates should return false without an active owning ability"),
			Task->BP_ShouldBroadcastAbilityTaskDelegates());
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
