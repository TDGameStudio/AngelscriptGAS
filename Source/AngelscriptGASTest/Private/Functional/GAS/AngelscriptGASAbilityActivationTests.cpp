// AngelscriptGASAbilityActivationTests.cpp
//
// CQTest coverage for ability activation and cancellation APIs:
//   - TryActivateAbilitySpec
//   - CancelAbilityByHandle / CancelAbilitiesByTags
//   - CanActivateAbilityByClass / CanActivateAbilitySpec
//   - GetActiveAbilitiesWithTags
//   - SetAbilitySpecSourceObject / GetAbilitySpecSourceObject
//   - BP_SetRemoveAbilityOnEnd
//
// Automation IDs: Angelscript.GAS.Functional.AbilityActivation.*

#include "AngelscriptAbilitySystemComponent.h"
#include "AngelscriptGASAbility.h"
#include "Components/ActorTestSpawner.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS


TEST_CLASS_WITH_FLAGS(FAngelscriptGASAbilityActivationTests,
	"Angelscript.GAS.Functional.AbilityActivation",
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

	TEST_METHOD(TryActivateAbilitySpecReturnsFalseOnInvalidHandle)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle InvalidHandle;
		bool bResult = ASC->TryActivateAbilitySpec(InvalidHandle, false);
		TestRunner->TestFalse(
			TEXT("TryActivateAbilitySpec with invalid handle should return false"),
			bResult);
	}

	TEST_METHOD(CancelAbilityByHandleDoesNotCrashOnInvalidHandle)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle InvalidHandle;
		ASC->CancelAbilityByHandle(InvalidHandle);
		TestRunner->TestTrue(
			TEXT("CancelAbilityByHandle with invalid handle should not crash"),
			true);
	}

	TEST_METHOD(CancelAbilitiesByTagsDoesNotCrashWithEmptyTags)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayTagContainer EmptyWith;
		FGameplayTagContainer EmptyWithout;
		ASC->CancelAbilitiesByTags(EmptyWith, EmptyWithout, nullptr);
		TestRunner->TestTrue(
			TEXT("CancelAbilitiesByTags with empty tag containers should not crash"),
			true);
	}

	TEST_METHOD(CanActivateAbilityByClassReturnsFalseWhenNotGranted)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityActivNotGranted"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityActivNotGranted.as"),
			TEXT(R"AS(
UCLASS()
class UTestNotGrantedAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestNotGrantedAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TestRunner->TestFalse(
			TEXT("CanActivateAbilityByClass should return false when ability is not granted"),
			ASC->CanActivateAbilityByClass(AbilityClass));
	}

	TEST_METHOD(CanActivateAbilityByClassReturnsTrueWhenGranted)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityActivGranted"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityActivGranted.as"),
			TEXT(R"AS(
UCLASS()
class UTestGrantedAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestGrantedAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);
		TestRunner->TestTrue(
			TEXT("CanActivateAbilityByClass should return true when ability is granted"),
			ASC->CanActivateAbilityByClass(AbilityClass));
	}

	TEST_METHOD(CanActivateAbilitySpecReturnsFalseForInvalidHandle)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle InvalidHandle;
		TestRunner->TestFalse(
			TEXT("CanActivateAbilitySpec should return false for invalid handle"),
			ASC->CanActivateAbilitySpec(InvalidHandle));
	}

	TEST_METHOD(CanActivateAbilitySpecReturnsTrueForValidSpec)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityActivValidSpec"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityActivValidSpec.as"),
			TEXT(R"AS(
UCLASS()
class UTestValidSpecAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestValidSpecAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle Handle = ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);
		TestRunner->TestTrue(
			TEXT("CanActivateAbilitySpec should return true for a valid granted ability spec"),
			ASC->CanActivateAbilitySpec(Handle));
	}

	TEST_METHOD(GetActiveAbilitiesWithTagsReturnsEmptyWhenNoneActive)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayTagContainer QueryTags;
		FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Test"), false);
		if (TestTag.IsValid())
		{
			QueryTags.AddTag(TestTag);
		}

		TArray<UGameplayAbility*> ActiveAbilities;
		ASC->GetActiveAbilitiesWithTags(QueryTags, ActiveAbilities);
		TestRunner->TestEqual(
			TEXT("GetActiveAbilitiesWithTags should return empty array when no abilities are active"),
			ActiveAbilities.Num(), 0);
	}

	TEST_METHOD(SetAbilitySpecSourceObjectStoresObject)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityActivSrcObj"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityActivSrcObj.as"),
			TEXT(R"AS(
UCLASS()
class UTestSrcObjAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestSrcObjAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle Handle = ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);

		UObject* SourceObj = &Spawner.SpawnActor<AActor>();
		ASC->SetAbilitySpecSourceObject(Handle, SourceObj);

		UObject* Retrieved = ASC->GetAbilitySpecSourceObject(Handle);
		TestRunner->TestEqual(
			TEXT("GetAbilitySpecSourceObject should return the object set via SetAbilitySpecSourceObject"),
			Retrieved, SourceObj);
	}

	TEST_METHOD(GetAbilitySpecSourceObjectReturnsNullForInvalidHandle)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle InvalidHandle;
		UObject* Result = ASC->GetAbilitySpecSourceObject(InvalidHandle);
		TestRunner->TestNull(
			TEXT("GetAbilitySpecSourceObject should return null for invalid handle"),
			Result);
	}

	TEST_METHOD(BP_SetRemoveAbilityOnEndClearsInactiveSpec)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityActivRemoveEnd"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityActivRemoveEnd.as"),
			TEXT(R"AS(
UCLASS()
class UTestRemoveEndAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestRemoveEndAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle Handle = ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);
		FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
		if (!TestRunner->TestNotNull(TEXT("Spec should exist after GiveAbility"), Spec)) { return; }

		ASC->BP_SetRemoveAbilityOnEnd(Handle);

		TestRunner->TestNull(
			TEXT("BP_SetRemoveAbilityOnEnd should clear an inactive spec immediately"),
			ASC->FindAbilitySpecFromHandle(Handle));
	}

	TEST_METHOD(TryActivateAbilitySpecReturnsTrueOnSuccess)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityActivSuccess"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityActivSuccess.as"),
			TEXT(R"AS(
UCLASS()
class UTestActivateSuccessAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestActivateSuccessAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle Handle = ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);
		bool bResult = ASC->TryActivateAbilitySpec(Handle, false);
		TestRunner->TestTrue(
			TEXT("TryActivateAbilitySpec with valid handle should return true"),
			bResult);
	}

	TEST_METHOD(CanActivateAbilityByClassReturnsFalseForNullClass)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TestRunner->TestFalse(
			TEXT("CanActivateAbilityByClass should return false for null class"),
			ASC->CanActivateAbilityByClass(nullptr));
	}

	TEST_METHOD(SetAbilitySpecSourceObjectWithInvalidHandleAfterOtherOperationsDoesNotCrash)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle InvalidHandle;
		UObject* SomeObj = &Spawner.SpawnActor<AActor>();
		ASC->SetAbilitySpecSourceObject(InvalidHandle, SomeObj);
		TestRunner->TestTrue(
			TEXT("SetAbilitySpecSourceObject with invalid handle should not crash"),
			true);
	}

	TEST_METHOD(GetAbilitySpecSourceObjectReturnsNullWhenNotSet)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityActivNoSrc"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityActivNoSrc.as"),
			TEXT(R"AS(
UCLASS()
class UTestNoSrcAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestNoSrcAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle Handle = ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);
		UObject* Result = ASC->GetAbilitySpecSourceObject(Handle);
		TestRunner->TestNull(
			TEXT("GetAbilitySpecSourceObject should return null when no source object was set"),
			Result);
	}

	TEST_METHOD(ActivateAbilitiesUsingTagsReturnsFalseWhenNoAbilitiesMatch)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayTagContainer Tags;
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Test"), false);
		if (!Tag.IsValid()) { TestRunner->AddInfo(TEXT("Tag not registered, skipping")); return; }
		Tags.AddTag(Tag);

		bool bResult = ASC->ActivateAbilitiesUsingTags(Tags, false);
		TestRunner->TestFalse(
			TEXT("ActivateAbilitiesUsingTags should return false when no abilities match the tags"),
			bResult);
	}

	TEST_METHOD(ActivateAbilitiesUsingTagsWithEmptyContainerReturnsFalse)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayTagContainer EmptyTags;
		bool bResult = ASC->ActivateAbilitiesUsingTags(EmptyTags, false);
		TestRunner->TestFalse(
			TEXT("ActivateAbilitiesUsingTags with empty container should return false"),
			bResult);
	}

	TEST_METHOD(GetActiveAbilitiesWithTagsReturnsEmptyArrayWhenNoAbilitiesGranted)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayTagContainer Tags;
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Test"), false);
		if (!Tag.IsValid()) { TestRunner->AddInfo(TEXT("Tag not registered, skipping")); return; }
		Tags.AddTag(Tag);

		TArray<UGameplayAbility*> ActiveAbilities;
		ASC->GetActiveAbilitiesWithTags(Tags, ActiveAbilities);
		TestRunner->TestEqual(
			TEXT("GetActiveAbilitiesWithTags should return empty array when no abilities are granted"),
			ActiveAbilities.Num(), 0);
	}

	TEST_METHOD(SetAbilitySpecSourceObjectUpdatesExistingSpec)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AbilityActivSetSrc"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AbilityActivSetSrc.as"),
			TEXT(R"AS(
UCLASS()
class UTestSetSrcAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestSetSrcAbility"));
		if (AbilityClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle Handle = ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);
		UObject* NewSrc = &Spawner.SpawnActor<AActor>();
		ASC->SetAbilitySpecSourceObject(Handle, NewSrc);

		UObject* Result = ASC->GetAbilitySpecSourceObject(Handle);
		TestRunner->TestEqual(
			TEXT("SetAbilitySpecSourceObject should update the source object on an existing spec"),
			Result, NewSrc);
	}

	TEST_METHOD(SetAbilitySpecSourceObjectWithInvalidHandleDoesNotCrash)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		FGameplayAbilitySpecHandle InvalidHandle;
		UObject* SomeObj = &Spawner.SpawnActor<AActor>();
		ASC->SetAbilitySpecSourceObject(InvalidHandle, SomeObj);
		TestRunner->TestTrue(
			TEXT("SetAbilitySpecSourceObject with invalid handle should not crash"),
			true);
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
