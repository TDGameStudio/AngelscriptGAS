// AngelscriptGASCharacterTagTests.cpp
//
// CQTest coverage for GAS base class tag integration:
//   - AAngelscriptGASCharacter::GetOwnedGameplayTags reflects ASC tags
//   - Base class GetAbilitySystemComponent returns correct ASC
//   - Script subclass inheritance of interfaces
//
// Automation IDs: Angelscript.GAS.Functional.CharacterTags.*

#include "AngelscriptGASActor.h"
#include "AngelscriptGASCharacter.h"
#include "AngelscriptGASPawn.h"
#include "AngelscriptAbilitySystemComponent.h"
#include "Components/ActorTestSpawner.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "GameplayTagsManager.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS


TEST_CLASS_WITH_FLAGS(FAngelscriptGASCharacterTagTests,
	"Angelscript.GAS.Functional.CharacterTags",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
{
	FActorTestSpawner Spawner;

	template <typename ActorType>
	ActorType* SpawnConcreteScriptSubclass(
		FAngelscriptEngine& Engine,
		FName ModuleName,
		const FString& Filename,
		const FString& Source,
		FName ExpectedGeneratedClassName)
	{
		using namespace AngelscriptFunctionalTestUtils;

		UClass* ScriptClass = CompileScriptModule(
			*TestRunner,
			Engine,
			ModuleName,
			Filename,
			Source,
			ExpectedGeneratedClassName);
		if (ScriptClass == nullptr)
		{
			return nullptr;
		}

		return SpawnScriptActor<ActorType>(*TestRunner, Spawner, ScriptClass);
	}

	AAngelscriptGASCharacter* SpawnConcreteGASCharacter(FAngelscriptEngine& Engine, FName ModuleName, const FString& ClassName)
	{
		return SpawnConcreteScriptSubclass<AAngelscriptGASCharacter>(
			Engine,
			ModuleName,
			ClassName + TEXT(".as"),
			FString::Printf(TEXT(R"AS(
UCLASS()
class %s : AAngelscriptGASCharacter
{
}
)AS"), *ClassName),
			FName(*ClassName));
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

	TEST_METHOD(GASActorGetAbilitySystemComponentReturnsASC)
	{
		const AAngelscriptGASActor* Actor = GetDefault<AAngelscriptGASActor>();
		if (!TestRunner->TestNotNull(TEXT("AAngelscriptGASActor should have a class default object"), Actor)) { return; }
		UAbilitySystemComponent* ASC = Actor->GetAbilitySystemComponent();
		TestRunner->TestNotNull(
			TEXT("AAngelscriptGASActor CDO GetAbilitySystemComponent should return non-null"),
			ASC);
	}

	TEST_METHOD(GASPawnGetAbilitySystemComponentReturnsASC)
	{
		const AAngelscriptGASPawn* Pawn = GetDefault<AAngelscriptGASPawn>();
		if (!TestRunner->TestNotNull(TEXT("AAngelscriptGASPawn should have a class default object"), Pawn)) { return; }
		UAbilitySystemComponent* ASC = Pawn->GetAbilitySystemComponent();
		TestRunner->TestNotNull(
			TEXT("AAngelscriptGASPawn CDO GetAbilitySystemComponent should return non-null"),
			ASC);
	}

	TEST_METHOD(GASCharacterGetAbilitySystemComponentReturnsASC)
	{
		const AAngelscriptGASCharacter* Character = GetDefault<AAngelscriptGASCharacter>();
		if (!TestRunner->TestNotNull(TEXT("AAngelscriptGASCharacter should have a class default object"), Character)) { return; }
		UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
		TestRunner->TestNotNull(
			TEXT("AAngelscriptGASCharacter CDO GetAbilitySystemComponent should return non-null"),
			ASC);
	}

	TEST_METHOD(GetOwnedGameplayTagsReturnsEmptyByDefault)
	{
		const AAngelscriptGASCharacter* Character = GetDefault<AAngelscriptGASCharacter>();
		if (!TestRunner->TestNotNull(TEXT("AAngelscriptGASCharacter should have a class default object"), Character)) { return; }
		FGameplayTagContainer TagContainer;
		Character->GetOwnedGameplayTags(TagContainer);
		TestRunner->TestEqual(
			TEXT("GetOwnedGameplayTags should return empty container by default"),
			TagContainer.Num(), 0);
	}

	TEST_METHOD(GetOwnedGameplayTagsReflectsASCTags)
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("CharTagConcreteReflectsTags"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		AAngelscriptGASCharacter* Character = SpawnConcreteGASCharacter(Engine, ModuleName, TEXT("ATestCharTagReflectsTags"));
		if (!TestRunner->TestNotNull(TEXT("Concrete GAS character subclass should spawn"), Character)) { return; }

		UAngelscriptAbilitySystemComponent* ASC = Cast<UAngelscriptAbilitySystemComponent>(Character->GetAbilitySystemComponent());
		if (!TestRunner->TestNotNull(TEXT("Character ASC should exist"), ASC)) { return; }

		FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Test"), false);
		if (!TestTag.IsValid())
		{
			TestRunner->AddInfo(TEXT("Tag 'Ability.Test' not registered, skipping"));
			return;
		}

		ASC->AddLooseGameplayTag(TestTag);

		FGameplayTagContainer TagContainer;
		Character->GetOwnedGameplayTags(TagContainer);
		TestRunner->TestTrue(
			TEXT("GetOwnedGameplayTags should reflect tags added to the ASC"),
			TagContainer.HasTag(TestTag));
	}

	TEST_METHOD(GetOwnedGameplayTagsUpdatesOnTagRemoval)
	{
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("CharTagConcreteRemovesTags"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		AAngelscriptGASCharacter* Character = SpawnConcreteGASCharacter(Engine, ModuleName, TEXT("ATestCharTagRemovesTags"));
		if (!TestRunner->TestNotNull(TEXT("Concrete GAS character subclass should spawn"), Character)) { return; }

		UAngelscriptAbilitySystemComponent* ASC = Cast<UAngelscriptAbilitySystemComponent>(Character->GetAbilitySystemComponent());
		if (!TestRunner->TestNotNull(TEXT("Character ASC should exist"), ASC)) { return; }

		FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Test"), false);
		if (!TestTag.IsValid())
		{
			TestRunner->AddInfo(TEXT("Tag 'Ability.Test' not registered, skipping"));
			return;
		}

		ASC->AddLooseGameplayTag(TestTag);
		ASC->RemoveLooseGameplayTag(TestTag);

		FGameplayTagContainer TagContainer;
		Character->GetOwnedGameplayTags(TagContainer);
		TestRunner->TestFalse(
			TEXT("GetOwnedGameplayTags should not contain a removed tag"),
			TagContainer.HasTag(TestTag));
	}

	TEST_METHOD(ScriptSubclassOfGASPawnInheritsASC)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("CharTagPawnSub"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* ScriptPawnClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("CharTagPawnSub.as"),
			TEXT(R"AS(
UCLASS()
class ATestGASPawnSub : AAngelscriptGASPawn
{
}
)AS"),
			TEXT("ATestGASPawnSub"));
		if (ScriptPawnClass == nullptr) { return; }

		TestRunner->TestTrue(
			TEXT("Script subclass of AAngelscriptGASPawn should implement IAbilitySystemInterface"),
			ScriptPawnClass->ImplementsInterface(UAbilitySystemInterface::StaticClass()));
	}

	TEST_METHOD(ScriptSubclassOfGASCharacterInheritsInterfaces)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("CharTagCharSub"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* ScriptCharClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("CharTagCharSub.as"),
			TEXT(R"AS(
UCLASS()
class ATestGASCharacterSub : AAngelscriptGASCharacter
{
}
)AS"),
			TEXT("ATestGASCharacterSub"));
		if (ScriptCharClass == nullptr) { return; }

		TestRunner->TestTrue(
			TEXT("Script subclass of AAngelscriptGASCharacter should implement IAbilitySystemInterface"),
			ScriptCharClass->ImplementsInterface(UAbilitySystemInterface::StaticClass()));
		TestRunner->TestTrue(
			TEXT("Script subclass of AAngelscriptGASCharacter should implement IGameplayTagAssetInterface"),
			ScriptCharClass->ImplementsInterface(UGameplayTagAssetInterface::StaticClass()));
	}

	TEST_METHOD(GASCharacterGetOwnedGameplayTagsReturnsEmptyByDefault)
	{
		const AAngelscriptGASCharacter* Character = GetDefault<AAngelscriptGASCharacter>();
		if (!TestRunner->TestNotNull(TEXT("AAngelscriptGASCharacter should have a class default object"), Character)) { return; }
		FGameplayTagContainer Tags;
		Character->GetOwnedGameplayTags(Tags);
		TestRunner->TestTrue(
			TEXT("GetOwnedGameplayTags should return empty container by default"),
			Tags.IsEmpty());
	}

	TEST_METHOD(GASCharacterGetAbilitySystemComponentReturnsNonNull)
	{
		const AAngelscriptGASCharacter* Character = GetDefault<AAngelscriptGASCharacter>();
		if (!TestRunner->TestNotNull(TEXT("AAngelscriptGASCharacter should have a class default object"), Character)) { return; }
		UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
		TestRunner->TestNotNull(
			TEXT("AAngelscriptGASCharacter CDO GetAbilitySystemComponent should return non-null"),
			ASC);
	}

	TEST_METHOD(GASCharacterASCIsAngelscriptType)
	{
		const AAngelscriptGASCharacter* Character = GetDefault<AAngelscriptGASCharacter>();
		if (!TestRunner->TestNotNull(TEXT("AAngelscriptGASCharacter should have a class default object"), Character)) { return; }
		UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
		if (!TestRunner->TestNotNull(TEXT("ASC should exist"), ASC)) { return; }
		TestRunner->TestTrue(
			TEXT("AAngelscriptGASCharacter ASC should be UAngelscriptAbilitySystemComponent"),
			ASC->IsA<UAngelscriptAbilitySystemComponent>());
	}

	TEST_METHOD(GASPawnGetAbilitySystemComponentReturnsNonNull)
	{
		const AAngelscriptGASPawn* Pawn = GetDefault<AAngelscriptGASPawn>();
		if (!TestRunner->TestNotNull(TEXT("AAngelscriptGASPawn should have a class default object"), Pawn)) { return; }
		UAbilitySystemComponent* ASC = Pawn->GetAbilitySystemComponent();
		TestRunner->TestNotNull(
			TEXT("AAngelscriptGASPawn CDO GetAbilitySystemComponent should return non-null"),
			ASC);
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
