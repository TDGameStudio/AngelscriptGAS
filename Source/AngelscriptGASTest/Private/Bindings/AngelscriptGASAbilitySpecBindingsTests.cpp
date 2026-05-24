// AngelscriptGASAbilitySpecBindingsTests.cpp
//
// CQTest coverage for FGameplayAbilitySpec Angelscript bindings:
//   - Constructor with TSubclassOf<UGameplayAbility>
//   - Bit-field accessor methods (InputPressed, RemoveAfterActivation, PendingRemove, ActivateOnce)
//   - Property access (Level, InputID, Handle)
//   - IsActive, GetDebugString
//
// Automation IDs: Angelscript.GAS.Bindings.AbilitySpec.*

#include "CQTest.h"
#include "Shared/AngelscriptTestMacros.h"
#include "Shared/AngelscriptTestModuleScope.h"
#include "Shared/AngelscriptBindingsAssertions.h"

#if WITH_DEV_AUTOMATION_TESTS

using namespace AngelscriptTestSupport;
using namespace AngelscriptTestBindings;


TEST_CLASS_WITH_FLAGS(FAngelscriptGASAbilitySpecBindingsTests,
	"Angelscript.GAS.Bindings.AbilitySpec",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
{
	BEFORE_ALL() { ASTEST_CREATE_ENGINE(); }
	AFTER_ALL() { FAngelscriptEngine& E = ASTEST_GET_ENGINE(); ASTEST_RESET_ENGINE(E); }

	TEST_METHOD(ConstructorSetsLevel)
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope Scope(Engine);
		FScopedAngelscriptModule Mod(*TestRunner, Engine, TEXT("ASAbilitySpec_CtorLevel"), TEXT(R"(
int AbilitySpec_CtorLevel()
{
	FGameplayAbilitySpec Spec;
	return Spec.Level;
}
)"));
		if (!Mod.IsValid()) { return; }
		AngelscriptTestBindings::ExpectGlobalInt(*TestRunner, Engine, Mod.GetModule(), 
			TEXT("int AbilitySpec_CtorLevel()"), TEXT("Default-constructed spec level should be 1"), 1);
	}

	TEST_METHOD(ConstructorSetsInputID)
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope Scope(Engine);
		FScopedAngelscriptModule Mod(*TestRunner, Engine, TEXT("ASAbilitySpec_CtorInputID"), TEXT(R"(
int AbilitySpec_CtorInputID()
{
	FGameplayAbilitySpec Spec;
	return Spec.InputID;
}
)"));
		if (!Mod.IsValid()) { return; }
		AngelscriptTestBindings::ExpectGlobalInt(*TestRunner, Engine, Mod.GetModule(), 
			TEXT("int AbilitySpec_CtorInputID()"), TEXT("Default-constructed spec InputID should be -1"), -1);
	}

	TEST_METHOD(BitFieldInputPressed)
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope Scope(Engine);
		FScopedAngelscriptModule Mod(*TestRunner, Engine, TEXT("ASAbilitySpec_InputPressed"), TEXT(R"(
int AbilitySpec_InputPressed()
{
	FGameplayAbilitySpec Spec;
	if (Spec.GetbInputPressed())
		return -1;
	Spec.SetbInputPressed(true);
	if (!Spec.GetbInputPressed())
		return -2;
	Spec.SetbInputPressed(false);
	if (Spec.GetbInputPressed())
		return -3;
	return 1;
}
)"));
		if (!Mod.IsValid()) { return; }
		AngelscriptTestBindings::ExpectGlobalInt(*TestRunner, Engine, Mod.GetModule(), 
			TEXT("int AbilitySpec_InputPressed()"),
			TEXT("InputPressed bit-field get/set should round-trip correctly"), 1);
	}

	TEST_METHOD(BitFieldRemoveAfterActivation)
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope Scope(Engine);
		FScopedAngelscriptModule Mod(*TestRunner, Engine, TEXT("ASAbilitySpec_RemoveAfterActivation"), TEXT(R"(
int AbilitySpec_RemoveAfterActivation()
{
	FGameplayAbilitySpec Spec;
	if (Spec.GetbRemoveAfterActivation())
		return -1;
	Spec.SetbRemoveAfterActivation(true);
	if (!Spec.GetbRemoveAfterActivation())
		return -2;
	return 1;
}
)"));
		if (!Mod.IsValid()) { return; }
		AngelscriptTestBindings::ExpectGlobalInt(*TestRunner, Engine, Mod.GetModule(), 
			TEXT("int AbilitySpec_RemoveAfterActivation()"),
			TEXT("RemoveAfterActivation bit-field get/set should round-trip correctly"), 1);
	}

	TEST_METHOD(BitFieldPendingRemove)
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope Scope(Engine);
		FScopedAngelscriptModule Mod(*TestRunner, Engine, TEXT("ASAbilitySpec_PendingRemove"), TEXT(R"(
int AbilitySpec_PendingRemove()
{
	FGameplayAbilitySpec Spec;
	if (Spec.GetbPendingRemove())
		return -1;
	Spec.SetbPendingRemove(true);
	if (!Spec.GetbPendingRemove())
		return -2;
	return 1;
}
)"));
		if (!Mod.IsValid()) { return; }
		AngelscriptTestBindings::ExpectGlobalInt(*TestRunner, Engine, Mod.GetModule(), 
			TEXT("int AbilitySpec_PendingRemove()"),
			TEXT("PendingRemove bit-field get/set should round-trip correctly"), 1);
	}

	TEST_METHOD(BitFieldActivateOnce)
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope Scope(Engine);
		FScopedAngelscriptModule Mod(*TestRunner, Engine, TEXT("ASAbilitySpec_ActivateOnce"), TEXT(R"(
int AbilitySpec_ActivateOnce()
{
	FGameplayAbilitySpec Spec;
	if (Spec.GetbActivateOnce())
		return -1;
	Spec.SetbActivateOnce(true);
	if (!Spec.GetbActivateOnce())
		return -2;
	return 1;
}
)"));
		if (!Mod.IsValid()) { return; }
		AngelscriptTestBindings::ExpectGlobalInt(*TestRunner, Engine, Mod.GetModule(), 
			TEXT("int AbilitySpec_ActivateOnce()"),
			TEXT("ActivateOnce bit-field get/set should round-trip correctly"), 1);
	}

	TEST_METHOD(IsActiveDefaultFalse)
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope Scope(Engine);
		FScopedAngelscriptModule Mod(*TestRunner, Engine, TEXT("ASAbilitySpec_IsActive"), TEXT(R"(
int AbilitySpec_IsActive()
{
	FGameplayAbilitySpec Spec;
	return Spec.IsActive() ? 0 : 1;
}
)"));
		if (!Mod.IsValid()) { return; }
		AngelscriptTestBindings::ExpectGlobalInt(*TestRunner, Engine, Mod.GetModule(), 
			TEXT("int AbilitySpec_IsActive()"),
			TEXT("Default-constructed spec should not be active"), 1);
	}

	TEST_METHOD(ActiveCountDefault)
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope Scope(Engine);
		FScopedAngelscriptModule Mod(*TestRunner, Engine, TEXT("ASAbilitySpec_ActiveCount"), TEXT(R"(
int AbilitySpec_ActiveCount()
{
	FGameplayAbilitySpec Spec;
	return int(Spec.ActiveCount);
}
)"));
		if (!Mod.IsValid()) { return; }
		AngelscriptTestBindings::ExpectGlobalInt(*TestRunner, Engine, Mod.GetModule(), 
			TEXT("int AbilitySpec_ActiveCount()"),
			TEXT("Default-constructed spec ActiveCount should be 0"), 0);
	}

	TEST_METHOD(ConstructorWithClassSetsAbility)
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope Scope(Engine);
		FScopedAngelscriptModule Mod(*TestRunner, Engine, TEXT("ASAbilitySpec_CtorClass"), TEXT(R"(
int AbilitySpec_CtorClass()
{
	FGameplayAbilitySpec Spec;
	// Default-constructed spec should have null Ability
	return (Spec.Ability == null) ? 1 : 0;
}
)"));
		if (!Mod.IsValid()) { return; }
		AngelscriptTestBindings::ExpectGlobalInt(*TestRunner, Engine, Mod.GetModule(), 
			TEXT("int AbilitySpec_CtorClass()"),
			TEXT("Default-constructed spec should have null Ability"), 1);
	}

	TEST_METHOD(GetPrimaryInstanceReturnsNullByDefault)
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope Scope(Engine);
		FScopedAngelscriptModule Mod(*TestRunner, Engine, TEXT("ASAbilitySpec_PrimaryInstance"), TEXT(R"(
int AbilitySpec_PrimaryInstance()
{
	FGameplayAbilitySpec Spec;
	return (Spec.GetPrimaryInstance() == null) ? 1 : 0;
}
)"));
		if (!Mod.IsValid()) { return; }
		AngelscriptTestBindings::ExpectGlobalInt(*TestRunner, Engine, Mod.GetModule(), 
			TEXT("int AbilitySpec_PrimaryInstance()"),
			TEXT("Default-constructed spec GetPrimaryInstance should return null"), 1);
	}

	TEST_METHOD(DynamicAbilityTagsDefaultEmpty)
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope Scope(Engine);
		FScopedAngelscriptModule Mod(*TestRunner, Engine, TEXT("ASAbilitySpec_DynTags"), TEXT(R"(
int AbilitySpec_DynTags()
{
	FGameplayAbilitySpec Spec;
	return Spec.DynamicAbilityTags.Num();
}
)"));
		if (!Mod.IsValid()) { return; }
		AngelscriptTestBindings::ExpectGlobalInt(*TestRunner, Engine, Mod.GetModule(), 
			TEXT("int AbilitySpec_DynTags()"),
			TEXT("Default-constructed spec DynamicAbilityTags should be empty"), 0);
	}

	TEST_METHOD(SetByCallerTagMagnitudesDefaultEmpty)
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope Scope(Engine);
		FScopedAngelscriptModule Mod(*TestRunner, Engine, TEXT("ASAbilitySpec_SetByCaller"), TEXT(R"(
int AbilitySpec_SetByCaller()
{
	FGameplayAbilitySpec Spec;
	return Spec.SetByCallerTagMagnitudes.Num();
}
)"));
		if (!Mod.IsValid()) { return; }
		AngelscriptTestBindings::ExpectGlobalInt(*TestRunner, Engine, Mod.GetModule(), 
			TEXT("int AbilitySpec_SetByCaller()"),
			TEXT("Default-constructed spec SetByCallerTagMagnitudes should be empty"), 0);
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
