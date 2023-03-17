using UnrealBuildTool; 

public class CauldronGenWorld: ModuleRules 

{ 

	public CauldronGenWorld(ReadOnlyTargetRules Target) : base(Target) 

	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PrivateIncludePaths.AddRange(new string[] 
		{
			"Runtime/Renderer/Private",
			"CauldronGenWorld/Private"
		});
		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.Add("TargetPlatform");
		}
		PublicDependencyModuleNames.Add("Core");
		PublicDependencyModuleNames.Add("Engine");
		PublicDependencyModuleNames.Add("MaterialShaderQualitySettings");
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Renderer",
			"RenderCore",
			"RHI",
			"Projects"
		});
		
		if (Target.bBuildEditor == true)
		{

			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"UnrealEd",
					"MaterialUtilities",
					"SlateCore",
					"Slate"
				}
			);

			CircularlyReferencedDependentModules.AddRange(
				new string[] {
					"UnrealEd",
					"MaterialUtilities",
				}
			);
		}
	} 

}