// AngelscriptGASFGameplayAttributeBindingsTests.cpp
//
// CQTest coverage for FGameplayAttribute bindings and ASC accessor methods:
//   - FGameplayAttribute default validity
//   - FGameplayAttribute from script set validity and class retrieval
//   - ASC GetAbilityActorInfo / GetAvatar / GetPlayerController
//   - BP_InitAbilityActorInfo updates actor info
//   - GetAttributeCurrentValue / GetAttributeBaseValue with defaults
//   - TryGetAttributeCurrentValue / TryGetAttributeBaseValue failure paths
//
// Automation IDs: Angelscript.GAS.Bindings.FGameplayAttribute.*

#include "AngelscriptAbilitySystemComponent.h"
#include "AngelscriptAttributeSet.h"
#include "Components/ActorTestSpawner.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "Misc/ScopeExit.h"

#if WITH_ANGELSCRIPT_UNITTESTS


namespace
{
	struct FAngelscriptGASAttributeFixture
	{
		FActorTestSpawner Spawner;

		AActor* SpawnActor()
		{
			return &Spawner.SpawnActor<AActor>();
		}

		UAngelscriptAbilitySystemComponent* CreateASC(
			AActor* OwnerActor,
			AActor* AvatarActor = nullptr,
			bool bInitAbilityActorInfo = true,
			bool bAllowAutomaticComponentInit = true)
		{
			UAngelscriptAbilitySystemComponent* ASC = NewObject<UAngelscriptAbilitySystemComponent>(OwnerActor);
			OwnerActor->AddInstanceComponent(ASC);
			if (!bAllowAutomaticComponentInit)
			{
				ASC->bWantsInitializeComponent = false;
			}
			ASC->RegisterComponent();
			if (bInitAbilityActorInfo)
			{
				ASC->InitAbilityActorInfo(OwnerActor, AvatarActor != nullptr ? AvatarActor : OwnerActor);
			}
			return ASC;
		}
	};
}

TEST_CLASS_WITH_FLAGS(FAngelscriptGASFGameplayAttributeBindingsTests,
	"Angelscript.GAS.Bindings.FGameplayAttribute",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
{
	TEST_METHOD(FGameplayAttributeDefaultIsInvalid)
	{
		FGameplayAttribute DefaultAttr;
		TestRunner->TestFalse(
			TEXT("Default-constructed FGameplayAttribute should be invalid"),
			DefaultAttr.IsValid());
	}

	TEST_METHOD(FGameplayAttributeFromScriptSetIsValid)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_CREATE_ENGINE_FULL();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("FGABindingsValid"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("FGABindingsValid.as"),
			TEXT(R"AS(
UCLASS()
class UTestFGAValidAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Power;
}
)AS"),
			TEXT("UTestFGAValidAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		FGameplayAttribute Attr = UAngelscriptAttributeSet::GetGameplayAttribute(AttributeSetClass, TEXT("Power"));
		TestRunner->TestTrue(
			TEXT("FGameplayAttribute from a script attribute set should be valid"),
			Attr.IsValid());
	}

	TEST_METHOD(FGameplayAttributeGetAttributeSetClassReturnsCorrectClass)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_CREATE_ENGINE_FULL();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("FGABindingsClass"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("FGABindingsClass.as"),
			TEXT(R"AS(
UCLASS()
class UTestFGAClassAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Agility;
}
)AS"),
			TEXT("UTestFGAClassAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		FGameplayAttribute Attr = UAngelscriptAttributeSet::GetGameplayAttribute(AttributeSetClass, TEXT("Agility"));
		TestRunner->TestTrue(TEXT("Attribute should be valid"), Attr.IsValid());
		TestRunner->TestEqual(
			TEXT("GetAttributeSetClass should return the script attribute set class"),
			Attr.GetAttributeSetClass(), AttributeSetClass);
	}

	TEST_METHOD(FGameplayAttributeGetAttributeSetClassReturnsNullWhenInvalid)
	{
		FAngelscriptEngine& Engine = ASTEST_CREATE_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		asIScriptModule* Module = BuildModule(
			*TestRunner,
			Engine,
			"FGABindingsInvalidAttributeClass",
			TEXT(R"AS(
int InvalidAttributeSetClassIsNull()
{
	FGameplayAttribute Attr;
	return Attr.GetAttributeSetClass() == null ? 1 : 0;
}
)AS"));
		if (Module == nullptr) { return; }
		ON_SCOPE_EXIT { Engine.DiscardModule(TEXT("FGABindingsInvalidAttributeClass")); };

		asIScriptFunction* Function = GetFunctionByDecl(
			*TestRunner, *Module, TEXT("int InvalidAttributeSetClassIsNull()"));
		if (Function == nullptr) { return; }

		int32 Result = 0;
		if (!ExecuteIntFunction(*TestRunner, Engine, *Function, Result)) { return; }
		TestRunner->TestEqual(
			TEXT("FGameplayAttribute binding should return null class for invalid attributes"),
			Result, 1);
	}

	TEST_METHOD(ASCGetAbilityActorInfoReturnsValidInfo)
	{
		FAngelscriptGASAttributeFixture Fixture;
		AActor* TestActor = Fixture.SpawnActor();
		UAngelscriptAbilitySystemComponent* ASC = Fixture.CreateASC(TestActor);

		FGameplayAbilityActorInfo& ActorInfo = ASC->GetAbilityActorInfo();
		TestRunner->TestEqual(
			TEXT("GetAbilityActorInfo should return info with correct OwnerActor after init"),
			ActorInfo.OwnerActor.Get(), TestActor);
	}

	TEST_METHOD(ASCGetAvatarReturnsCorrectActor)
	{
		FAngelscriptGASAttributeFixture Fixture;
		AActor* OwnerActor = Fixture.SpawnActor();
		AActor* AvatarActor = Fixture.SpawnActor();
		UAngelscriptAbilitySystemComponent* ASC = Fixture.CreateASC(OwnerActor, AvatarActor);

		TestRunner->TestEqual(
			TEXT("GetAvatar should return the avatar actor passed to InitAbilityActorInfo"),
			ASC->GetAvatar(), AvatarActor);
	}

	TEST_METHOD(ASCGetPlayerControllerReturnsNullWithoutController)
	{
		FAngelscriptGASAttributeFixture Fixture;
		AActor* TestActor = Fixture.SpawnActor();
		UAngelscriptAbilitySystemComponent* ASC = Fixture.CreateASC(TestActor);

		TestRunner->TestNull(
			TEXT("GetPlayerController should return null when no PlayerController is set"),
			ASC->GetPlayerController());
	}

	TEST_METHOD(BP_InitAbilityActorInfoSetsActorInfo)
	{
		FAngelscriptGASAttributeFixture Fixture;
		AActor* OwnerActor = Fixture.SpawnActor();
		AActor* AvatarActor = Fixture.SpawnActor();
		UAngelscriptAbilitySystemComponent* ASC = Fixture.CreateASC(OwnerActor);

		ASC->BP_InitAbilityActorInfo(OwnerActor, AvatarActor);

		FGameplayAbilityActorInfo& ActorInfo = ASC->GetAbilityActorInfo();
		TestRunner->TestEqual(
			TEXT("BP_InitAbilityActorInfo should set OwnerActor in ActorInfo"),
			ActorInfo.OwnerActor.Get(), OwnerActor);
		TestRunner->TestEqual(
			TEXT("BP_InitAbilityActorInfo should set AvatarActor in ActorInfo"),
			ActorInfo.AvatarActor.Get(), AvatarActor);
	}

	TEST_METHOD(GetAttributeCurrentValueReturnsDefaultWhenMissing)
	{
		FAngelscriptGASAttributeFixture Fixture;
		AActor* TestActor = Fixture.SpawnActor();
		UAngelscriptAbilitySystemComponent* ASC = Fixture.CreateASC(TestActor);

		float Value = ASC->GetAttributeCurrentValue(nullptr, TEXT("NonExistent"), -999.f);
		TestRunner->TestEqual(
			TEXT("GetAttributeCurrentValue should return DefaultValue when attribute does not exist"),
			Value, -999.f);
	}

	TEST_METHOD(GetAttributeBaseValueReturnsDefaultWhenMissing)
	{
		FAngelscriptGASAttributeFixture Fixture;
		AActor* TestActor = Fixture.SpawnActor();
		UAngelscriptAbilitySystemComponent* ASC = Fixture.CreateASC(TestActor);

		float Value = ASC->GetAttributeBaseValue(nullptr, TEXT("NonExistent"), -777.f);
		TestRunner->TestEqual(
			TEXT("GetAttributeBaseValue should return DefaultValue when attribute does not exist"),
			Value, -777.f);
	}

	TEST_METHOD(TryGetAttributeCurrentValueReturnsFalseWhenMissing)
	{
		FAngelscriptGASAttributeFixture Fixture;
		AActor* TestActor = Fixture.SpawnActor();
		UAngelscriptAbilitySystemComponent* ASC = Fixture.CreateASC(TestActor);

		float OutValue = 0.f;
		bool bResult = ASC->TryGetAttributeCurrentValue(nullptr, TEXT("NonExistent"), OutValue);
		TestRunner->TestFalse(
			TEXT("TryGetAttributeCurrentValue should return false when attribute does not exist"),
			bResult);
	}

	TEST_METHOD(TryGetAttributeBaseValueReturnsFalseWhenMissing)
	{
		FAngelscriptGASAttributeFixture Fixture;
		AActor* TestActor = Fixture.SpawnActor();
		UAngelscriptAbilitySystemComponent* ASC = Fixture.CreateASC(TestActor);

		float OutValue = 0.f;
		bool bResult = ASC->TryGetAttributeBaseValue(nullptr, TEXT("NonExistent"), OutValue);
		TestRunner->TestFalse(
			TEXT("TryGetAttributeBaseValue should return false when attribute does not exist"),
			bResult);
	}

	TEST_METHOD(ASCGetAvatarReturnsNullBeforeInit)
	{
		FAngelscriptGASAttributeFixture Fixture;
		AActor* TestActor = Fixture.SpawnActor();
		UAngelscriptAbilitySystemComponent* ASC = Fixture.CreateASC(
			TestActor, nullptr, false, false);

		TestRunner->TestNull(
			TEXT("GetAvatar should return null before InitAbilityActorInfo is called"),
			ASC->GetAvatar());
	}

	TEST_METHOD(FGameplayAttributeFromInvalidClassIsInvalid)
	{
		FGameplayAttribute Attr;
		const bool bFound = UAngelscriptAttributeSet::TryGetGameplayAttribute(nullptr, TEXT("SomeAttr"), Attr);
		TestRunner->TestFalse(
			TEXT("TryGetGameplayAttribute should return false for null class"),
			bFound);
		TestRunner->TestFalse(
			TEXT("FGameplayAttribute from null class should be invalid"),
			Attr.IsValid());
	}

	TEST_METHOD(TrySetAttributeBaseValueReturnsTrueOnSuccess)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_CREATE_ENGINE_FULL();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("FGABindingsSetBase"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("FGABindingsSetBase.as"),
			TEXT(R"AS(
UCLASS()
class UTestSetBaseAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Vitality;
}
)AS"),
			TEXT("UTestSetBaseAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		FAngelscriptGASAttributeFixture Fixture;
		AActor* TestActor = Fixture.SpawnActor();
		UAngelscriptAbilitySystemComponent* ASC = Fixture.CreateASC(TestActor);

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);

		bool bResult = ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Vitality"), 99.f);
		TestRunner->TestTrue(
			TEXT("TrySetAttributeBaseValue should return true for a valid registered attribute"),
			bResult);
	}

	TEST_METHOD(GetAttributeCurrentValueCheckedReturnsCorrectValue)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_CREATE_ENGINE_FULL();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("FGABindingsGetCurrent"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("FGABindingsGetCurrent.as"),
			TEXT(R"AS(
UCLASS()
class UTestGetCurrentAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Wisdom;
}
)AS"),
			TEXT("UTestGetCurrentAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		FAngelscriptGASAttributeFixture Fixture;
		AActor* TestActor = Fixture.SpawnActor();
		UAngelscriptAbilitySystemComponent* ASC = Fixture.CreateASC(TestActor);

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Wisdom"), 55.f);

		float OutValue = 0.f;
		bool bResult = ASC->TryGetAttributeCurrentValue(AttrSetSubclass, TEXT("Wisdom"), OutValue);
		TestRunner->TestTrue(TEXT("TryGetAttributeCurrentValue should return true"), bResult);
		TestRunner->TestEqual(
			TEXT("TryGetAttributeCurrentValue should return the correct current value"),
			OutValue, 55.f);
	}

	TEST_METHOD(GetAttributeBaseValueCheckedReturnsCorrectValue)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_CREATE_ENGINE_FULL();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("FGABindingsGetBase"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("FGABindingsGetBase.as"),
			TEXT(R"AS(
UCLASS()
class UTestGetBaseAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Intelligence;
}
)AS"),
			TEXT("UTestGetBaseAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		FAngelscriptGASAttributeFixture Fixture;
		AActor* TestActor = Fixture.SpawnActor();
		UAngelscriptAbilitySystemComponent* ASC = Fixture.CreateASC(TestActor);

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		ASC->RegisterAttributeSet(AttrSetSubclass);
		ASC->TrySetAttributeBaseValue(AttrSetSubclass, TEXT("Intelligence"), 77.f);

		float OutValue = 0.f;
		bool bResult = ASC->TryGetAttributeBaseValue(AttrSetSubclass, TEXT("Intelligence"), OutValue);
		TestRunner->TestTrue(TEXT("TryGetAttributeBaseValue should return true"), bResult);
		TestRunner->TestEqual(
			TEXT("TryGetAttributeBaseValue should return the correct base value"),
			OutValue, 77.f);
	}
};

#endif // WITH_ANGELSCRIPT_UNITTESTS
