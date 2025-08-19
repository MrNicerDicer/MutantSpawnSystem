class CfgPatches
{
    class MutantSpawnSystem
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {
            "DZ_Data",
            "DZ_Scripts",
            "DZ_Characters",
            "DZ_Characters_Zombies",
            "DZ_Animals",
            "DZ_AI"
        };
        author = "MrNicerDicer";
        name = "Mutant Spawn System - Server Only";
        version = "2.0";
        serverOnly = 1;
    };
};

class CfgMods
{
    class MutantSpawnSystem
    {
        dir = "MutantSpawnSystem";
        picture = "";
        action = "";
        hideName = 1;
        hidePicture = 1;
        name = "MutantSpawnSystem";
        credits = "MrNicerDicer";
        author = "MrNicerDicer";
        authorID = "0";
        version = "2.0";
        extra = 0;
        type = "mod";
        
        serverOnly = 1;
        
        dependencies[] = {"World", "Mission"};
        
        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"MutantSpawnSystem/scripts/4_World"};
            };
        };
    };
};