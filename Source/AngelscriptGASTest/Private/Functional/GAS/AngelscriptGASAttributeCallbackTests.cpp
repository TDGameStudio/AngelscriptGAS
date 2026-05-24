// AngelscriptGASAttributeCallbackTests.cpp
//
// CQTest coverage for attribute change callback registration and broadcasting:
//   - RegisterCallbackForAttribute / GetAndRegisterCallbackForAttribute (trampoline)
//   - RegisterAttributeChangedCallback / GetAndRegisterAttributeChangedCallback (UFunction)
//   - OnAttributeChanged delegate broadcasting with FAngelscriptModifiedAttribute
//   - Duplicate binding prevention (IsBoundToObject)
//   - Error paths: invalid attribute, null object, NAME_None function
//
// Automation IDs: Angelscript.GAS.Functional.AttributeCallbacks.*

#include "AngelscriptAbilitySystemComponent.h"
#include "AngelscriptAttributeSet.h"
#include "AngelscriptGASTestDelegateCapture.h"
#include "Components/ActorTestSpawner.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS


TEST_CLASS_WITH_FLAGS(FAngelscriptGASAttributeCallbackTests,
	"Angelscript.GAS.Functional.AttributeCallbacks",
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
	// pilot for rationale (each TEST_METHOD uses unique module + class names and
	// its own ON_SCOPE_EXIT { DiscardModule(...) } so the shared engine stays
	// clean across methods).
	BEFORE_ALL()
	{
		ASTEST_CREATE_ENGINE();
	}

	AFTER_ALL()
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		ASTEST_RESET_ENGINE(Engine);
	}

	TEST_METHOD(RegisterCallbackForAttributeBindsTrampoline)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrCallbackTrampoline"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrCallbackTrampoline.as"),
			TEXT(R"AS(
UCLASS()
class UTestTrampolineAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Stamina;
}
)AS"),
			TEXT("UTestTrampolineAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Stamina"), 50.f);

		ASC->RegisterCallbackForAttribute(AttrSetSubclass, TEXT("Stamina"));

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnAttributeChanged.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAttributeChanged);

		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Stamina"), 75.f);
		TestRunner->TestTrue(TEXT("OnAttributeChanged should fire after RegisterCallbackForAttribute"), Capture->bFired);

		ASC->OnAttributeChanged.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAttributeChanged);
	}

	TEST_METHOD(GetAndRegisterCallbackForAttributeReturnsCurrentValue)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrCallbackGetReg"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrCallbackGetReg.as"),
			TEXT(R"AS(
UCLASS()
class UTestGetRegAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Mana;
}
)AS"),
			TEXT("UTestGetRegAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Mana"), 42.f);

		float OutValue = 0.f;
		ASC->GetAndRegisterCallbackForAttribute(AttrSetSubclass, TEXT("Mana"), OutValue);
		TestRunner->TestEqual(
			TEXT("GetAndRegisterCallbackForAttribute should return the current attribute value"),
			OutValue, 42.f);
	}

	TEST_METHOD(OnAttributeChangedTrampolineBroadcastsCorrectData)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrCallbackData"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrCallbackData.as"),
			TEXT(R"AS(
UCLASS()
class UTestCallbackDataAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Energy;
}
)AS"),
			TEXT("UTestCallbackDataAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Energy"), 20.f);

		ASC->RegisterCallbackForAttribute(AttrSetSubclass, TEXT("Energy"));

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnAttributeChanged.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAttributeChanged);

		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Energy"), 35.f);

		TestRunner->TestEqual(TEXT("FAngelscriptModifiedAttribute.Name should be the attribute name"),
			Capture->CapturedAttributeChange.Name, FName(TEXT("Energy")));
		TestRunner->TestEqual(TEXT("FAngelscriptModifiedAttribute.OldValue should be the previous value"),
			Capture->CapturedAttributeChange.OldValue, 20.f);
		TestRunner->TestEqual(TEXT("FAngelscriptModifiedAttribute.NewValue should be the new value"),
			Capture->CapturedAttributeChange.NewValue, 35.f);

		ASC->OnAttributeChanged.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAttributeChanged);
	}

	TEST_METHOD(RegisterCallbackForAttributeNoDuplicateBinding)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrCallbackNoDup"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrCallbackNoDup.as"),
			TEXT(R"AS(
UCLASS()
class UTestNoDupAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Armor;
}
)AS"),
			TEXT("UTestNoDupAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Armor"), 10.f);

		ASC->RegisterCallbackForAttribute(AttrSetSubclass, TEXT("Armor"));
		ASC->RegisterCallbackForAttribute(AttrSetSubclass, TEXT("Armor"));

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnAttributeChanged.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAttributeChanged);

		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Armor"), 25.f);
		TestRunner->TestEqual(
			TEXT("Duplicate RegisterCallbackForAttribute should not cause multiple broadcasts"),
			Capture->FireCount, 1);

		ASC->OnAttributeChanged.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAttributeChanged);
	}

	TEST_METHOD(GetAndRegisterAttributeChangedCallbackReturnsCurrentValue)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrCallbackGetRegChanged"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrCallbackGetRegChanged.as"),
			TEXT(R"AS(
UCLASS()
class UTestGetRegChangedAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Spirit;
}
)AS"),
			TEXT("UTestGetRegChangedAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Spirit"), 88.f);

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		float OutValue = 0.f;
		ASC->GetAndRegisterAttributeChangedCallback(
			AttrSetSubclass,
			TEXT("Spirit"),
			Capture,
			GET_FUNCTION_NAME_CHECKED(UAngelscriptGASTestDelegateCapture, HandleGameplayAttributeChanged),
			OutValue);

		TestRunner->TestEqual(
			TEXT("GetAndRegisterAttributeChangedCallback should return the current attribute value"),
			OutValue, 88.f);

		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Spirit"), 99.f);
		TestRunner->TestTrue(
			TEXT("GetAndRegisterAttributeChangedCallback should bind the provided UFunction"),
			Capture->bFired);
	}

	TEST_METHOD(RegisterAttributeChangedCallbackWithInvalidAttributeDoesNotCrash)
	{
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		ASC->RegisterAttributeChangedCallback(nullptr, TEXT("NonExistent"), ASC, FName(TEXT("SomeFunc")));
		TestRunner->TestTrue(TEXT("RegisterAttributeChangedCallback with invalid attribute should not crash"), true);
	}

	TEST_METHOD(RegisterAttributeChangedCallbackWithNullObjectDoesNotCrash)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrCallbackNullObj"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrCallbackNullObj.as"),
			TEXT(R"AS(
UCLASS()
class UTestNullObjAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Willpower;
}
)AS"),
			TEXT("UTestNullObjAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);

		ASC->RegisterAttributeChangedCallback(AttrSetSubclass, TEXT("Willpower"), nullptr, FName(TEXT("SomeFunc")));
		TestRunner->TestTrue(TEXT("RegisterAttributeChangedCallback with null object should not crash"), true);
	}

	TEST_METHOD(RegisterAttributeChangedCallbackWithNoneNameDoesNotCrash)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrCallbackNoneName"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrCallbackNoneName.as"),
			TEXT(R"AS(
UCLASS()
class UTestNoneNameAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Resolve;
}
)AS"),
			TEXT("UTestNoneNameAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);

		ASC->RegisterAttributeChangedCallback(AttrSetSubclass, TEXT("Resolve"), ASC, NAME_None);
		TestRunner->TestTrue(TEXT("RegisterAttributeChangedCallback with NAME_None should not crash"), true);
	}

	TEST_METHOD(MultipleAttributesEachGetOwnCallback)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrCallbackMulti"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrCallbackMulti.as"),
			TEXT(R"AS(
UCLASS()
class UTestMultiCallbackAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Shield;
}
)AS"),
			TEXT("UTestMultiCallbackAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Health"), 100.f);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Shield"), 50.f);

		ASC->RegisterCallbackForAttribute(AttrSetSubclass, TEXT("Health"));
		ASC->RegisterCallbackForAttribute(AttrSetSubclass, TEXT("Shield"));

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnAttributeChanged.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAttributeChanged);

		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Health"), 80.f);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Shield"), 30.f);

		TestRunner->TestEqual(TEXT("Should fire callback for each attribute change"), Capture->AttributeChangedNames.Num(), 2);
		TestRunner->TestTrue(TEXT("First callback should be for Health"),
			Capture->AttributeChangedNames.Num() > 0 && Capture->AttributeChangedNames[0] == FName(TEXT("Health")));
		TestRunner->TestTrue(TEXT("Second callback should be for Shield"),
			Capture->AttributeChangedNames.Num() > 1 && Capture->AttributeChangedNames[1] == FName(TEXT("Shield")));

		ASC->OnAttributeChanged.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAttributeChanged);
	}

	TEST_METHOD(CallbackFiredWhenBaseValueIsApplied)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrCallbackUnchanged"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrCallbackUnchanged.as"),
			TEXT(R"AS(
UCLASS()
class UTestUnchangedAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Focus;
}
)AS"),
			TEXT("UTestUnchangedAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Focus"), 60.f);

		ASC->RegisterCallbackForAttribute(AttrSetSubclass, TEXT("Focus"));

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnAttributeChanged.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAttributeChanged);

		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Focus"), 60.f);
		TestRunner->TestEqual(
			TEXT("OnAttributeChanged should fire when the base value setter applies an attribute value"),
			Capture->FireCount, 1);
		TestRunner->TestEqual(
			TEXT("The same-value base setter should report matching old and new values"),
			Capture->CapturedAttributeChange.OldValue, Capture->CapturedAttributeChange.NewValue);

		ASC->OnAttributeChanged.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAttributeChanged);
	}

	TEST_METHOD(GetAndRegisterCallbackForAttributeAlsoBindsTrampoline)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrCallbackGetRegBind"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrCallbackGetRegBind.as"),
			TEXT(R"AS(
UCLASS()
class UTestGetRegBindAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Agility;
}
)AS"),
			TEXT("UTestGetRegBindAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Agility"), 10.f);

		float OutValue = 0.f;
		ASC->GetAndRegisterCallbackForAttribute(AttrSetSubclass, TEXT("Agility"), OutValue);

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnAttributeChanged.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAttributeChanged);

		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Agility"), 20.f);
		TestRunner->TestTrue(
			TEXT("GetAndRegisterCallbackForAttribute should also bind the trampoline so OnAttributeChanged fires"),
			Capture->bFired);

		ASC->OnAttributeChanged.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAttributeChanged);
	}

	TEST_METHOD(RegisterCallbackForDifferentAttributesIndependent)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrCallbackIndep"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrCallbackIndep.as"),
			TEXT(R"AS(
UCLASS()
class UTestIndepAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Strength;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Dexterity;
}
)AS"),
			TEXT("UTestIndepAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Strength"), 10.f);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Dexterity"), 20.f);

		ASC->RegisterCallbackForAttribute(AttrSetSubclass, TEXT("Strength"));

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnAttributeChanged.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAttributeChanged);

		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Dexterity"), 30.f);
		TestRunner->TestEqual(
			TEXT("Changing an attribute without a registered callback should not fire OnAttributeChanged"),
			Capture->AttributeChangedNames.Num(), 0);

		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Strength"), 15.f);
		TestRunner->TestEqual(
			TEXT("Changing the registered attribute should fire OnAttributeChanged"),
			Capture->AttributeChangedNames.Num(), 1);

		ASC->OnAttributeChanged.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAttributeChanged);
	}

	TEST_METHOD(OnAttributeChangedNotFiredWithoutRegistration)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrCallbackNoReg"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrCallbackNoReg.as"),
			TEXT(R"AS(
UCLASS()
class UTestNoRegAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Luck;
}
)AS"),
			TEXT("UTestNoRegAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Luck"), 5.f);

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnAttributeChanged.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAttributeChanged);

		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Luck"), 10.f);
		TestRunner->TestEqual(
			TEXT("OnAttributeChanged should not fire if RegisterCallbackForAttribute was never called"),
			Capture->FireCount, 0);

		ASC->OnAttributeChanged.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAttributeChanged);
	}

	TEST_METHOD(RegisterAttributeChangedCallbackWithValidParamsDoesNotCrash)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrCallbackValidParams"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrCallbackValidParams.as"),
			TEXT(R"AS(
UCLASS()
class UTestValidParamsAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Charisma;
}
)AS"),
			TEXT("UTestValidParamsAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC();

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);

		ASC->RegisterAttributeChangedCallback(AttrSetSubclass, TEXT("Charisma"), ASC, FName(TEXT("NonExistentFunc")));
		TestRunner->TestTrue(
			TEXT("RegisterAttributeChangedCallback with valid class/attribute/object but non-existent function should not crash"),
			true);
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
