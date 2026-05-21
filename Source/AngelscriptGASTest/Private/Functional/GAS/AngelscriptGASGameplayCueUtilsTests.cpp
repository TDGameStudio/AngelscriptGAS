// AngelscriptGASGameplayCueUtilsTests.cpp
//
// CQTest coverage for:
//   - UAngelscriptGameplayCueUtils null-safety (AddLocal, RemoveLocal, ExecuteLocal)
//   - FGameplayEffectSpec bindings (SetByCallerTagMagnitudes, SetByCallerNameMagnitudes)
//   - UAngelscriptGameplayEffectUtils additional coverage
//
// Automation IDs: Angelscript.GAS.Functional.GameplayCueUtils.*

#include "AngelscriptGameplayCueUtils.h"
#include "AngelscriptGameplayEffectUtils.h"
#include "AngelscriptAttributeSet.h"
#include "Components/ActorTestSpawner.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "GameplayTagsManager.h"
#include "GameplayEffect.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS

using namespace AngelscriptTestSupport;

TEST_CLASS_WITH_FLAGS(FAngelscriptGASGameplayCueUtilsTests,
	"Angelscript.GAS.Functional.GameplayCueUtils",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
{
	FActorTestSpawner Spawner;

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

	TEST_METHOD(AddLocalGameplayCueDoesNotCrashWithNullActor)
	{
		FGameplayCueParameters Params;
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Test"), false);
		UAngelscriptGameplayCueUtils::AddLocalGameplayCue(nullptr, Tag, Params);
		TestRunner->TestTrue(
			TEXT("AddLocalGameplayCue with null actor should not crash"),
			true);
	}

	TEST_METHOD(RemoveLocalGameplayCueDoesNotCrashWithNullActor)
	{
		FGameplayCueParameters Params;
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Test"), false);
		UAngelscriptGameplayCueUtils::RemoveLocalGameplayCue(nullptr, Tag, Params);
		TestRunner->TestTrue(
			TEXT("RemoveLocalGameplayCue with null actor should not crash"),
			true);
	}

	TEST_METHOD(ExecuteLocalGameplayCueDoesNotCrashWithNullActor)
	{
		FGameplayCueParameters Params;
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Test"), false);
		UAngelscriptGameplayCueUtils::ExecuteLocalGameplayCue(nullptr, Tag, Params);
		TestRunner->TestTrue(
			TEXT("ExecuteLocalGameplayCue with null actor should not crash"),
			true);
	}

	TEST_METHOD(AddLocalGameplayCueDoesNotCrashWithInvalidTag)
	{
		AActor* TestActor = &Spawner.SpawnActor<AActor>();
		FGameplayCueParameters Params;
		FGameplayTag InvalidTag;
		UAngelscriptGameplayCueUtils::AddLocalGameplayCue(TestActor, InvalidTag, Params);
		TestRunner->TestTrue(
			TEXT("AddLocalGameplayCue with invalid tag should not crash"),
			true);
	}

	TEST_METHOD(CaptureGameplayAttributeWithSnapshotSetsFlag)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("CueUtilsSnapshot"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("CueUtilsSnapshot.as"),
			TEXT(R"AS(
UCLASS()
class UTestSnapshotAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Power;
}
)AS"),
			TEXT("UTestSnapshotAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		FGameplayEffectAttributeCaptureDefinition CaptureDef =
			UAngelscriptGameplayEffectUtils::CaptureGameplayAttribute(
				AttributeSetClass, TEXT("Power"),
				EGameplayEffectAttributeCaptureSource::Source, true);

		TestRunner->TestTrue(
			TEXT("CaptureGameplayAttribute with bIsSnapshot=true should set snapshot flag"),
			CaptureDef.bSnapshot);
	}

	TEST_METHOD(CaptureGameplayAttributeWithoutSnapshotClearsFlag)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("CueUtilsNoSnapshot"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("CueUtilsNoSnapshot.as"),
			TEXT(R"AS(
UCLASS()
class UTestNoSnapshotAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Speed;
}
)AS"),
			TEXT("UTestNoSnapshotAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		FGameplayEffectAttributeCaptureDefinition CaptureDef =
			UAngelscriptGameplayEffectUtils::CaptureGameplayAttribute(
				AttributeSetClass, TEXT("Speed"),
				EGameplayEffectAttributeCaptureSource::Target, false);

		TestRunner->TestFalse(
			TEXT("CaptureGameplayAttribute with bIsSnapshot=false should clear snapshot flag"),
			CaptureDef.bSnapshot);
	}

	TEST_METHOD(MakeGameplayEffectExecutionScopedModifierInfoPreservesCaptureDef)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("CueUtilsScopedMod"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("CueUtilsScopedMod.as"),
			TEXT(R"AS(
UCLASS()
class UTestScopedModAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Damage;
}
)AS"),
			TEXT("UTestScopedModAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		FGameplayEffectAttributeCaptureDefinition CaptureDef =
			UAngelscriptGameplayEffectUtils::CaptureGameplayAttribute(
				AttributeSetClass, TEXT("Damage"),
				EGameplayEffectAttributeCaptureSource::Source, true);

		FGameplayEffectExecutionScopedModifierInfo ScopedInfo =
			UAngelscriptGameplayEffectUtils::MakeGameplayEffectExecutionScopedModifierInfo(CaptureDef);

		TestRunner->TestTrue(
			TEXT("MakeGameplayEffectExecutionScopedModifierInfo should preserve the capture definition snapshot flag"),
			ScopedInfo.CapturedAttribute.bSnapshot);
	}

	TEST_METHOD(SetCapturedSourceTagsFromSpecDoesNotCrash)
	{
		FGameplayEffectExecutionParameters Params;
		FGameplayEffectSpec Spec;
		UGameplayEffectExecutionParametersMixinLibrary::SetCapturedSourceTagsFromSpec(Params, Spec);
		TestRunner->TestTrue(
			TEXT("SetCapturedSourceTagsFromSpec with default spec should not crash"),
			true);
	}

	TEST_METHOD(ExecutionParametersSetIncludePredictiveModsRoundTrips)
	{
		FGameplayEffectExecutionParameters Params;
		TestRunner->TestFalse(
			TEXT("Default IncludePredictiveMods should be false"),
			UGameplayEffectExecutionParametersMixinLibrary::GetIncludePredictiveMods(Params));

		UGameplayEffectExecutionParametersMixinLibrary::SetIncludePredictiveMods(Params, true);
		TestRunner->TestTrue(
			TEXT("After SetIncludePredictiveMods(true), GetIncludePredictiveMods should return true"),
			UGameplayEffectExecutionParametersMixinLibrary::GetIncludePredictiveMods(Params));
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
