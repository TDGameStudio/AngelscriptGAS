// AngelscriptGASAttributeSetUtilityTests.cpp
//
// CQTest coverage for UAngelscriptAttributeSet static utility functions:
//   - GetGameplayAttribute
//   - TryGetGameplayAttribute
//   - CompareGameplayAttributes
//
// Automation IDs: Angelscript.GAS.Functional.AttributeSetUtility.*

#include "AngelscriptAttributeSet.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS

using namespace AngelscriptTestSupport;

TEST_CLASS_WITH_FLAGS(FAngelscriptGASAttributeSetUtilityTests,
	"Angelscript.GAS.Functional.AttributeSetUtility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
{
	TEST_METHOD(GetGameplayAttributeFromScriptSubclass)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_CREATE_ENGINE_FULL();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrUtilGetAttr"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner,
			Engine,
			ModuleName,
			TEXT("AttrUtilGetAttr.as"),
			TEXT(R"AS(
UCLASS()
class UTestUtilAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Strength;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Agility;
}
)AS"),
			TEXT("UTestUtilAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		FGameplayAttribute StrengthAttr = UAngelscriptAttributeSet::GetGameplayAttribute(
			AttributeSetClass, TEXT("Strength"));
		TestRunner->TestTrue(
			TEXT("GetGameplayAttribute should return a valid attribute for 'Strength'"),
			StrengthAttr.IsValid());

		FGameplayAttribute AgilityAttr = UAngelscriptAttributeSet::GetGameplayAttribute(
			AttributeSetClass, TEXT("Agility"));
		TestRunner->TestTrue(
			TEXT("GetGameplayAttribute should return a valid attribute for 'Agility'"),
			AgilityAttr.IsValid());
	}

	TEST_METHOD(TryGetGameplayAttributeSuccess)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_CREATE_ENGINE_FULL();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrUtilTryGet"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner,
			Engine,
			ModuleName,
			TEXT("AttrUtilTryGet.as"),
			TEXT(R"AS(
UCLASS()
class UTestTryGetAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Mana;
}
)AS"),
			TEXT("UTestTryGetAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		FGameplayAttribute OutAttr;
		bool bFound = UAngelscriptAttributeSet::TryGetGameplayAttribute(
			AttributeSetClass, TEXT("Mana"), OutAttr);
		TestRunner->TestTrue(TEXT("TryGetGameplayAttribute should return true for existing 'Mana'"), bFound);
		TestRunner->TestTrue(TEXT("TryGetGameplayAttribute out param should be valid"), OutAttr.IsValid());
	}

	TEST_METHOD(TryGetGameplayAttributeFailure)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_CREATE_ENGINE_FULL();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrUtilTryGetFail"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner,
			Engine,
			ModuleName,
			TEXT("AttrUtilTryGetFail.as"),
			TEXT(R"AS(
UCLASS()
class UTestTryGetFailAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Health;
}
)AS"),
			TEXT("UTestTryGetFailAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		FGameplayAttribute OutAttr;
		bool bFound = UAngelscriptAttributeSet::TryGetGameplayAttribute(
			AttributeSetClass, TEXT("NonExistentAttribute"), OutAttr);
		TestRunner->TestFalse(TEXT("TryGetGameplayAttribute should return false for non-existent attribute"), bFound);
		TestRunner->TestFalse(TEXT("TryGetGameplayAttribute out param should be invalid for non-existent attribute"), OutAttr.IsValid());
	}

	TEST_METHOD(CompareGameplayAttributesEqual)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_CREATE_ENGINE_FULL();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrUtilCompare"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner,
			Engine,
			ModuleName,
			TEXT("AttrUtilCompare.as"),
			TEXT(R"AS(
UCLASS()
class UTestCompareAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Speed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Armor;
}
)AS"),
			TEXT("UTestCompareAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		FGameplayAttribute SpeedA = UAngelscriptAttributeSet::GetGameplayAttribute(
			AttributeSetClass, TEXT("Speed"));
		FGameplayAttribute SpeedB = UAngelscriptAttributeSet::GetGameplayAttribute(
			AttributeSetClass, TEXT("Speed"));
		FGameplayAttribute ArmorAttr = UAngelscriptAttributeSet::GetGameplayAttribute(
			AttributeSetClass, TEXT("Armor"));

		TestRunner->TestTrue(
			TEXT("CompareGameplayAttributes should return true for same attribute"),
			UAngelscriptAttributeSet::CompareGameplayAttributes(SpeedA, SpeedB));
		TestRunner->TestFalse(
			TEXT("CompareGameplayAttributes should return false for different attributes"),
			UAngelscriptAttributeSet::CompareGameplayAttributes(SpeedA, ArmorAttr));
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
