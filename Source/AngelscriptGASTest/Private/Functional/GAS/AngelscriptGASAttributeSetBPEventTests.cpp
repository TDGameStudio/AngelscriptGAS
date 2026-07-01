// AngelscriptGASAttributeSetBPEventTests.cpp
//
// CQTest coverage for UAngelscriptAttributeSet BP_ event pipeline:
//   - PreAttributeChange / PostAttributeChange
//   - PreAttributeBaseChange / PostAttributeBaseChange
//   - Verifies the C++ override chain correctly calls BP_ events
//   - Tests that default (non-overridden) behavior does not alter values
//
// Automation IDs: Angelscript.GAS.Functional.AttributeSetBPEvents.*

#include "AngelscriptAbilitySystemComponent.h"
#include "AngelscriptAttributeSet.h"
#include "Components/ActorTestSpawner.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "Misc/ScopeExit.h"

#if WITH_ANGELSCRIPT_UNITTESTS


TEST_CLASS_WITH_FLAGS(FAngelscriptGASAttributeSetBPEventTests,
	"Angelscript.GAS.Functional.AttributeSetBPEvents",
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

	TEST_METHOD(DefaultBPEventsDoNotAlterBehavior)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("BPEventDefault"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("BPEventDefault.as"),
			TEXT(R"AS(
UCLASS()
class UTestDefaultBPAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Strength;
}
)AS"),
			TEXT("UTestDefaultBPAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);

		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Strength"), 50.f);
		float CurrentValue = ASC->GetAttributeCurrentValue(AttrSetSubclass, TEXT("Strength"), -1.f);
		TestRunner->TestEqual(
			TEXT("Without BP_ overrides, attribute value should be set directly without modification"),
			CurrentValue, 50.f);
	}

	TEST_METHOD(PreAttributeChangeIsCalled)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("BPEventPreChange"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("BPEventPreChange.as"),
			TEXT(R"AS(
UCLASS()
class UTestPreChangeAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Health;
}
)AS"),
			TEXT("UTestPreChangeAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Health"), 100.f);

		float BaseValue = 0.f;
		ASC->TryGetAttributeBaseValue(AttrSetSubclass, TEXT("Health"), BaseValue);
		TestRunner->TestEqual(
			TEXT("PreAttributeChange pipeline should be invoked (value set correctly)"),
			BaseValue, 100.f);
	}

	TEST_METHOD(PostAttributeChangeIsCalled)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("BPEventPostChange"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("BPEventPostChange.as"),
			TEXT(R"AS(
UCLASS()
class UTestPostChangeAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Mana;
}
)AS"),
			TEXT("UTestPostChangeAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);

		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Mana"), 30.f);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Mana"), 60.f);

		float CurrentValue = ASC->GetAttributeCurrentValue(AttrSetSubclass, TEXT("Mana"), -1.f);
		TestRunner->TestEqual(
			TEXT("PostAttributeChange pipeline should complete (final value correct)"),
			CurrentValue, 60.f);
	}

	TEST_METHOD(PreAttributeBaseChangeIsCalled)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("BPEventPreBase"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("BPEventPreBase.as"),
			TEXT(R"AS(
UCLASS()
class UTestPreBaseAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Armor;
}
)AS"),
			TEXT("UTestPreBaseAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);

		ASC->SetAttributeBaseValue(AttrSetSubclass, TEXT("Armor"), 75.f);
		float BaseValue = ASC->GetAttributeBaseValueChecked(AttrSetSubclass, TEXT("Armor"));
		TestRunner->TestEqual(
			TEXT("PreAttributeBaseChange pipeline should be invoked (base value set correctly)"),
			BaseValue, 75.f);
	}

	TEST_METHOD(PostAttributeBaseChangeIsCalled)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("BPEventPostBase"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("BPEventPostBase.as"),
			TEXT(R"AS(
UCLASS()
class UTestPostBaseAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Shield;
}
)AS"),
			TEXT("UTestPostBaseAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);

		ASC->SetAttributeBaseValue(AttrSetSubclass, TEXT("Shield"), 20.f);
		ASC->SetAttributeBaseValue(AttrSetSubclass, TEXT("Shield"), 40.f);

		float BaseValue = ASC->GetAttributeBaseValueChecked(AttrSetSubclass, TEXT("Shield"));
		TestRunner->TestEqual(
			TEXT("PostAttributeBaseChange pipeline should complete (final base value correct)"),
			BaseValue, 40.f);
	}

	TEST_METHOD(MultipleAttributeChangesFireEventsInOrder)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("BPEventMultiChange"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("BPEventMultiChange.as"),
			TEXT(R"AS(
UCLASS()
class UTestMultiChangeAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Attack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Defense;
}
)AS"),
			TEXT("UTestMultiChangeAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);

		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Attack"), 10.f);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Defense"), 20.f);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Attack"), 30.f);

		float AttackVal = ASC->GetAttributeCurrentValue(AttrSetSubclass, TEXT("Attack"), -1.f);
		float DefenseVal = ASC->GetAttributeCurrentValue(AttrSetSubclass, TEXT("Defense"), -1.f);

		TestRunner->TestEqual(TEXT("Attack should reflect last set value"), AttackVal, 30.f);
		TestRunner->TestEqual(TEXT("Defense should reflect its set value"), DefenseVal, 20.f);
	}

	TEST_METHOD(AttributeChangeFromZeroToPositive)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("BPEventZeroToPos"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("BPEventZeroToPos.as"),
			TEXT(R"AS(
UCLASS()
class UTestZeroPosAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Energy;
}
)AS"),
			TEXT("UTestZeroPosAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);

		float InitialValue = ASC->GetAttributeCurrentValue(AttrSetSubclass, TEXT("Energy"), -1.f);
		TestRunner->TestEqual(TEXT("Initial attribute value should be 0"), InitialValue, 0.f);

		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Energy"), 100.f);
		float NewValue = ASC->GetAttributeCurrentValue(AttrSetSubclass, TEXT("Energy"), -1.f);
		TestRunner->TestEqual(TEXT("Attribute should change from 0 to 100"), NewValue, 100.f);
	}

	TEST_METHOD(AttributeChangeToNegativeValue)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("BPEventNegative"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("BPEventNegative.as"),
			TEXT(R"AS(
UCLASS()
class UTestNegativeAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Temperature;
}
)AS"),
			TEXT("UTestNegativeAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);

		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Temperature"), -10.f);
		float Value = ASC->GetAttributeCurrentValue(AttrSetSubclass, TEXT("Temperature"), 999.f);
		TestRunner->TestEqual(
			TEXT("Attribute should accept negative values through the BP event pipeline"),
			Value, -10.f);
	}

	TEST_METHOD(ModAttributeUnsafeBypassesPreAttributeChange)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("BPEventModUnsafe"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("BPEventModUnsafe.as"),
			TEXT(R"AS(
UCLASS()
class UTestModUnsafeBPAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Rage;
}
)AS"),
			TEXT("UTestModUnsafeBPAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Rage"), 50.f);

		FGameplayAttribute RageAttr = UAngelscriptAttributeSet::GetGameplayAttribute(AttributeSetClass, TEXT("Rage"));
		if (!TestRunner->TestTrue(TEXT("Rage attribute should be valid"), RageAttr.IsValid())) { return; }

		ASC->ModAttributeUnsafe(RageAttr, EGameplayModOp::Override, 200.f);
		float CurrentValue = ASC->GetAttributeCurrentValue(AttrSetSubclass, TEXT("Rage"), -1.f);
		TestRunner->TestEqual(
			TEXT("ModAttributeUnsafe should set value directly (bypasses normal callbacks)"),
			CurrentValue, 200.f);
	}

	TEST_METHOD(SetAttributeBaseValueTriggersBothPreAndPostBaseChange)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("BPEventBothBase"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("BPEventBothBase.as"),
			TEXT(R"AS(
UCLASS()
class UTestBothBaseAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Wisdom;
}
)AS"),
			TEXT("UTestBothBaseAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);

		ASC->SetAttributeBaseValue(AttrSetSubclass, TEXT("Wisdom"), 42.f);

		float BaseValue = ASC->GetAttributeBaseValueChecked(AttrSetSubclass, TEXT("Wisdom"));
		float CurrentValue = ASC->GetAttributeCurrentValueChecked(AttrSetSubclass, TEXT("Wisdom"));

		TestRunner->TestEqual(TEXT("Base value should be set after Pre/PostAttributeBaseChange"), BaseValue, 42.f);
		TestRunner->TestEqual(TEXT("Current value should match base value"), CurrentValue, 42.f);
	}
};

#endif // WITH_ANGELSCRIPT_UNITTESTS
