// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class TrueSkyPlugin : ModuleRules
	{
		public TrueSkyPlugin(TargetInfo Target)
		{
			PublicIncludePaths.AddRange(
				new string[] {
					"Core",
                    "Developer/AssetTools/Private",
                    "Editor/PlacementMode/Private",
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"TrueSkyPlugin/Private"
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Slate",
					"Engine",
					"UnrealEd",
					"LevelEditor",
					"MainFrame",
                    "AssetTools",
                    "PlacementMode",
                    "UnrealEd",
                    "CollectionManager",
                    "ContentBrowser",
					"EditorStyle"
					// ... add other public dependencies that you statically link with here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"RenderCore",
                    "RHI",
                    "D3D11RHI",
					"Slate",
					"SlateCore",
                    "Renderer"
					// ... add private dependencies that you statically link with here ...
				}
				);

            AddThirdPartyPrivateStaticDependencies(Target,"DX11");
		}
	}
}