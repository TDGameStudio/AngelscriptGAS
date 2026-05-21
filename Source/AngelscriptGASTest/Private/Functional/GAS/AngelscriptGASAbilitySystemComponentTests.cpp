// AngelscriptGASAbilitySystemComponentTests.cpp
//
// CQTest coverage for UAngelscriptAbilitySystemComponent:
//   - Component creation and defaults
//   - Attribute set registration
//   - Attribute value get/set operations
//   - Basic ability give/has/cancel flow
//
// Automation IDs: Angelscript.GAS.Functional.AbilitySystemComponent.*

#include "AngelscriptAbilitySystemComponent.h"
#include "AngelscriptAttributeSet.h"
#include "AngelscriptGASActor.h"
#include "Components/ActorTestSpawner.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS

using namespace AngelscriptTestSupport;

TEST_CLASS_WITH_FLAGS(FAngelscriptGASAbilitySystemComponentTests,
	"Angelscript.GAS.Functional.AbilitySystemComponent",
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

	TEST_METHOD(RegisterAttributeSetCreatesInstance)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("ASCRegAttrSet"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner,
			Engine,
			ModuleName,
			TEXT("ASCRegAttrSet.as"),
			TEXT(R"AS(
UCLASS()
class UASCTestAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData MaxHealth;
}
)AS"),
			TEXT("UASCTestAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		UAngelscriptAttributeSet* RegisteredSet = ASC->RegisterAttributeSet(AttrSetSubclass);

		TestRunner->TestNotNull(
			TEXT("RegisterAttributeSet should return a non-null attribute set instance"),
			RegisteredSet);

		if (RegisteredSet)
		{
			TestRunner->TestTrue(
				TEXT("Registered attribute set should be of the expected class"),
				RegisteredSet->IsA(AttributeSetClass));
		}
	}

	TEST_METHOD(RegisterAttributeSetNoDuplicates)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("ASCNoDupAttr"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner,
			Engine,
			ModuleName,
			TEXT("ASCNoDupAttr.as"),
			TEXT(R"AS(
UCLASS()
class UASCNoDupAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Stamina;
}
)AS"),
			TEXT("UASCNoDupAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		UAngelscriptAttributeSet* FirstSet = ASC->RegisterAttributeSet(AttrSetSubclass);
		UAngelscriptAttributeSet* SecondSet = ASC->RegisterAttributeSet(AttrSetSubclass);

		TestRunner->TestNotNull(TEXT("First RegisterAttributeSet call should succeed"), FirstSet);
		TestRunner->TestEqual(
			TEXT("Second RegisterAttributeSet call should return the same instance (no duplicates)"),
			SecondSet, FirstSet);
	}

	TEST_METHOD(GetAttributeCurrentValueReturnsDefault)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("ASCGetAttrVal"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner,
			Engine,
			ModuleName,
			TEXT("ASCGetAttrVal.as"),
			TEXT(R"AS(
UCLASS()
class UASCGetValAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Energy;
}
)AS"),
			TEXT("UASCGetValAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);

		float DefaultVal = -999.f;
		float CurrentValue = ASC->GetAttributeCurrentValue(AttrSetSubclass, TEXT("Energy"), DefaultVal);

		TestRunner->TestEqual(
			TEXT("GetAttributeCurrentValue for a freshly registered attribute should return 0 (initial value)"),
			CurrentValue, 0.f);
	}

	TEST_METHOD(TrySetAndGetAttributeBaseValue)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("ASCSetGetBase"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner,
			Engine,
			ModuleName,
			TEXT("ASCSetGetBase.as"),
			TEXT(R"AS(
UCLASS()
class UASCSetGetBaseAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Power;
}
)AS"),
			TEXT("UASCSetGetBaseAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);

		bool bSetSuccess = ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Power"), 42.f);
		TestRunner->TestTrue(TEXT("TrySetAttributeBaseValue should succeed for registered attribute"), bSetSuccess);

		float OutBaseValue = 0.f;
		bool bGetSuccess = ASC->TryGetAttributeBaseValue(AttrSetSubclass, TEXT("Power"), OutBaseValue);
		TestRunner->TestTrue(TEXT("TryGetAttributeBaseValue should succeed for registered attribute"), bGetSuccess);
		TestRunner->TestEqual(TEXT("TryGetAttributeBaseValue should return the value we set"), OutBaseValue, 42.f);
	}

	TEST_METHOD(TrySetAttributeBaseValueFailsForMissing)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		bool bSetSuccess = ASC->TrySetAttributeBaseValue(nullptr, TEXT("Nonexistent"), 10.f);
		TestRunner->TestFalse(
			TEXT("TrySetAttributeBaseValue should fail for null attribute set class"),
			bSetSuccess);
	}

	TEST_METHOD(GetAttributeCurrentValueReturnsDefaultForMissing)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		const float Sentinel = -123.f;
		float Value = ASC->GetAttributeCurrentValue(nullptr, TEXT("Missing"), Sentinel);
		TestRunner->TestEqual(
			TEXT("GetAttributeCurrentValue should return DefaultValue when attribute set class is null"),
			Value, Sentinel);
	}

	TEST_METHOD(RegisterAttributeSetReturnsNullForNullClass)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TestRunner->AddExpectedError(TEXT("Please provide a valid AttributeSetClass to RegisterAttributeSet()"), EAutomationExpectedErrorFlags::Contains, 1);
		TSubclassOf<UAngelscriptAttributeSet> NullClass(nullptr);
		UAngelscriptAttributeSet* Result = ASC->RegisterAttributeSet(NullClass);
		TestRunner->TestNull(
			TEXT("RegisterAttributeSet with null class should return null"),
			Result);
	}

	TEST_METHOD(TrySetAttributeBaseValueReturnsFalseForNullClass)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		bool bResult = ASC->TrySetAttributeBaseValue(nullptr, TEXT("Health"), 100.f);
		TestRunner->TestFalse(
			TEXT("TrySetAttributeBaseValue with null class should return false"),
			bResult);
	}

	TEST_METHOD(TryGetAttributeCurrentValueReturnsFalseForNullClass)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		float OutValue = 0.f;
		bool bResult = ASC->TryGetAttributeCurrentValue(nullptr, TEXT("Health"), OutValue);
		TestRunner->TestFalse(
			TEXT("TryGetAttributeCurrentValue with null class should return false"),
			bResult);
	}

	TEST_METHOD(GetAttributeBaseValueReturnsDefaultForNullClass)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		const float Sentinel = -555.f;
		float Value = ASC->GetAttributeBaseValue(nullptr, TEXT("Health"), Sentinel);
		TestRunner->TestEqual(
			TEXT("GetAttributeBaseValue with null class should return the default value"),
			Value, Sentinel);
	}

	TEST_METHOD(SetAttributeBaseValueWithInvalidAttributeDoesNotCrash)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("ASCInvalidAttrSet"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner,
			Engine,
			ModuleName,
			TEXT("ASCInvalidAttrSet.as"),
			TEXT(R"AS(
UCLASS()
class UASCInvalidAttrAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Mana;
}
)AS"),
			TEXT("UASCInvalidAttrAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);

		bool bResult = ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("NonExistentAttribute"), 50.f);
		TestRunner->TestFalse(
			TEXT("TrySetAttributeBaseValue with non-existent attribute name should return false and not crash"),
			bResult);
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
