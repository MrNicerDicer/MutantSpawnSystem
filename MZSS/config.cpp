class CfgPatches
{
    class MZSS
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
        name = "MZSS";
        version = "2.8.7";
        serverOnly = 1;
    };

    // Legacy dependency support for servers/other mods still checking the old patch name.
    class MutantSpawnSystem
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
        author = "MrNicerDicer";
        name = "MutantSpawnSystem Legacy";
        version = "2.8.7";
        serverOnly = 1;
    };
};

class CfgMods
{
    class MZSS
    {
        dir = "MZSS";
        picture = "";
        action = "";
        hideName = 1;
        hidePicture = 1;
        name = "MZSS";
        credits = "MrNicerDicer";
        author = "MrNicerDicer";
        authorID = "0";
        version = "2.8.7";
        extra = 0;
        type = "mod";
        serverOnly = 1;
        dependencies[] = {"World"};

        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"MZSS/scripts/4_World"};
            };
        };
    };
};
