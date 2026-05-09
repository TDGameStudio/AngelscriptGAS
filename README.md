# AngelscriptGAS

AngelscriptGAS is an optional Gameplay Ability System extension plugin for UnrealAngelscriptPlugin.

Install it next to the core plugin in an Unreal project:

```text
<ProjectRoot>/Plugins/Angelscript/
<ProjectRoot>/Plugins/AngelscriptGAS/
```

## Dependencies

This plugin depends on:

- `UnrealAngelscriptPlugin` installed as `Plugins/Angelscript`.
- Unreal Engine Gameplay Ability System plugins such as `GameplayAbilities`, `GameplayTags`, and `GameplayTasks` as required by the `.uplugin` descriptor and project setup.

## Contents

- `AngelscriptGAS.uplugin` - Unreal plugin descriptor.
- `Config/` - plugin configuration.
- `Source/AngelscriptGAS/` - runtime/editor-facing GAS extension code for AngelScript integration.
- `Source/AngelscriptGASTest/` - automation tests for the GAS extension.

## Basic Usage

1. Install `UnrealAngelscriptPlugin` as `Plugins/Angelscript`.
2. Clone this repository as `Plugins/AngelscriptGAS`.
3. Regenerate project files if needed.
4. Build your editor target with both plugins enabled.

## History

This repository starts from a snapshot import. Earlier development history and planning context remain in `TDGameStudio/AngelscriptProject`.
