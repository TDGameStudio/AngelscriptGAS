using UnrealBuildTool;

namespace UnrealBuildTool.Rules
{
	public class AngelscriptGAS : ModuleRules
	{
		public AngelscriptGAS(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"AngelscriptRuntime",
				"AngelscriptGameplayTags",
				"GameplayAbilities",
				"GameplayTags",
				"GameplayTasks",
			});
		}
	}
}
