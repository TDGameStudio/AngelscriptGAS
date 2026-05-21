// AngelscriptGASAttributeSetOverrideTests.cpp
//
// CQTest coverage for UAngelscriptAttributeSet:
//   - BP_GetOwningActor / BP_GetOwningAbilitySystemComponent / BP_GetActorInfo
//   - TrySetAttributeBaseValue / TryGetAttributeCurrentValue / TryGetAttributeBaseValue (from set)
//   - PostInitProperties sets AttributeName on FAngelscriptGameplayAttributeData
//   - ReplicatedAttributeBlackList filtering
//   - PreAttributeBaseChange / PostAttributeBaseChange pipeline wiring
//
// Automation IDs: Angelscript.GAS.Functional.AttributeSetOverrides.*

#include "AngelscriptAbilitySystemComponent.h"
#include "AngelscriptAttributeSet.h"
#include "Components/ActorTestSpawner.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS

using namespace AngelscriptTestSupport;

TEST_CLASS_WITH_FLAGS(FAngelscriptGASAttributeSetOverrideTests,
	"Angelscript.GAS.Functional.AttributeSetOverrides",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
{
	FActorTestSpawner Spawner;

	AActor* SpawnTestActor()
	{
		return &Spawner.SpawnActor<AActor>();
	}

	UAngelscriptAbilitySystemComponent* CreateTestASC(AActor* TestActor)
	{
		UAngelscriptAbilitySystemComponent* ASC = NewObject<UAngelscriptAbilitySystemComponent>(TestActor);
		TestActor->AddInstanceComponent(ASC);
		ASC->RegisterComponent();
		ASC->InitAbilityActorInfo(TestActor, TestActor);
		return ASC;
	}

	// Acquire-once / reset-once: every TEST_METHOD now uses ASTEST_GET_ENGINE() and
	// relies on its own ON_SCOPE_EXIT { Engine.DiscardModule(...); } to leave the
	// shared engine clean for the next method. Each method already uses unique
	// module + class names, so they do not collide with each other across the
	// shared engine. See Documents/Guides/ASTestSuiteMemoryPeakRootCause.md §4.1
	// (2026-05-14 GAS migration pilot) for rationale.
	BEFORE_ALL()
	{
		ASTEST_CREATE_ENGINE();
	}

	AFTER_ALL()
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		ASTEST_RESET_ENGINE(Engine);
	}

	TEST_METHOD(BP_GetOwningActorReturnsCorrectActor)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrOverrideOwner"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrOverrideOwner.as"),
			TEXT(R"AS(
UCLASS()
class UTestOwnerAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Luck;
}
)AS"),
			TEXT("UTestOwnerAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		UAngelscriptAttributeSet* AttrSet = ASC->RegisterAttributeSet(AttrSetSubclass);
		if (!TestRunner->TestNotNull(TEXT("Attribute set should be registered"), AttrSet)) { return; }

		AActor* OwningActor = AttrSet->BP_GetOwningActor();
		TestRunner->TestEqual(
			TEXT("BP_GetOwningActor should return the actor that owns the ASC"),
			OwningActor, TestActor);
	}

	TEST_METHOD(BP_GetOwningAbilitySystemComponentReturnsCorrectASC)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrOverrideASC"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrOverrideASC.as"),
			TEXT(R"AS(
UCLASS()
class UTestASCRefAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Wisdom;
}
)AS"),
			TEXT("UTestASCRefAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		UAngelscriptAttributeSet* AttrSet = ASC->RegisterAttributeSet(AttrSetSubclass);
		if (!TestRunner->TestNotNull(TEXT("Attribute set should be registered"), AttrSet)) { return; }

		UAngelscriptAbilitySystemComponent* OwningASC = AttrSet->BP_GetOwningAbilitySystemComponent();
		TestRunner->TestEqual(
			TEXT("BP_GetOwningAbilitySystemComponent should return the ASC that owns this set"),
			OwningASC, ASC);
	}

	TEST_METHOD(AttributeSetTrySetBaseValueFromSelf)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrOverrideSelfSet"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrOverrideSelfSet.as"),
			TEXT(R"AS(
UCLASS()
class UTestSelfSetAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Dexterity;
}
)AS"),
			TEXT("UTestSelfSetAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		UAngelscriptAttributeSet* AttrSet = ASC->RegisterAttributeSet(AttrSetSubclass);
		if (!TestRunner->TestNotNull(TEXT("Attribute set should be registered"), AttrSet)) { return; }

		bool bSetResult = AttrSet->TrySetAttributeBaseValue(TEXT("Dexterity"), 77.f);
		TestRunner->TestTrue(TEXT("TrySetAttributeBaseValue from set should succeed"), bSetResult);

		float OutValue = 0.f;
		bool bGetResult = AttrSet->TryGetAttributeBaseValue(TEXT("Dexterity"), OutValue);
		TestRunner->TestTrue(TEXT("TryGetAttributeBaseValue from set should succeed"), bGetResult);
		TestRunner->TestEqual(TEXT("TryGetAttributeBaseValue should return the value set"), OutValue, 77.f);
	}

	TEST_METHOD(AttributeSetTryGetCurrentValueFromSelf)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrOverrideSelfGet"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrOverrideSelfGet.as"),
			TEXT(R"AS(
UCLASS()
class UTestSelfGetAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Intelligence;
}
)AS"),
			TEXT("UTestSelfGetAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		UAngelscriptAttributeSet* AttrSet = ASC->RegisterAttributeSet(AttrSetSubclass);
		if (!TestRunner->TestNotNull(TEXT("Attribute set should be registered"), AttrSet)) { return; }

		AttrSet->TrySetAttributeBaseValue(TEXT("Intelligence"), 55.f);

		float OutValue = 0.f;
		bool bGetResult = AttrSet->TryGetAttributeCurrentValue(TEXT("Intelligence"), OutValue);
		TestRunner->TestTrue(TEXT("TryGetAttributeCurrentValue from set should succeed"), bGetResult);
		TestRunner->TestEqual(TEXT("TryGetAttributeCurrentValue should return the current value"), OutValue, 55.f);
	}

	TEST_METHOD(PostInitPropertiesSetsAttributeName)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrOverridePostInit"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrOverridePostInit.as"),
			TEXT(R"AS(
UCLASS()
class UTestPostInitAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Charisma;
}
)AS"),
			TEXT("UTestPostInitAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAttributeSet* AttrSet = NewObject<UAngelscriptAttributeSet>(GetTransientPackage(), AttributeSetClass);
		if (!TestRunner->TestNotNull(TEXT("Attribute set should be created"), AttrSet)) { return; }

		FStructProperty* CharismaProp = FindFProperty<FStructProperty>(AttributeSetClass, TEXT("Charisma"));
		if (!TestRunner->TestNotNull(TEXT("Charisma property should exist"), CharismaProp)) { return; }

		FAngelscriptGameplayAttributeData* AttrData = CharismaProp->ContainerPtrToValuePtr<FAngelscriptGameplayAttributeData>(AttrSet);
		if (!TestRunner->TestNotNull(TEXT("Attribute data pointer should be valid"), AttrData)) { return; }

		TestRunner->TestEqual(
			TEXT("PostInitProperties should set AttributeName to the property name"),
			AttrData->AttributeName, FName(TEXT("Charisma")));
	}

	TEST_METHOD(TrySetBaseValueFailsWithoutASC)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrOverrideNoASC"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrOverrideNoASC.as"),
			TEXT(R"AS(
UCLASS()
class UTestNoASCAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Fortitude;
}
)AS"),
			TEXT("UTestNoASCAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAttributeSet* AttrSet = NewObject<UAngelscriptAttributeSet>(GetTransientPackage(), AttributeSetClass);
		if (!TestRunner->TestNotNull(TEXT("Attribute set should be created"), AttrSet)) { return; }

		bool bResult = AttrSet->TrySetAttributeBaseValue(TEXT("Fortitude"), 10.f);
		TestRunner->TestFalse(
			TEXT("TrySetAttributeBaseValue should fail when attribute set has no owning ASC"),
			bResult);
	}

	TEST_METHOD(BP_GetActorInfoReturnsValidAfterInit)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrOverrideActorInfo"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrOverrideActorInfo.as"),
			TEXT(R"AS(
UCLASS()
class UTestActorInfoAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Vitality;
}
)AS"),
			TEXT("UTestActorInfoAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		UAngelscriptAttributeSet* AttrSet = ASC->RegisterAttributeSet(AttrSetSubclass);
		if (!TestRunner->TestNotNull(TEXT("Attribute set should be registered"), AttrSet)) { return; }

		FGameplayAbilityActorInfo& ActorInfo = AttrSet->BP_GetActorInfo();
		TestRunner->TestEqual(
			TEXT("BP_GetActorInfo should return actor info with the correct owner"),
			ActorInfo.OwnerActor.Get(), TestActor);
	}

	TEST_METHOD(MultipleAttributeFieldsGetCorrectNames)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrOverrideMultiName"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrOverrideMultiName.as"),
			TEXT(R"AS(
UCLASS()
class UTestMultiNameAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Attack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Defense;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Speed;
}
)AS"),
			TEXT("UTestMultiNameAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAttributeSet* AttrSet = NewObject<UAngelscriptAttributeSet>(GetTransientPackage(), AttributeSetClass);
		if (!TestRunner->TestNotNull(TEXT("Attribute set should be created"), AttrSet)) { return; }

		auto VerifyAttributeName = [&](const TCHAR* FieldName)
		{
			FStructProperty* Prop = FindFProperty<FStructProperty>(AttributeSetClass, FieldName);
			if (TestRunner->TestNotNull(*FString::Printf(TEXT("%s property should exist"), FieldName), Prop))
			{
				FAngelscriptGameplayAttributeData* Data = Prop->ContainerPtrToValuePtr<FAngelscriptGameplayAttributeData>(AttrSet);
				if (TestRunner->TestNotNull(*FString::Printf(TEXT("%s data pointer should be valid"), FieldName), Data))
				{
					TestRunner->TestEqual(
						*FString::Printf(TEXT("%s AttributeName should match property name"), FieldName),
						Data->AttributeName, FName(FieldName));
				}
			}
		};

		VerifyAttributeName(TEXT("Attack"));
		VerifyAttributeName(TEXT("Defense"));
		VerifyAttributeName(TEXT("Speed"));
	}

	TEST_METHOD(ASCModAttributeUnsafeAppliesModifier)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrOverrideModUnsafe"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrOverrideModUnsafe.as"),
			TEXT(R"AS(
UCLASS()
class UTestModUnsafeAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Rage;
}
)AS"),
			TEXT("UTestModUnsafeAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Rage"), 10.f);

		FGameplayAttribute RageAttr = UAngelscriptAttributeSet::GetGameplayAttribute(AttributeSetClass, TEXT("Rage"));
		if (!TestRunner->TestTrue(TEXT("Rage attribute should be valid"), RageAttr.IsValid())) { return; }

		ASC->ModAttributeUnsafe(RageAttr, EGameplayModOp::Additive, 5.f);

		float CurrentValue = ASC->GetAttributeCurrentValue(AttrSetSubclass, TEXT("Rage"), -1.f);
		TestRunner->TestEqual(
			TEXT("ModAttributeUnsafe with Additive should increase the current value"),
			CurrentValue, 15.f);
	}

	TEST_METHOD(SetAttributeBaseValueCheckedAndGetChecked)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrOverrideChecked"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrOverrideChecked.as"),
			TEXT(R"AS(
UCLASS()
class UTestCheckedAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Focus;
}
)AS"),
			TEXT("UTestCheckedAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);

		ASC->SetAttributeBaseValue(AttrSetSubclass, TEXT("Focus"), 99.f);
		float BaseValue = ASC->GetAttributeBaseValueChecked(AttrSetSubclass, TEXT("Focus"));
		TestRunner->TestEqual(
			TEXT("GetAttributeBaseValueChecked should return the value set by SetAttributeBaseValue"),
			BaseValue, 99.f);

		float CurrentValue = ASC->GetAttributeCurrentValueChecked(AttrSetSubclass, TEXT("Focus"));
		TestRunner->TestEqual(
			TEXT("GetAttributeCurrentValueChecked should return the current value matching base"),
			CurrentValue, 99.f);
	}

	TEST_METHOD(TryGetAttributeBaseValueFromSelf)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrOverrideGetBase"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrOverrideGetBase.as"),
			TEXT(R"AS(
UCLASS()
class UTestGetBaseAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Endurance;
}
)AS"),
			TEXT("UTestGetBaseAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		UAngelscriptAttributeSet* AttrSet = ASC->RegisterAttributeSet(AttrSetSubclass);
		if (!TestRunner->TestNotNull(TEXT("Attribute set should be registered"), AttrSet)) { return; }

		AttrSet->TrySetAttributeBaseValue(TEXT("Endurance"), 66.f);

		float OutValue = 0.f;
		bool bResult = AttrSet->TryGetAttributeBaseValue(TEXT("Endurance"), OutValue);
		TestRunner->TestTrue(TEXT("TryGetAttributeBaseValue from set should succeed"), bResult);
		TestRunner->TestEqual(TEXT("TryGetAttributeBaseValue should return the base value"), OutValue, 66.f);
	}

	TEST_METHOD(TrySetBaseValueWithInvalidNameReturnsFalse)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrOverrideInvalidSet"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrOverrideInvalidSet.as"),
			TEXT(R"AS(
UCLASS()
class UTestInvalidSetAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Valid;
}
)AS"),
			TEXT("UTestInvalidSetAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		UAngelscriptAttributeSet* AttrSet = ASC->RegisterAttributeSet(AttrSetSubclass);
		if (!TestRunner->TestNotNull(TEXT("Attribute set should be registered"), AttrSet)) { return; }

		bool bResult = AttrSet->TrySetAttributeBaseValue(TEXT("NonExistent"), 10.f);
		TestRunner->TestFalse(
			TEXT("TrySetAttributeBaseValue with invalid attribute name should return false"),
			bResult);
	}

	TEST_METHOD(TryGetCurrentValueWithInvalidNameReturnsFalse)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrOverrideInvalidGet"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrOverrideInvalidGet.as"),
			TEXT(R"AS(
UCLASS()
class UTestInvalidGetAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Valid;
}
)AS"),
			TEXT("UTestInvalidGetAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		UAngelscriptAttributeSet* AttrSet = ASC->RegisterAttributeSet(AttrSetSubclass);
		if (!TestRunner->TestNotNull(TEXT("Attribute set should be registered"), AttrSet)) { return; }

		float OutValue = 0.f;
		bool bResult = AttrSet->TryGetAttributeCurrentValue(TEXT("NonExistent"), OutValue);
		TestRunner->TestFalse(
			TEXT("TryGetAttributeCurrentValue with invalid attribute name should return false"),
			bResult);
	}

	TEST_METHOD(BP_GetOwningActorReturnsNullWithoutASC)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrOverrideNoASCOwner"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrOverrideNoASCOwner.as"),
			TEXT(R"AS(
UCLASS()
class UTestNoASCOwnerAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Temp;
}
)AS"),
			TEXT("UTestNoASCOwnerAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAttributeSet* AttrSet = NewObject<UAngelscriptAttributeSet>(GetTransientPackage(), AttributeSetClass);
		AActor* Owner = AttrSet->BP_GetOwningActor();
		TestRunner->TestNull(
			TEXT("BP_GetOwningActor should return null when attribute set has no owning ASC"),
			Owner);
	}

	TEST_METHOD(BP_GetOwningAbilitySystemComponentReturnsNullWithoutASC)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("AttrOverrideNoASCComp"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("AttrOverrideNoASCComp.as"),
			TEXT(R"AS(
UCLASS()
class UTestNoASCCompAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Temp;
}
)AS"),
			TEXT("UTestNoASCCompAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		UAngelscriptAttributeSet* AttrSet = NewObject<UAngelscriptAttributeSet>(GetTransientPackage(), AttributeSetClass);
		UAngelscriptAbilitySystemComponent* ASC = AttrSet->BP_GetOwningAbilitySystemComponent();
		TestRunner->TestNull(
			TEXT("BP_GetOwningAbilitySystemComponent should return null when attribute set has no owning ASC"),
			ASC);
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
