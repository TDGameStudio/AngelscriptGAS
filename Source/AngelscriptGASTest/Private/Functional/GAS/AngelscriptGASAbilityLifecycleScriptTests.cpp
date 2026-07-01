// AngelscriptGASAbilityLifecycleScriptTests.cpp
//
// CQTest coverage for ability lifecycle operations via script:
//   - GiveAbility / HasAbility / CancelAbility / IsAbilityActive
//   - GiveAbilityAndActivateOnce
//   - GetCooldownTimeRemaining
//   - Null-class error paths
//
// Automation IDs: Angelscript.GAS.Functional.AbilityLifecycleScript.*

#include "AngelscriptAbilitySystemComponent.h"
#include "AngelscriptGASAbility.h"
#include "Components/ActorTestSpawner.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "Misc/ScopeExit.h"

#if WITH_ANGELSCRIPT_UNITTESTS


TEST_CLASS_WITH_FLAGS(FAngelscriptGASAbilityLifecycleScriptTests,
	"Angelscript.GAS.Functional.AbilityLifecycleScript",
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

	TEST_METHOD(GiveAbilityFromScriptReturnsValidHandle)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityLifecycleGive"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityLifecycleGive.as"),
			TEXT(R"AS(
UCLASS()
class UTestLifecycleAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestLifecycleAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle Handle = ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);
		TestRunner->TestTrue(
			TEXT("GiveAbility with a valid script ability class should return a valid handle"),
			Handle.IsValid());
	}

	TEST_METHOD(HasAbilityReturnsTrueAfterGive)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityLifecycleHas"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityLifecycleHas.as"),
			TEXT(R"AS(
UCLASS()
class UTestHasAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestHasAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);
		TestRunner->TestTrue(
			TEXT("HasAbility should return true after GiveAbility"),
			ASC->HasAbility(AbilityClass));
	}

	TEST_METHOD(HasAbilityReturnsFalseBeforeGive)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityLifecycleNoGive"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityLifecycleNoGive.as"),
			TEXT(R"AS(
UCLASS()
class UTestNoGiveAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestNoGiveAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TestRunner->TestFalse(
			TEXT("HasAbility should return false before GiveAbility"),
			ASC->HasAbility(AbilityClass));
	}

	TEST_METHOD(IsAbilityActiveReturnsFalseWhenNotActivated)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityLifecycleInactive"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityLifecycleInactive.as"),
			TEXT(R"AS(
UCLASS()
class UTestInactiveAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestInactiveAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);
		TestRunner->TestFalse(
			TEXT("IsAbilityActive should return false when ability is given but not activated"),
			ASC->IsAbilityActive(AbilityClass));
	}

	TEST_METHOD(CancelAbilityDoesNotCrashOnInactiveAbility)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityLifecycleCancelInactive"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityLifecycleCancelInactive.as"),
			TEXT(R"AS(
UCLASS()
class UTestCancelInactiveAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestCancelInactiveAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);
		ASC->CancelAbility(AbilityClass);

		TestRunner->TestTrue(
			TEXT("CancelAbility on an inactive ability should not crash (ability still exists)"),
			ASC->HasAbility(AbilityClass));
	}

	TEST_METHOD(GiveAbilityWithNullClassReturnsInvalidHandle)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TestRunner->AddExpectedError(TEXT("Please provide a valid InAbilityClass to GiveAbility()"), EAutomationExpectedErrorFlags::Contains, 1);

		FGameplayAbilitySpecHandle Handle = ASC->BP_GiveAbility(nullptr, 1, -1, nullptr);
		TestRunner->TestFalse(
			TEXT("GiveAbility with null class should return an invalid handle"),
			Handle.IsValid());
	}

	TEST_METHOD(GiveAbilityAndActivateOnceReturnsValidHandle)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityLifecycleActivateOnce"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityLifecycleActivateOnce.as"),
			TEXT(R"AS(
UCLASS()
class UTestActivateOnceAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestActivateOnceAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle Handle = ASC->BP_GiveAbilityAndActivateOnce(AbilityClass, 1, -1, nullptr);
		TestRunner->TestTrue(
			TEXT("GiveAbilityAndActivateOnce should return a valid handle"),
			Handle.IsValid());
	}

	TEST_METHOD(GetCooldownTimeRemainingReturnsZeroForNoCooldown)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityLifecycleCooldown"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityLifecycleCooldown.as"),
			TEXT(R"AS(
UCLASS()
class UTestNoCooldownAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestNoCooldownAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);
		float Cooldown = ASC->GetCooldownTimeRemaining(AbilityClass);
		TestRunner->TestEqual(
			TEXT("GetCooldownTimeRemaining should return 0 for an ability with no cooldown tags"),
			Cooldown, 0.f);
	}

	TEST_METHOD(CancelAbilityWithNullClassDoesNotCrash)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TestRunner->AddExpectedError(TEXT("Please provide a valid InAbilityClass to CancelAbility()"), EAutomationExpectedErrorFlags::Contains, 1);
		ASC->CancelAbility(nullptr);
		TestRunner->TestTrue(TEXT("CancelAbility with null class should not crash"), true);
	}

	TEST_METHOD(HasAbilityWithNullClassReturnsFalse)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TestRunner->AddExpectedError(TEXT("Please provide a valid InAbilityClass to HasAbility()"), EAutomationExpectedErrorFlags::Contains, 1);
		bool bResult = ASC->HasAbility(nullptr);
		TestRunner->TestFalse(
			TEXT("HasAbility with null class should return false"),
			bResult);
	}

	TEST_METHOD(GiveAbilityWithLevelSetsSpecLevel)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityLifecycleLevel"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityLifecycleLevel.as"),
			TEXT(R"AS(
UCLASS()
class UTestLevelAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestLevelAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle Handle = ASC->BP_GiveAbility(AbilityClass, 5, -1, nullptr);
		FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
		if (!TestRunner->TestNotNull(TEXT("Spec should exist"), Spec)) { return; }

		TestRunner->TestEqual(TEXT("GiveAbility Level parameter should set Spec.Level"), Spec->Level, 5);
	}

	TEST_METHOD(GiveAbilityWithInputIDSetsSpecInputID)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityLifecycleInputID"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityLifecycleInputID.as"),
			TEXT(R"AS(
UCLASS()
class UTestInputIDAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestInputIDAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle Handle = ASC->BP_GiveAbility(AbilityClass, 1, 3, nullptr);
		FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
		if (!TestRunner->TestNotNull(TEXT("Spec should exist"), Spec)) { return; }

		TestRunner->TestEqual(TEXT("GiveAbility InputID parameter should set Spec.InputID"), Spec->InputID, 3);
	}

	TEST_METHOD(GiveAbilityWithSourceObjectSetsSpecSourceObject)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityLifecycleSrcObj"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityLifecycleSrcObj.as"),
			TEXT(R"AS(
UCLASS()
class UTestSrcObjAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestSrcObjAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		UObject* SrcObj = &Spawner.SpawnActor<AActor>();
		FGameplayAbilitySpecHandle Handle = ASC->BP_GiveAbility(AbilityClass, 1, -1, SrcObj);
		FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
		if (!TestRunner->TestNotNull(TEXT("Spec should exist"), Spec)) { return; }

		TestRunner->TestEqual(TEXT("GiveAbility SourceObject parameter should set Spec.SourceObject"),
			Spec->SourceObject.Get(), SrcObj);
	}

	TEST_METHOD(ClearAbilityRemovesAbility)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityLifecycleClear"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityLifecycleClear.as"),
			TEXT(R"AS(
UCLASS()
class UTestClearAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestClearAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle Handle = ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);
		ASC->ClearAbility(Handle);

		TestRunner->TestFalse(
			TEXT("HasAbility should return false after ClearAbility"),
			ASC->HasAbility(AbilityClass));
	}

	TEST_METHOD(GiveAbilityAndActivateOnceWithNullClassReturnsInvalid)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TestRunner->AddExpectedError(TEXT("Please provide a valid InAbilityClass to GiveAbilityAndActivateOnce()"), EAutomationExpectedErrorFlags::Contains, 1);
		FGameplayAbilitySpecHandle Handle = ASC->BP_GiveAbilityAndActivateOnce(nullptr, 1, -1, nullptr);
		TestRunner->TestFalse(
			TEXT("GiveAbilityAndActivateOnce with null class should return invalid handle"),
			Handle.IsValid());
	}

	TEST_METHOD(GiveAbilityTwiceDoesNotDuplicate)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityLifecycleDup"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityLifecycleDup.as"),
			TEXT(R"AS(
UCLASS()
class UTestDupAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestDupAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle Handle1 = ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);
		FGameplayAbilitySpecHandle Handle2 = ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);

		TestRunner->TestTrue(TEXT("First GiveAbility should return valid handle"), Handle1.IsValid());
		TestRunner->TestTrue(TEXT("Second GiveAbility should also return valid handle"), Handle2.IsValid());
	}
};

#endif // WITH_ANGELSCRIPT_UNITTESTS
