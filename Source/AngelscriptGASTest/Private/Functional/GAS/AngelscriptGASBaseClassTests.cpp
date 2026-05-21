// AngelscriptGASBaseClassTests.cpp
//
// CQTest coverage for GAS base actor classes:
//   - AAngelscriptGASActor IAbilitySystemInterface
//   - AAngelscriptGASPawn IAbilitySystemInterface
//   - AAngelscriptGASCharacter IAbilitySystemInterface + IGameplayTagAssetInterface
//
// Automation IDs: Angelscript.GAS.Functional.BaseClasses.*

#include "AngelscriptGASActor.h"
#include "AngelscriptGASCharacter.h"
#include "AngelscriptGASPawn.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS

using namespace AngelscriptTestSupport;

TEST_CLASS_WITH_FLAGS(FAngelscriptGASBaseClassTests,
	"Angelscript.GAS.Functional.BaseClasses",
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

	TEST_METHOD(GASActorImplementsAbilitySystemInterface)
	{
		UClass* ActorClass = AAngelscriptGASActor::StaticClass();
		TestRunner->TestTrue(
			TEXT("AAngelscriptGASActor should implement IAbilitySystemInterface"),
			ActorClass->ImplementsInterface(UAbilitySystemInterface::StaticClass()));
	}

	TEST_METHOD(GASActorHasAbilitySystemComponent)
	{
		UClass* ActorClass = AAngelscriptGASActor::StaticClass();
		FProperty* ASCProp = ActorClass->FindPropertyByName(TEXT("AbilitySystem"));
		TestRunner->TestNotNull(
			TEXT("AAngelscriptGASActor should have an AbilitySystem property"),
			ASCProp);
	}

	TEST_METHOD(GASPawnImplementsAbilitySystemInterface)
	{
		UClass* PawnClass = AAngelscriptGASPawn::StaticClass();
		TestRunner->TestTrue(
			TEXT("AAngelscriptGASPawn should implement IAbilitySystemInterface"),
			PawnClass->ImplementsInterface(UAbilitySystemInterface::StaticClass()));
	}

	TEST_METHOD(GASPawnHasAbilitySystemComponent)
	{
		UClass* PawnClass = AAngelscriptGASPawn::StaticClass();
		FProperty* ASCProp = PawnClass->FindPropertyByName(TEXT("AbilitySystem"));
		TestRunner->TestNotNull(
			TEXT("AAngelscriptGASPawn should have an AbilitySystem property"),
			ASCProp);
	}

	TEST_METHOD(GASCharacterImplementsAbilitySystemInterface)
	{
		UClass* CharClass = AAngelscriptGASCharacter::StaticClass();
		TestRunner->TestTrue(
			TEXT("AAngelscriptGASCharacter should implement IAbilitySystemInterface"),
			CharClass->ImplementsInterface(UAbilitySystemInterface::StaticClass()));
	}

	TEST_METHOD(GASCharacterImplementsGameplayTagAssetInterface)
	{
		UClass* CharClass = AAngelscriptGASCharacter::StaticClass();
		TestRunner->TestTrue(
			TEXT("AAngelscriptGASCharacter should implement IGameplayTagAssetInterface"),
			CharClass->ImplementsInterface(UGameplayTagAssetInterface::StaticClass()));
	}

	TEST_METHOD(GASCharacterHasAbilitySystemComponent)
	{
		UClass* CharClass = AAngelscriptGASCharacter::StaticClass();
		FProperty* ASCProp = CharClass->FindPropertyByName(TEXT("AbilitySystem"));
		TestRunner->TestNotNull(
			TEXT("AAngelscriptGASCharacter should have an AbilitySystem property"),
			ASCProp);
	}

	TEST_METHOD(GASActorASCPropertyIsCorrectType)
	{
		UClass* ActorClass = AAngelscriptGASActor::StaticClass();
		FObjectPropertyBase* ASCProp = CastField<FObjectPropertyBase>(
			ActorClass->FindPropertyByName(TEXT("AbilitySystem")));
		if (!TestRunner->TestNotNull(TEXT("AbilitySystem property should be an object property"), ASCProp))
		{
			return;
		}
		TestRunner->TestTrue(
			TEXT("AbilitySystem property should be of type UAngelscriptAbilitySystemComponent"),
			ASCProp->PropertyClass->IsChildOf(UAngelscriptAbilitySystemComponent::StaticClass()));
	}

	TEST_METHOD(ScriptSubclassInheritsASCFromGASActor)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("BaseClassScriptActor"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* ScriptActorClass = CompileScriptModule(
			*TestRunner,
			Engine,
			ModuleName,
			TEXT("BaseClassScriptActor.as"),
			TEXT(R"AS(
UCLASS()
class ATestGASActor : AAngelscriptGASActor
{
}
)AS"),
			TEXT("ATestGASActor"));
		if (ScriptActorClass == nullptr) { return; }

		TestRunner->TestTrue(
			TEXT("Script subclass of AAngelscriptGASActor should derive from AAngelscriptGASActor"),
			ScriptActorClass->IsChildOf(AAngelscriptGASActor::StaticClass()));

		TestRunner->TestTrue(
			TEXT("Script subclass should still implement IAbilitySystemInterface"),
			ScriptActorClass->ImplementsInterface(UAbilitySystemInterface::StaticClass()));
	}

	TEST_METHOD(GASActorASCIsNotNullAfterConstruction)
	{
		const AAngelscriptGASActor* Actor = GetDefault<AAngelscriptGASActor>();
		if (!TestRunner->TestNotNull(TEXT("AAngelscriptGASActor should have a class default object"), Actor)) { return; }
		UAbilitySystemComponent* ASC = Actor->GetAbilitySystemComponent();
		TestRunner->TestNotNull(
			TEXT("AAngelscriptGASActor CDO should have a non-null ASC after construction"),
			ASC);
	}

	TEST_METHOD(GASPawnASCIsNotNullAfterConstruction)
	{
		const AAngelscriptGASPawn* Pawn = GetDefault<AAngelscriptGASPawn>();
		if (!TestRunner->TestNotNull(TEXT("AAngelscriptGASPawn should have a class default object"), Pawn)) { return; }
		UAbilitySystemComponent* ASC = Pawn->GetAbilitySystemComponent();
		TestRunner->TestNotNull(
			TEXT("AAngelscriptGASPawn CDO should have a non-null ASC after construction"),
			ASC);
	}

	TEST_METHOD(GASCharacterASCIsNotNullAfterConstruction)
	{
		const AAngelscriptGASCharacter* Character = GetDefault<AAngelscriptGASCharacter>();
		if (!TestRunner->TestNotNull(TEXT("AAngelscriptGASCharacter should have a class default object"), Character)) { return; }
		UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
		TestRunner->TestNotNull(
			TEXT("AAngelscriptGASCharacter CDO should have a non-null ASC after construction"),
			ASC);
	}

	TEST_METHOD(GASPawnASCPropertyIsCorrectType)
	{
		UClass* PawnClass = AAngelscriptGASPawn::StaticClass();
		FObjectPropertyBase* ASCProp = CastField<FObjectPropertyBase>(
			PawnClass->FindPropertyByName(TEXT("AbilitySystem")));
		if (!TestRunner->TestNotNull(TEXT("AbilitySystem property should be an object property"), ASCProp))
		{
			return;
		}
		TestRunner->TestTrue(
			TEXT("AAngelscriptGASPawn AbilitySystem property should be of type UAngelscriptAbilitySystemComponent"),
			ASCProp->PropertyClass->IsChildOf(UAngelscriptAbilitySystemComponent::StaticClass()));
	}

	TEST_METHOD(GASCharacterASCPropertyIsCorrectType)
	{
		UClass* CharClass = AAngelscriptGASCharacter::StaticClass();
		FObjectPropertyBase* ASCProp = CastField<FObjectPropertyBase>(
			CharClass->FindPropertyByName(TEXT("AbilitySystem")));
		if (!TestRunner->TestNotNull(TEXT("AbilitySystem property should be an object property"), ASCProp))
		{
			return;
		}
		TestRunner->TestTrue(
			TEXT("AAngelscriptGASCharacter AbilitySystem property should be of type UAngelscriptAbilitySystemComponent"),
			ASCProp->PropertyClass->IsChildOf(UAngelscriptAbilitySystemComponent::StaticClass()));
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
