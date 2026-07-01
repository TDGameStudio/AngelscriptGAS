// AngelscriptGASAttributeChangedDataMixinTests.cpp
//
// CQTest coverage for FAngelscriptAttributeChangedData and UAngelscriptAttributeChangedDataMixinLibrary:
//   - Default construction values
//   - GetNewValue / GetOldValue / GetGameplayAttribute accessors
//   - GetEffectSpec / GetGameplayModifierEvaluatedData / GetTargetAbilitySystemComponent null-safety
//   - Copy constructor behavior
//
// Automation IDs: Angelscript.GAS.Bindings.AttributeChangedDataMixin.*

#include "AngelscriptAbilitySystemComponent.h"
#include "AngelscriptAttributeSet.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "Misc/ScopeExit.h"

#if WITH_ANGELSCRIPT_UNITTESTS


TEST_CLASS_WITH_FLAGS(FAngelscriptGASAttributeChangedDataMixinTests,
	"Angelscript.GAS.Bindings.AttributeChangedDataMixin",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
{
	TEST_METHOD(DefaultConstructedHasZeroValues)
	{
		FAngelscriptAttributeChangedData Data;
		float NewValue = UAngelscriptAttributeChangedDataMixinLibrary::GetNewValue(Data);
		float OldValue = UAngelscriptAttributeChangedDataMixinLibrary::GetOldValue(Data);

		TestRunner->TestEqual(TEXT("Default NewValue should be 0"), NewValue, 0.f);
		TestRunner->TestEqual(TEXT("Default OldValue should be 0"), OldValue, 0.f);
	}

	TEST_METHOD(GetNewValueReturnsCorrectValue)
	{
		FAngelscriptAttributeChangedData Data;
		Data.WrappedData.NewValue = 42.5f;

		float NewValue = UAngelscriptAttributeChangedDataMixinLibrary::GetNewValue(Data);
		TestRunner->TestEqual(TEXT("GetNewValue should return the set NewValue"), NewValue, 42.5f);
	}

	TEST_METHOD(GetOldValueReturnsCorrectValue)
	{
		FAngelscriptAttributeChangedData Data;
		Data.WrappedData.OldValue = 17.3f;

		float OldValue = UAngelscriptAttributeChangedDataMixinLibrary::GetOldValue(Data);
		TestRunner->TestEqual(TEXT("GetOldValue should return the set OldValue"), OldValue, 17.3f);
	}

	TEST_METHOD(GetGameplayAttributeReturnsAttribute)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_CREATE_ENGINE_FULL();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("MixinGetAttr"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("MixinGetAttr.as"),
			TEXT(R"AS(
UCLASS()
class UTestMixinAttrSet : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Luck;
}
)AS"),
			TEXT("UTestMixinAttrSet"));
		if (AttributeSetClass == nullptr) { return; }

		FGameplayAttribute Attr = UAngelscriptAttributeSet::GetGameplayAttribute(AttributeSetClass, TEXT("Luck"));
		if (!TestRunner->TestTrue(TEXT("Attribute should be valid"), Attr.IsValid())) { return; }

		FAngelscriptAttributeChangedData Data;
		Data.WrappedData.Attribute = Attr;

		const FGameplayAttribute& Retrieved = UAngelscriptAttributeChangedDataMixinLibrary::GetGameplayAttribute(Data);
		TestRunner->TestTrue(
			TEXT("GetGameplayAttribute should return the attribute set on WrappedData"),
			Retrieved.IsValid());
		TestRunner->TestEqual(
			TEXT("GetGameplayAttribute should return the same attribute"),
			Retrieved.GetAttributeSetClass(), Attr.GetAttributeSetClass());
	}

	TEST_METHOD(GetEffectSpecReturnsFalseWhenNoEffect)
	{
		FAngelscriptAttributeChangedData Data;
		bool bIsValid = true;
		UAngelscriptAttributeChangedDataMixinLibrary::GetEffectSpec(Data, bIsValid);
		TestRunner->TestFalse(
			TEXT("GetEffectSpec should set bIsValid to false when no GE mod data exists"),
			bIsValid);
	}

	TEST_METHOD(GetGameplayModifierEvaluatedDataReturnsFalseWhenNoEffect)
	{
		FAngelscriptAttributeChangedData Data;
		bool bIsValid = true;
		UAngelscriptAttributeChangedDataMixinLibrary::GetGameplayModifierEvaluatedData(Data, bIsValid);
		TestRunner->TestFalse(
			TEXT("GetGameplayModifierEvaluatedData should set bIsValid to false when no GE mod data exists"),
			bIsValid);
	}

	TEST_METHOD(GetTargetAbilitySystemComponentReturnsNullWhenNoEffect)
	{
		FAngelscriptAttributeChangedData Data;
		UAbilitySystemComponent* Result = UAngelscriptAttributeChangedDataMixinLibrary::GetTargetAbilitySystemComponent(Data);
		TestRunner->TestNull(
			TEXT("GetTargetAbilitySystemComponent should return null when no GE mod data exists"),
			Result);
	}

	TEST_METHOD(CopyConstructorPreservesValues)
	{
		FAngelscriptAttributeChangedData Original;
		Original.WrappedData.OldValue = 10.f;
		Original.WrappedData.NewValue = 20.f;

		FAngelscriptAttributeChangedData Copy(Original);

		float CopyOld = UAngelscriptAttributeChangedDataMixinLibrary::GetOldValue(Copy);
		float CopyNew = UAngelscriptAttributeChangedDataMixinLibrary::GetNewValue(Copy);

		TestRunner->TestEqual(TEXT("Copy constructor should preserve OldValue"), CopyOld, 10.f);
		TestRunner->TestEqual(TEXT("Copy constructor should preserve NewValue"), CopyNew, 20.f);
	}
};

#endif // WITH_ANGELSCRIPT_UNITTESTS
