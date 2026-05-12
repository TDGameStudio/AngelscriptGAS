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
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "GameplayTagsManager.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS

using namespace AngelscriptTestSupport;

TEST_CLASS_WITH_FLAGS(FAngelscriptGASCharacterTagTests,
	"Angelscript.GAS.Functional.CharacterTags",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
{
	TEST_METHOD(GASActorGetAbilitySystemComponentReturnsASC)
	{
		AAngelscriptGASActor* Actor = NewObject<AAngelscriptGASActor>(GetTransientPackage());
		UAbilitySystemComponent* ASC = Actor->GetAbilitySystemComponent();
		TestRunner->TestNotNull(
			TEXT("AAngelscriptGASActor::GetAbilitySystemComponent should return non-null"),
			ASC);
	}

	TEST_METHOD(GASPawnGetAbilitySystemComponentReturnsASC)
	{
		AAngelscriptGASPawn* Pawn = NewObject<AAngelscriptGASPawn>(GetTransientPackage());
		UAbilitySystemComponent* ASC = Pawn->GetAbilitySystemComponent();
		TestRunner->TestNotNull(
			TEXT("AAngelscriptGASPawn::GetAbilitySystemComponent should return non-null"),
			ASC);
	}

	TEST_METHOD(GASCharacterGetAbilitySystemComponentReturnsASC)
	{
		AAngelscriptGASCharacter* Character = NewObject<AAngelscriptGASCharacter>(GetTransientPackage());
		UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
		TestRunner->TestNotNull(
			TEXT("AAngelscriptGASCharacter::GetAbilitySystemComponent should return non-null"),
			ASC);
	}

	TEST_METHOD(GetOwnedGameplayTagsReturnsEmptyByDefault)
	{
		AAngelscriptGASCharacter* Character = NewObject<AAngelscriptGASCharacter>(GetTransientPackage());
		FGameplayTagContainer TagContainer;
		Character->GetOwnedGameplayTags(TagContainer);
		TestRunner->TestEqual(
			TEXT("GetOwnedGameplayTags should return empty container by default"),
			TagContainer.Num(), 0);
	}

	TEST_METHOD(GetOwnedGameplayTagsReflectsASCTags)
	{
		AAngelscriptGASCharacter* Character = NewObject<AAngelscriptGASCharacter>(GetTransientPackage());
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
		AAngelscriptGASCharacter* Character = NewObject<AAngelscriptGASCharacter>(GetTransientPackage());
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
		FAngelscriptEngine& Engine = ASTEST_CREATE_ENGINE_FULL();
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
		FAngelscriptEngine& Engine = ASTEST_CREATE_ENGINE_FULL();
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
		AAngelscriptGASCharacter* Character = NewObject<AAngelscriptGASCharacter>(GetTransientPackage());
		FGameplayTagContainer Tags;
		Character->GetOwnedGameplayTags(Tags);
		TestRunner->TestTrue(
			TEXT("GetOwnedGameplayTags should return empty container by default"),
			Tags.IsEmpty());
	}

	TEST_METHOD(GASCharacterGetAbilitySystemComponentReturnsNonNull)
	{
		AAngelscriptGASCharacter* Character = NewObject<AAngelscriptGASCharacter>(GetTransientPackage());
		UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
		TestRunner->TestNotNull(
			TEXT("AAngelscriptGASCharacter::GetAbilitySystemComponent should return non-null"),
			ASC);
	}

	TEST_METHOD(GASCharacterASCIsAngelscriptType)
	{
		AAngelscriptGASCharacter* Character = NewObject<AAngelscriptGASCharacter>(GetTransientPackage());
		UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
		if (!TestRunner->TestNotNull(TEXT("ASC should exist"), ASC)) { return; }
		TestRunner->TestTrue(
			TEXT("AAngelscriptGASCharacter ASC should be UAngelscriptAbilitySystemComponent"),
			ASC->IsA<UAngelscriptAbilitySystemComponent>());
	}

	TEST_METHOD(GASPawnGetAbilitySystemComponentReturnsNonNull)
	{
		AAngelscriptGASPawn* Pawn = NewObject<AAngelscriptGASPawn>(GetTransientPackage());
		UAbilitySystemComponent* ASC = Pawn->GetAbilitySystemComponent();
		TestRunner->TestNotNull(
			TEXT("AAngelscriptGASPawn::GetAbilitySystemComponent should return non-null"),
			ASC);
	}
};

#endif // WITH_DEV_AUTOMATION_TESTS
