// AngelscriptGASGameplayEffectUtilsTests.cpp
//
// CQTest coverage for UAngelscriptGameplayEffectUtils and
// UGameplayEffectExecutionParametersMixinLibrary:
//   - CaptureGameplayAttribute
//   - MakeGameplayModifierEvaluationData
//   - MakeGameplayEffectExecutionScopedModifierInfo
//   - FGameplayEffectExecutionParameters mixin accessors
//
// Automation IDs: Angelscript.GAS.Functional.GameplayEffectUtils.*

#include "AngelscriptGameplayEffectUtils.h"
#include "AngelscriptAttributeSet.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS

using namespace AngelscriptTestSupport;

TEST_CLASS_WITH_FLAGS(FAngelscriptGASGameplayEffectUtilsTests,
	"Angelscript.GAS.Functional.GameplayEffectUtils",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
{
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

	TEST_METHOD(CaptureGameplayAttributeReturnsValidDefinition)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("EffUtilCapture"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner,
			Engine,
			ModuleName,
			TEXT("EffUtilCapture.as"),
			TEXT(R"AS(
UCLASS()
class UEffUtilCaptureAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Damage;
}
)AS"),
			TEXT("UEffUtilCaptureAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		FGameplayEffectAttributeCaptureDefinition CaptureDef =
			UAngelscriptGameplayEffectUtils::CaptureGameplayAttribute(
				AttributeSetClass,
				TEXT("Damage"),
				EGameplayEffectAttributeCaptureSource::Source,
				true);

		TestRunner->TestTrue(
			TEXT("CaptureGameplayAttribute should return a definition with a valid attribute"),
			CaptureDef.AttributeToCapture.IsValid());
		TestRunner->TestEqual(
			TEXT("CaptureGameplayAttribute should set the capture source correctly"),
			static_cast<int32>(CaptureDef.AttributeSource),
			static_cast<int32>(EGameplayEffectAttributeCaptureSource::Source));
		TestRunner->TestTrue(
			TEXT("CaptureGameplayAttribute should set bSnapshot correctly"),
			CaptureDef.bSnapshot);
	}

	TEST_METHOD(CaptureGameplayAttributeTargetNotSnapshot)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("EffUtilCaptureTarget"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner,
			Engine,
			ModuleName,
			TEXT("EffUtilCaptureTarget.as"),
			TEXT(R"AS(
UCLASS()
class UEffUtilCaptureTargetAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Defense;
}
)AS"),
			TEXT("UEffUtilCaptureTargetAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		FGameplayEffectAttributeCaptureDefinition CaptureDef =
			UAngelscriptGameplayEffectUtils::CaptureGameplayAttribute(
				AttributeSetClass,
				TEXT("Defense"),
				EGameplayEffectAttributeCaptureSource::Target,
				false);

		TestRunner->TestTrue(
			TEXT("CaptureGameplayAttribute (Target) should return a valid attribute"),
			CaptureDef.AttributeToCapture.IsValid());
		TestRunner->TestEqual(
			TEXT("CaptureGameplayAttribute should set Target source"),
			static_cast<int32>(CaptureDef.AttributeSource),
			static_cast<int32>(EGameplayEffectAttributeCaptureSource::Target));
		TestRunner->TestFalse(
			TEXT("CaptureGameplayAttribute should set bSnapshot=false"),
			CaptureDef.bSnapshot);
	}

	TEST_METHOD(MakeGameplayModifierEvaluationDataSetsFields)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("EffUtilModEval"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner,
			Engine,
			ModuleName,
			TEXT("EffUtilModEval.as"),
			TEXT(R"AS(
UCLASS()
class UEffUtilModEvalAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData AttackPower;
}
)AS"),
			TEXT("UEffUtilModEvalAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		FGameplayAttribute Attr = UAngelscriptAttributeSet::GetGameplayAttribute(
			AttributeSetClass, TEXT("AttackPower"));
		if (!TestRunner->TestTrue(TEXT("Attribute should be valid for MakeGameplayModifierEvaluationData test"), Attr.IsValid()))
		{
			return;
		}

		FGameplayModifierEvaluatedData EvalData =
			UAngelscriptGameplayEffectUtils::MakeGameplayModifierEvaluationData(
				Attr, EGameplayModOp::Additive, 25.f);

		TestRunner->TestTrue(
			TEXT("MakeGameplayModifierEvaluationData should set the attribute"),
			EvalData.Attribute.IsValid());
		TestRunner->TestEqual(
			TEXT("MakeGameplayModifierEvaluationData should set the mod op"),
			static_cast<int32>(EvalData.ModifierOp.GetValue()),
			static_cast<int32>(EGameplayModOp::Additive));
		TestRunner->TestEqual(
			TEXT("MakeGameplayModifierEvaluationData should set the magnitude"),
			EvalData.Magnitude, 25.f);
	}

	TEST_METHOD(ExecutionParametersIncludePredictiveModsDefault)
	{
		FGameplayEffectExecutionParameters Params;
		bool bDefault = UGameplayEffectExecutionParametersMixinLibrary::GetIncludePredictiveMods(Params);
		TestRunner->TestFalse(
			TEXT("Default FGameplayEffectExecutionParameters should have IncludePredictiveMods=false"),
			bDefault);

		UGameplayEffectExecutionParametersMixinLibrary::SetIncludePredictiveMods(Params, true);
		bool bAfterSet = UGameplayEffectExecutionParametersMixinLibrary::GetIncludePredictiveMods(Params);
		TestRunner->TestTrue(
			TEXT("SetIncludePredictiveMods(true) should be reflected by GetIncludePredictiveMods"),
			bAfterSet);
	}

	TEST_METHOD(ExecutionParametersTagFilterAccess)
	{
		FGameplayEffectExecutionParameters Params;

		FGameplayTagContainer& SourceFilter =
			UGameplayEffectExecutionParametersMixinLibrary::GetAppliedSourceTagFilter(Params);
		TestRunner->TestEqual(
			TEXT("Default source tag filter should be empty"),
			SourceFilter.Num(), 0);

		FGameplayTagContainer& TargetFilter =
			UGameplayEffectExecutionParametersMixinLibrary::GetAppliedTargetTagFilter(Params);
		TestRunner->TestEqual(
			TEXT("Default target tag filter should be empty"),
			TargetFilter.Num(), 0);
	}

	TEST_METHOD(ExecutionParametersIgnoreHandlesAccess)
	{
		FGameplayEffectExecutionParameters Params;

		TArray<FActiveGameplayEffectHandle>& Handles =
			UGameplayEffectExecutionParametersMixinLibrary::GetIgnoreHandles(Params);
		TestRunner->TestEqual(
			TEXT("Default ignore handles array should be empty"),
			Handles.Num(), 0);
	}

	TEST_METHOD(MakeExecutionScopedModifierInfoSetsCaptureDef)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("EffUtilScopedMod"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner,
			Engine,
			ModuleName,
			TEXT("EffUtilScopedMod.as"),
			TEXT(R"AS(
UCLASS()
class UEffUtilScopedModAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData CritChance;
}
)AS"),
			TEXT("UEffUtilScopedModAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		FGameplayEffectAttributeCaptureDefinition CaptureDef =
			UAngelscriptGameplayEffectUtils::CaptureGameplayAttribute(
				AttributeSetClass, TEXT("CritChance"),
				EGameplayEffectAttributeCaptureSource::Source, true);

		FGameplayEffectExecutionScopedModifierInfo ScopedInfo =
			UAngelscriptGameplayEffectUtils::MakeGameplayEffectExecutionScopedModifierInfo(CaptureDef);

		TestRunner->TestTrue(
			TEXT("MakeGameplayEffectExecutionScopedModifierInfo should preserve the capture definition attribute"),
			ScopedInfo.CapturedAttribute.AttributeToCapture.IsValid());
		TestRunner->TestEqual(
			TEXT("MakeGameplayEffectExecutionScopedModifierInfo should preserve the capture source"),
			static_cast<int32>(ScopedInfo.CapturedAttribute.AttributeSource),
			static_cast<int32>(EGameplayEffectAttributeCaptureSource::Source));
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
