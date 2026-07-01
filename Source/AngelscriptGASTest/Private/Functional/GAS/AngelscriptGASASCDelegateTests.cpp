// AngelscriptGASASCDelegateTests.cpp
//
// CQTest coverage for UAngelscriptAbilitySystemComponent delegate broadcasting:
//   - OnInitAbilityActorInfo
//   - OnAbilityGiven / OnAbilityRemoved
//   - OnOwnedTagUpdated
//   - OnAttributeSetRegistered (immediate and late-bind)
//
// Automation IDs: Angelscript.GAS.Functional.ASCDelegates.*

#include "AngelscriptAbilitySystemComponent.h"
#include "AngelscriptGASAbility.h"
#include "AngelscriptGASTestDelegateCapture.h"
#include "Components/ActorTestSpawner.h"
#include "CQTest.h"
#include "Shared/AngelscriptFunctionalTestUtils.h"
#include "Shared/AngelscriptTestMacros.h"
#include "GameplayTagsManager.h"
#include "Misc/ScopeExit.h"

#if WITH_ANGELSCRIPT_UNITTESTS


namespace AngelscriptGASASCDelegateTestHelpers
{
	struct FDelegateCapture
	{
		bool bFired = false;
		AActor* CapturedOwner = nullptr;
		AActor* CapturedAvatar = nullptr;
		FGameplayTag CapturedTag;
		bool CapturedTagExists = false;
		int32 FireCount = 0;
	};
}

TEST_CLASS_WITH_FLAGS(FAngelscriptGASASCDelegateTests,
	"Angelscript.GAS.Functional.ASCDelegates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
{
	FActorTestSpawner Spawner;

	AActor* SpawnTestActor()
	{
		return &Spawner.SpawnActor<AActor>();
	}

	UAngelscriptAbilitySystemComponent* CreateTestASC(AActor* OwnerActor, AActor* AvatarActor = nullptr, bool bInitAbilityActorInfo = true)
	{
		UAngelscriptAbilitySystemComponent* ASC = NewObject<UAngelscriptAbilitySystemComponent>(OwnerActor);
		OwnerActor->AddInstanceComponent(ASC);
		ASC->RegisterComponent();
		if (bInitAbilityActorInfo)
		{
			ASC->InitAbilityActorInfo(OwnerActor, AvatarActor != nullptr ? AvatarActor : OwnerActor);
		}
		return ASC;
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

	TEST_METHOD(OnInitAbilityActorInfoBroadcastsOnInit)
	{
		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor, nullptr, false);

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnInitAbilityActorInfo.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleInitAbilityActorInfo);

		ASC->InitAbilityActorInfo(TestActor, TestActor);

		TestRunner->TestTrue(TEXT("OnInitAbilityActorInfo should fire on InitAbilityActorInfo"), Capture->bFired);
		TestRunner->TestEqual(TEXT("OnInitAbilityActorInfo should pass correct owner"), Capture->CapturedOwner, TestActor);
		TestRunner->TestEqual(TEXT("OnInitAbilityActorInfo should pass correct avatar"), Capture->CapturedAvatar, TestActor);

		ASC->OnInitAbilityActorInfo.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleInitAbilityActorInfo);
	}

	TEST_METHOD(OnInitAbilityActorInfoNotBroadcastWhenUnbound)
	{
		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor, nullptr, false);

		ASC->InitAbilityActorInfo(TestActor, TestActor);
		TestRunner->TestTrue(TEXT("InitAbilityActorInfo with unbound delegate should not crash"), true);
	}

	TEST_METHOD(OnAbilityGivenBroadcastsOnGiveAbility)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("ASCDelegateGive"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("ASCDelegateGive.as"),
			TEXT(R"AS(
UCLASS()
class UTestDelegateGiveAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestDelegateGiveAbility"));
		if (AbilityClass == nullptr) { return; }

		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnAbilityGiven.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAbilityGiven);

		ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);
		TestRunner->TestTrue(TEXT("OnAbilityGiven should fire when an ability is given"), Capture->bFired);

		ASC->OnAbilityGiven.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAbilityGiven);
	}

	TEST_METHOD(OnAbilityRemovedBroadcastsOnClearAbility)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("ASCDelegateRemove"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("ASCDelegateRemove.as"),
			TEXT(R"AS(
UCLASS()
class UTestDelegateRemoveAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestDelegateRemoveAbility"));
		if (AbilityClass == nullptr) { return; }

		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		FGameplayAbilitySpecHandle GivenHandle = ASC->BP_GiveAbility(AbilityClass, 1, -1, nullptr);

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnAbilityRemoved.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAbilityRemoved);

		ASC->ClearAbility(GivenHandle);
		TestRunner->TestTrue(TEXT("OnAbilityRemoved should fire when an ability is cleared"), Capture->bFired);

		ASC->OnAbilityRemoved.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAbilityRemoved);
	}

	TEST_METHOD(OnOwnedTagUpdatedBroadcastsOnTagChange)
	{
		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Test"), false);
		if (!TestTag.IsValid())
		{
			TestRunner->AddInfo(TEXT("Gameplay tag 'Ability.Test' not registered, skipping"));
			return;
		}

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		Capture->FilterTag = TestTag;
		Capture->bUseTagFilter = true;
		ASC->OnOwnedTagUpdated.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleOwnedTagUpdated);

		ASC->AddLooseGameplayTag(TestTag);
		TestRunner->TestTrue(TEXT("OnOwnedTagUpdated should fire when a tag is added"), Capture->bFired);
		TestRunner->TestTrue(TEXT("OnOwnedTagUpdated should pass TagExists=true on add"), Capture->bCapturedTagExists);

		ASC->OnOwnedTagUpdated.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleOwnedTagUpdated);
	}

	TEST_METHOD(OnOwnedTagUpdatedBroadcastsOnTagRemoval)
	{
		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Test"), false);
		if (!TestTag.IsValid())
		{
			TestRunner->AddInfo(TEXT("Gameplay tag 'Ability.Test' not registered, skipping"));
			return;
		}

		ASC->AddLooseGameplayTag(TestTag);

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		Capture->FilterTag = TestTag;
		Capture->bUseTagFilter = true;
		Capture->bCapturedTagExists = true;
		ASC->OnOwnedTagUpdated.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleOwnedTagUpdated);

		ASC->RemoveLooseGameplayTag(TestTag);
		TestRunner->TestTrue(TEXT("OnOwnedTagUpdated should fire when a tag is removed"), Capture->bFired);
		TestRunner->TestFalse(TEXT("OnOwnedTagUpdated should pass TagExists=false on remove"), Capture->bCapturedTagExists);

		ASC->OnOwnedTagUpdated.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleOwnedTagUpdated);
	}

	TEST_METHOD(OnAttributeSetRegisteredDelegateFires)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("ASCDelegateAttrReg"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AttributeSetClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("ASCDelegateAttrReg.as"),
			TEXT(R"AS(
UCLASS()
class UTestDelegateRegAttributes : UAngelscriptAttributeSet
{
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAngelscriptGameplayAttributeData Vigor;
}
)AS"),
			TEXT("UTestDelegateRegAttributes"));
		if (AttributeSetClass == nullptr) { return; }

		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnAttributeSetRegistered(Capture, GET_FUNCTION_NAME_CHECKED(UAngelscriptGASTestDelegateCapture, HandleAttributeSetRegistered));

		TSubclassOf<UAngelscriptAttributeSet> AttrSetSubclass(AttributeSetClass);
		UAngelscriptAttributeSet* RegisteredSet = ASC->RegisterAttributeSet(AttrSetSubclass);
		TestRunner->TestNotNull(
			TEXT("RegisterAttributeSet should succeed for delegate test"),
			RegisteredSet);
		TestRunner->TestTrue(TEXT("OnAttributeSetRegistered should fire for a newly registered set"), Capture->bFired);
		TestRunner->TestEqual(TEXT("OnAttributeSetRegistered should pass the registered set"), Capture->CapturedAttributeSet, RegisteredSet);
	}

	TEST_METHOD(MultipleGiveAbilityFiresOnAbilityGivenEachTime)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("ASCDelegateMultiGive"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClassA = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("ASCDelegateMultiGive.as"),
			TEXT(R"AS(
UCLASS()
class UTestMultiGiveAbilityA : UAngelscriptGASAbility
{
}

UCLASS()
class UTestMultiGiveAbilityB : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestMultiGiveAbilityA"));
		if (AbilityClassA == nullptr) { return; }

		UClass* AbilityClassB = FindGeneratedClass(&Engine, TEXT("UTestMultiGiveAbilityB"));
		if (!TestRunner->TestNotNull(TEXT("Second ability class should be generated"), AbilityClassB)) { return; }

		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnAbilityGiven.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAbilityGiven);

		ASC->BP_GiveAbility(AbilityClassA, 1, -1, nullptr);
		ASC->BP_GiveAbility(AbilityClassB, 1, -1, nullptr);

		TestRunner->TestEqual(
			TEXT("OnAbilityGiven should fire once per GiveAbility call"),
			Capture->FireCount, 2);

		ASC->OnAbilityGiven.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAbilityGiven);
	}

	TEST_METHOD(OnInitAbilityActorInfoFiresWithDifferentOwnerAndAvatar)
	{
		AActor* OwnerActor = SpawnTestActor();
		AActor* AvatarActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(OwnerActor, nullptr, false);

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnInitAbilityActorInfo.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleInitAbilityActorInfo);

		ASC->InitAbilityActorInfo(OwnerActor, AvatarActor);

		TestRunner->TestEqual(TEXT("OnInitAbilityActorInfo should pass correct owner when different from avatar"),
			Capture->CapturedOwner, OwnerActor);
		TestRunner->TestEqual(TEXT("OnInitAbilityActorInfo should pass correct avatar when different from owner"),
			Capture->CapturedAvatar, AvatarActor);

		ASC->OnInitAbilityActorInfo.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleInitAbilityActorInfo);
	}

	TEST_METHOD(OnInitAbilityActorInfoFiresOnReInit)
	{
		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnInitAbilityActorInfo.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleInitAbilityActorInfo);

		ASC->InitAbilityActorInfo(TestActor, TestActor);
		ASC->InitAbilityActorInfo(TestActor, TestActor);

		TestRunner->TestEqual(TEXT("OnInitAbilityActorInfo should fire each time InitAbilityActorInfo is called"),
			Capture->FireCount, 2);

		ASC->OnInitAbilityActorInfo.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleInitAbilityActorInfo);
	}

	TEST_METHOD(OnAbilityGivenPassesCorrectSpec)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("ASCDelegateSpecInfo"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("ASCDelegateSpecInfo.as"),
			TEXT(R"AS(
UCLASS()
class UTestSpecInfoAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestSpecInfoAbility"));
		if (AbilityClass == nullptr) { return; }

		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnAbilityGiven.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAbilityGiven);

		ASC->BP_GiveAbility(AbilityClass, 7, 2, nullptr);
		TestRunner->TestEqual(TEXT("OnAbilityGiven Spec should have correct Level"), Capture->CapturedAbilityLevel, 7);
		TestRunner->TestEqual(TEXT("OnAbilityGiven Spec should have correct InputID"), Capture->CapturedAbilityInputID, 2);

		ASC->OnAbilityGiven.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAbilityGiven);
	}

	TEST_METHOD(OnAbilityRemovedPassesCorrectSpec)
	{
		using namespace AngelscriptFunctionalTestUtils;
		FAngelscriptEngine& Engine = ASTEST_GET_ENGINE();
		FAngelscriptEngineScope EngineScope(Engine);

		static const FName ModuleName(TEXT("ASCDelegateRemoveSpec"));
		ON_SCOPE_EXIT { Engine.DiscardModule(*ModuleName.ToString()); };

		UClass* AbilityClass = CompileScriptModule(
			*TestRunner, Engine, ModuleName,
			TEXT("ASCDelegateRemoveSpec.as"),
			TEXT(R"AS(
UCLASS()
class UTestRemoveSpecAbility : UAngelscriptGASAbility
{
}
)AS"),
			TEXT("UTestRemoveSpecAbility"));
		if (AbilityClass == nullptr) { return; }

		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		FGameplayAbilitySpecHandle GivenHandle = ASC->BP_GiveAbility(AbilityClass, 3, 4, nullptr);

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		ASC->OnAbilityRemoved.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAbilityRemoved);

		ASC->ClearAbility(GivenHandle);
		TestRunner->TestEqual(TEXT("OnAbilityRemoved Spec should have the Level from when it was given"),
			Capture->CapturedAbilityLevel, 3);

		ASC->OnAbilityRemoved.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleAbilityRemoved);
	}

	TEST_METHOD(OnOwnedTagUpdatedDoesNotFireForUnrelatedTag)
	{
		AActor* TestActor = SpawnTestActor();
		UAngelscriptAbilitySystemComponent* ASC = CreateTestASC(TestActor);

		FGameplayTag TagA = FGameplayTag::RequestGameplayTag(TEXT("Ability.Test"), false);
		FGameplayTag TagB = FGameplayTag::RequestGameplayTag(TEXT("Ability.Cooldown"), false);
		if (!TagA.IsValid() || !TagB.IsValid())
		{
			TestRunner->AddInfo(TEXT("Required tags not registered, skipping"));
			return;
		}

		UAngelscriptGASTestDelegateCapture* Capture = NewObject<UAngelscriptGASTestDelegateCapture>(GetTransientPackage());
		Capture->FilterTag = TagB;
		Capture->bUseTagFilter = true;
		ASC->OnOwnedTagUpdated.AddDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleOwnedTagUpdated);

		ASC->AddLooseGameplayTag(TagA);
		TestRunner->TestFalse(
			TEXT("OnOwnedTagUpdated should not fire for TagB when only TagA is added"),
			Capture->bFired);

		ASC->OnOwnedTagUpdated.RemoveDynamic(Capture, &UAngelscriptGASTestDelegateCapture::HandleOwnedTagUpdated);
	}
};

#endif // WITH_ANGELSCRIPT_UNITTESTS
