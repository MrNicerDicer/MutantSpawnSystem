class SimpleSpawnManager : Managed
{
    bool m_SystemEnabled;
    bool m_DebugMode;
    bool m_DiagnosticMode;
    bool m_LogLoad;
    bool m_LogSpawn;
    bool m_LogDespawn;
    bool m_LogReload;
    bool m_LogValidator;
    bool m_LogAdmin;
    bool m_LogDiagnostics;
    string m_SettingsSource;
    string m_TiersSource;
    string m_ZonesSource;
    int m_MaxTotalSpawnedEntities;
    bool m_PersistCooldowns;
    float m_CooldownPersistenceInterval;
    float m_CooldownSaveTimer;
    bool m_PersistWorldState;
    float m_WorldStateSaveInterval;
    float m_WorldStateSaveTimer;
    float m_StartupLogDelay;
    float m_CheckTimer;
    float m_CheckInterval;
    float m_MinSpawnDistanceFromPlayer;
    float m_MinSpawnDistanceSq;
    
    // Total elapsed time (seconds) used for grace period comparisons
    float m_ElapsedTime;
    // Grace period before despawn triggers after players leave (Legacy-Feld)
    float m_DespawnGracePeriod;
    // Despawn Delay: Sekunden ab leer bis Despawn (neues primäres Feld)
    float m_DespawnDelay;
    // Optional safety: keep zones alive while spawned living entities are close to players.
    bool m_ProtectLivingEntitiesFromDespawn;
    float m_LivingEntityDespawnProtectionDistance;
    float m_LivingEntityDespawnProtectionDistanceSq;
    // Minimum player move distance (squared) before re-checking zone assignment
    float m_PlayerMoveThresholdSq;
    // Soft Spawn: Sekunden zwischen einzelnen Spawns pro Slot
    float m_SoftSpawnInterval;
    
    // Spawn queue for burst-rate limiting
    ref array<ref SimpleSpawnPoint> m_SpawnQueuePoints;
    ref array<ref SimpleZone>       m_SpawnQueueZones;
    int m_MaxSpawnsPerTick;
    // Soft Spawn Queue: Zonen mit ausstehenden Soft Spawns
    ref array<ref SimpleZone> m_SoftSpawnZones;
    
    ref map<int, ref SimpleTier> m_Tiers;
    ref map<string, ref SimpleZone> m_ZonesMap;
    ref GlobalSettingsConfig m_GlobalSettings;
    
    ref map<int, ref array<ref SimpleZone>> m_ZoneGrid;
    static const int GRID_SIZE = 1000;
    
    // Player Cache
    ref array<Man> m_CachedPlayers;
    float m_PlayerCacheTimer;
    float m_PlayerCacheDuration;
    
    // Lazy Cleanup
    ref set<ref SimpleZone> m_DirtyZones;
    
    // Grid Cell Cache
    ref map<Man, int> m_PlayerGridCache;
    ref map<Man, vector> m_PlayerLastPos;

    // Optional migration/nest/world-state systems.
    bool m_MigrationEnabled;
    ref array<ref MZSS_MigrationLinkConfig> m_MigrationLinks;
    ref map<string, ref MZSS_MigrationLinkState> m_MigrationStateMap;
    ref array<ref MZSS_MigrationEffectState> m_ActiveMigrationEffects;

    bool m_NestsEnabled;
    ref array<ref MZSS_NestConfig> m_Nests;
    ref map<string, ref MZSS_NestState> m_NestStateMap;
    ref map<string, ref array<EntityAI>> m_NestEntities;

    // Optional personal zone notifications.
    bool m_NotificationsEnabled;
    float m_NotificationDefaultCooldown;
    ref array<ref MZSS_NotificationEntry> m_Notifications;
    ref map<string, float> m_NotificationCooldowns;

    // Optional player rules.
    bool m_PlayerRulesEnabled;
    bool m_HasIgnoredBySpawnedAIRules;
    ref map<string, ref MZSS_PlayerRuleEntry> m_PlayerRules;

    // Optional anti-clipping placement.
    bool m_AntiClippingEnabled;
    bool m_AntiClippingGroundRaycast;
    bool m_AntiClippingClearanceCheck;
    bool m_AntiClippingFallbackToOriginalPosition;
    int m_AntiClippingRetryAttempts;
    float m_AntiClippingRetryRadius;
    float m_AntiClippingMinWallDistance;
    float m_AntiClippingMinWallDistanceSq;
    float m_AntiClippingVerticalProbeUp;
    float m_AntiClippingVerticalProbeDown;

    // Optional admin-tool command polling. Keeps the main mod server-side and independent.
    float m_AdminCommandPollTimer;
    int m_LastAdminCommandId;
    
    void SimpleSpawnManager()
    {
        m_SystemEnabled = true;
        m_DebugMode = false;
        m_DiagnosticMode = false;
        m_LogLoad = true;
        m_LogSpawn = false;
        m_LogDespawn = false;
        m_LogReload = true;
        m_LogValidator = true;
        m_LogAdmin = true;
        m_LogDiagnostics = false;
        m_SettingsSource = "MZSS_Settings.json";
        m_TiersSource = "MZSS_Tiers.json";
        m_ZonesSource = "MZSS_Zones.json";
        m_MaxTotalSpawnedEntities = 0;
        m_PersistCooldowns = false;
        m_CooldownPersistenceInterval = 60.0;
        m_CooldownSaveTimer = 0.0;
        m_PersistWorldState = true;
        m_WorldStateSaveInterval = 60.0;
        m_WorldStateSaveTimer = 0.0;
        m_StartupLogDelay = 0.0;
        m_CheckTimer = 0.0;
        m_CheckInterval = 5.0;
        m_MinSpawnDistanceFromPlayer = 30.0;
        m_MinSpawnDistanceSq = 900.0;
        
        m_ElapsedTime = 0.0;
        m_DespawnGracePeriod = 120.0;
        m_DespawnDelay = 45.0;
        m_ProtectLivingEntitiesFromDespawn = true;
        m_LivingEntityDespawnProtectionDistance = 80.0;
        m_LivingEntityDespawnProtectionDistanceSq = 6400.0;
        m_PlayerMoveThresholdSq = 25.0; // 5m^2
        m_SoftSpawnInterval = 0.25;
        m_MaxSpawnsPerTick = 10;
        
        m_SpawnQueuePoints = new array<ref SimpleSpawnPoint>;
        m_SpawnQueueZones  = new array<ref SimpleZone>;
        m_SoftSpawnZones   = new array<ref SimpleZone>;
        
        m_Tiers = new map<int, ref SimpleTier>;
        m_ZonesMap = new map<string, ref SimpleZone>;
        m_ZoneGrid = new map<int, ref array<ref SimpleZone>>;
        m_GlobalSettings = new GlobalSettingsConfig;
        
        m_CachedPlayers = new array<Man>;
        m_PlayerCacheTimer = 0.0;
        m_PlayerCacheDuration = 15.0;
        
        m_DirtyZones = new set<ref SimpleZone>;
        
        m_PlayerGridCache = new map<Man, int>;
        m_PlayerLastPos = new map<Man, vector>;

        m_MigrationEnabled = false;
        m_MigrationLinks = new array<ref MZSS_MigrationLinkConfig>;
        m_MigrationStateMap = new map<string, ref MZSS_MigrationLinkState>;
        m_ActiveMigrationEffects = new array<ref MZSS_MigrationEffectState>;

        m_NestsEnabled = false;
        m_Nests = new array<ref MZSS_NestConfig>;
        m_NestStateMap = new map<string, ref MZSS_NestState>;
        m_NestEntities = new map<string, ref array<EntityAI>>;

        m_NotificationsEnabled = false;
        m_NotificationDefaultCooldown = 900.0;
        m_Notifications = new array<ref MZSS_NotificationEntry>;
        m_NotificationCooldowns = new map<string, float>;

        m_PlayerRulesEnabled = true;
        m_HasIgnoredBySpawnedAIRules = false;
        m_PlayerRules = new map<string, ref MZSS_PlayerRuleEntry>;

        m_AntiClippingEnabled = true;
        m_AntiClippingGroundRaycast = true;
        m_AntiClippingClearanceCheck = true;
        m_AntiClippingFallbackToOriginalPosition = true;
        m_AntiClippingRetryAttempts = 8;
        m_AntiClippingRetryRadius = 2.5;
        m_AntiClippingMinWallDistance = 0.8;
        m_AntiClippingMinWallDistanceSq = 0.64;
        m_AntiClippingVerticalProbeUp = 1.5;
        m_AntiClippingVerticalProbeDown = 5.0;

        m_AdminCommandPollTimer = 0.0;
        m_LastAdminCommandId = 0;
        InitAdminCommandState();
        
        Print("[MZSS] Initializing server-side spawn system v2.8...");
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CreateDefaultConfigs, 3000, false);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(LoadAllConfigs, 8000, false);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CleanupDeadEntities, 60000, true);
    }
    
    void CreateDefaultConfigs()
    {
        string configDir = "$profile:MutantSpawnSystem";

        if (!FileExist(configDir))
        {
            MakeDirectory(configDir);
            if (m_DebugMode || m_LogLoad) Print("[MZSS:LOAD] Created config directory");
        }

        EnsureSettingsFile();
        EnsureCoreConfigMigration();
        EnsurePlayerRulesFile();
        EnsureAntiClippingFile();
        EnsureMigrationFile();
        EnsureNestsFile();
        EnsureNotificationsFile();
        EnsureWorldStateFile();
    }


    void CreateDefaultSpawnerSettings(string filePath)
    {
        ref SpawnerSettingsConfig cfg = new SpawnerSettingsConfig();
        JsonFileLoader<SpawnerSettingsConfig>.JsonSaveFile(filePath, cfg);
        Print("[MZSS:LOAD] Created default " + filePath);
    }

    string GetSettingsPath()
    {
        return "$profile:MutantSpawnSystem/MZSS_Settings.json";
    }

    string GetLegacySettingsPath()
    {
        return "$profile:MutantSpawnSystem/SpawnerSettings.json";
    }

    string GetCooldownsPath()
    {
        return "$profile:MutantSpawnSystem/MZSS_Cooldowns.json";
    }

    string GetTiersPath()
    {
        return "$profile:MutantSpawnSystem/MZSS_Tiers.json";
    }

    string GetLegacyTiersPath()
    {
        return "$profile:MutantSpawnSystem/Tiers.json";
    }

    string GetZonesPath()
    {
        return "$profile:MutantSpawnSystem/MZSS_Zones.json";
    }

    string GetLegacyZonesPath()
    {
        return "$profile:MutantSpawnSystem/Zones.json";
    }

    string GetMigrationPath()
    {
        return "$profile:MutantSpawnSystem/MZSS_Migration.json";
    }

    string GetNestsPath()
    {
        return "$profile:MutantSpawnSystem/MZSS_Nests.json";
    }

    string GetWorldStatePath()
    {
        return "$profile:MutantSpawnSystem/MZSS_WorldState.json";
    }

    string GetNotificationsPath()
    {
        return "$profile:MutantSpawnSystem/MZSS_Notifications.json";
    }

    string GetPlayerRulesPath()
    {
        return "$profile:MutantSpawnSystem/MZSS_PlayerRules.json";
    }

    string GetAntiClippingPath()
    {
        return "$profile:MutantSpawnSystem/MZSS_AntiClipping.json";
    }

    bool EnsureSettingsFile()
    {
        string newPath = GetSettingsPath();
        string legacyPath = GetLegacySettingsPath();

        if (FileExist(newPath))
            return true;

        if (FileExist(legacyPath))
        {
            ref SpawnerSettingsConfig legacyCfg = new SpawnerSettingsConfig();
            JsonFileLoader<SpawnerSettingsConfig>.JsonLoadFile(legacyPath, legacyCfg);
            if (legacyCfg)
            {
                JsonFileLoader<SpawnerSettingsConfig>.JsonSaveFile(newPath, legacyCfg);
                Print("[MZSS:LOAD] Legacy SpawnerSettings.json detected. Created MZSS_Settings.json copy. Legacy file was kept.");
                return true;
            }
        }

        CreateDefaultSpawnerSettings(newPath);
        return true;
    }


    bool EnsurePlayerRulesFile()
    {
        string path = GetPlayerRulesPath();
        if (FileExist(path))
            return true;

        ref MZSS_PlayerRulesConfig cfg = new MZSS_PlayerRulesConfig();

        ref MZSS_PlayerRuleEntry example = new MZSS_PlayerRuleEntry();
        example.steamId = "76561198000000000";
        example.name = "Example Monolith / ignored player";
        example.canTriggerZones = false;
        example.ignoredBySpawnedAI = true;
        cfg.players.Insert(example);

        JsonFileLoader<MZSS_PlayerRulesConfig>.JsonSaveFile(path, cfg);
        Print("[MZSS:LOAD] Created default " + path);
        return true;
    }

    bool EnsureAntiClippingFile()
    {
        string path = GetAntiClippingPath();
        if (FileExist(path))
            return true;

        ref MZSS_AntiClippingConfig cfg = new MZSS_AntiClippingConfig();
        JsonFileLoader<MZSS_AntiClippingConfig>.JsonSaveFile(path, cfg);
        Print("[MZSS:LOAD] Created default " + path);
        return true;
    }

    void LoadPlayerRules()
    {
        m_PlayerRules.Clear();
        m_HasIgnoredBySpawnedAIRules = false;

        EnsurePlayerRulesFile();

        string path = GetPlayerRulesPath();
        if (!FileExist(path))
            return;

        ref MZSS_PlayerRulesConfig cfg = new MZSS_PlayerRulesConfig();
        JsonFileLoader<MZSS_PlayerRulesConfig>.JsonLoadFile(path, cfg);
        if (!cfg)
            return;

        m_PlayerRulesEnabled = cfg.enabled;

        if (!cfg.players)
            return;

        for (int i = 0; i < cfg.players.Count(); i++)
        {
            ref MZSS_PlayerRuleEntry entry = cfg.players.Get(i);
            if (!entry)
                continue;

            entry.steamId.TrimInPlace();
            if (entry.steamId == "")
                continue;

            m_PlayerRules.Set(entry.steamId, entry);

            if (entry.ignoredBySpawnedAI)
                m_HasIgnoredBySpawnedAIRules = true;
        }

        LogLoad("Loaded " + m_PlayerRules.Count().ToString() + " player rule(s).");
    }

    void LoadAntiClipping()
    {
        EnsureAntiClippingFile();

        string path = GetAntiClippingPath();
        if (!FileExist(path))
            return;

        ref MZSS_AntiClippingConfig cfg = new MZSS_AntiClippingConfig();
        JsonFileLoader<MZSS_AntiClippingConfig>.JsonLoadFile(path, cfg);
        if (!cfg)
            return;

        m_AntiClippingEnabled = cfg.enabled;
        m_AntiClippingGroundRaycast = cfg.groundRaycast;
        m_AntiClippingClearanceCheck = cfg.clearanceCheck;
        m_AntiClippingFallbackToOriginalPosition = cfg.fallbackToOriginalPosition;

        m_AntiClippingRetryAttempts = cfg.retryAttempts;
        if (m_AntiClippingRetryAttempts < 0)
            m_AntiClippingRetryAttempts = 0;

        m_AntiClippingRetryRadius = cfg.retryRadius;
        if (m_AntiClippingRetryRadius < 0)
            m_AntiClippingRetryRadius = 0;

        m_AntiClippingMinWallDistance = cfg.minWallDistance;
        if (m_AntiClippingMinWallDistance <= 0)
            m_AntiClippingMinWallDistance = 0.8;
        m_AntiClippingMinWallDistanceSq = m_AntiClippingMinWallDistance * m_AntiClippingMinWallDistance;

        m_AntiClippingVerticalProbeUp = cfg.verticalProbeUp;
        if (m_AntiClippingVerticalProbeUp <= 0)
            m_AntiClippingVerticalProbeUp = 1.5;

        m_AntiClippingVerticalProbeDown = cfg.verticalProbeDown;
        if (m_AntiClippingVerticalProbeDown <= 0)
            m_AntiClippingVerticalProbeDown = 5.0;

        // Backfill newly introduced values while keeping user settings.
        JsonFileLoader<MZSS_AntiClippingConfig>.JsonSaveFile(path, cfg);

        LogLoad("Loaded anti-clipping config. Enabled=" + m_AntiClippingEnabled.ToString());
    }

    bool EnsureCoreConfigMigration()
    {
        string newTiers = GetTiersPath();
        string oldTiers = GetLegacyTiersPath();

        if (!FileExist(newTiers))
        {
            if (FileExist(oldTiers))
            {
                ref TiersContainer oldTiersContainer = new TiersContainer();
                JsonFileLoader<TiersContainer>.JsonLoadFile(oldTiers, oldTiersContainer);
                if (oldTiersContainer)
                {
                    JsonFileLoader<TiersContainer>.JsonSaveFile(newTiers, oldTiersContainer);
                    Print("[MZSS:MIGRATION] Legacy Tiers.json detected. Created MZSS_Tiers.json. Legacy file was kept.");
                }
            }
            else
            {
                CreateDefaultTiers(newTiers);
            }
        }

        string newZones = GetZonesPath();
        string oldZones = GetLegacyZonesPath();

        if (!FileExist(newZones))
        {
            if (FileExist(oldZones))
            {
                ref ZonesContainer oldZonesContainer = new ZonesContainer();
                JsonFileLoader<ZonesContainer>.JsonLoadFile(oldZones, oldZonesContainer);
                if (oldZonesContainer)
                {
                    JsonFileLoader<ZonesContainer>.JsonSaveFile(newZones, oldZonesContainer);
                    Print("[MZSS:MIGRATION] Legacy Zones.json detected. Created MZSS_Zones.json. Legacy file was kept.");
                }
            }
            else
            {
                CreateDefaultZonesWithSpawnPoints(newZones);
            }
        }

        return true;
    }

    bool EnsureMigrationFile()
    {
        string path = GetMigrationPath();
        if (FileExist(path))
            return true;

        ref MZSS_MigrationConfig cfg = new MZSS_MigrationConfig();
        JsonFileLoader<MZSS_MigrationConfig>.JsonSaveFile(path, cfg);
        Print("[MZSS:LOAD] Created default " + path);
        return true;
    }

    bool EnsureNestsFile()
    {
        string path = GetNestsPath();
        if (FileExist(path))
            return true;

        ref MZSS_NestsConfig cfg = new MZSS_NestsConfig();
        JsonFileLoader<MZSS_NestsConfig>.JsonSaveFile(path, cfg);
        Print("[MZSS:LOAD] Created default " + path);
        return true;
    }

    bool EnsureNotificationsFile()
    {
        string path = GetNotificationsPath();
        if (FileExist(path))
            return true;

        ref MZSS_NotificationsConfig cfg = new MZSS_NotificationsConfig();
        JsonFileLoader<MZSS_NotificationsConfig>.JsonSaveFile(path, cfg);
        Print("[MZSS:LOAD] Created default " + path);
        return true;
    }

    bool EnsureWorldStateFile()
    {
        string path = GetWorldStatePath();
        if (FileExist(path))
            return true;

        ref MZSS_WorldStateConfig cfg = new MZSS_WorldStateConfig();
        JsonFileLoader<MZSS_WorldStateConfig>.JsonSaveFile(path, cfg);
        Print("[MZSS:LOAD] Created default " + path);
        return true;
    }

    void CreateDefaultTiers(string filePath)
    {
        ref TiersContainer container = new TiersContainer();
        
        ref TierConfig tier1 = new TierConfig();
        tier1.name = "Basic Zombies";
        tier1.classnames.Insert("ZmbM_HunterOld_Autumn");
        tier1.classnames.Insert("ZmbM_HunterOld_Spring");
        tier1.classnames.Insert("ZmbF_SurvivorNormal_Blue");
        tier1.classnames.Insert("ZmbF_SurvivorNormal_Red");
        tier1.classnames.Insert("ZmbM_FarmerFat_Beige");
        tier1.classnames.Insert("ZmbM_CitizenASkinny");
        tier1.classnames.Insert("ZmbM_CitizenBFat");
        container.tiers.Set(1, tier1);
        
        ref TierConfig tier2 = new TierConfig();
        tier2.name = "Military Zombies";
        tier2.classnames.Insert("ZmbM_SoldierNormal");
        tier2.classnames.Insert("ZmbM_PatrolNormal_PautRev");
        tier2.classnames.Insert("ZmbM_PatrolNormal_Autumn");
        tier2.classnames.Insert("ZmbM_usSoldier_normal_Woodland");
        container.tiers.Set(2, tier2);
        
        ref TierConfig tier3 = new TierConfig();
        tier3.name = "Wildlife Predators";
        tier3.classnames.Insert("Animal_UrsusArctos");
        tier3.classnames.Insert("Animal_CanisLupus_Grey");
        tier3.classnames.Insert("Animal_CanisLupus_White");
        container.tiers.Set(3, tier3);
        
        JsonFileLoader<TiersContainer>.JsonSaveFile(filePath, container);
        if (m_DebugMode) Print("[MZSS] Created default Tiers.json");
    }
    
    void CreateDefaultZonesWithSpawnPoints(string filePath)
    {
        ref ZonesContainer container = new ZonesContainer();
        
        ref ZoneConfig zone1 = new ZoneConfig();
        zone1.name = "Test_Zone_Cherno_Building";
        zone1.enabled = true;
        zone1.position = "6560 15 2630";
        zone1.triggerRadius = 100;
        zone1.spawnChance = 0.75;
        zone1.despawnOnExit = true;
        zone1.despawnDistance = 150;
        zone1.respawnCooldown = 300;
        
        ref SpawnPointConfig sp1 = new SpawnPointConfig();
        sp1.position = "6555 15 2625";
        sp1.radius = 1.5;
        sp1.tierIds.Insert(1);
        sp1.entities = 2;
        sp1.useFixedHeight = false;
        zone1.spawnPoints.Insert(sp1);
        
        ref SpawnPointConfig sp2 = new SpawnPointConfig();
        sp2.position = "6565 15 2635";
        sp2.radius = 2.0;
        sp2.tierIds.Insert(1);
        sp2.tierIds.Insert(2);
        sp2.entities = 1;
        sp2.useFixedHeight = false;
        zone1.spawnPoints.Insert(sp2);
        
        ref SpawnPointConfig sp3 = new SpawnPointConfig();
        sp3.position = "6558 18.5 2628";
        sp3.radius = 1.0;
        sp3.tierIds.Insert(2);
        sp3.entities = 3;
        sp3.useFixedHeight = true;
        zone1.spawnPoints.Insert(sp3);
        
        container.zones.Insert(zone1);
        
        ref ZoneConfig zone2 = new ZoneConfig();
        zone2.name = "Test_Zone_Underground_Bunker";
        zone2.enabled = true;
        zone2.position = "-1000 -50 -1000";
        zone2.triggerRadius = 80;
        zone2.spawnChance = 0.5;
        zone2.despawnOnExit = true;
        zone2.despawnDistance = 120;
        zone2.respawnCooldown = 420;
        
        ref SpawnPointConfig sp4 = new SpawnPointConfig();
        sp4.position = "-1000 -48.5 -995";
        sp4.radius = 2.0;
        sp4.tierIds.Insert(2);
        sp4.entities = 2;
        sp4.useFixedHeight = true;
        zone2.spawnPoints.Insert(sp4);
        
        ref SpawnPointConfig sp5 = new SpawnPointConfig();
        sp5.position = "-990 -48.5 -1000";
        sp5.radius = 1.5;
        sp5.tierIds.Insert(1);
        sp5.tierIds.Insert(2);
        sp5.entities = 3;
        sp5.useFixedHeight = true;
        zone2.spawnPoints.Insert(sp5);
        
        ref SpawnPointConfig sp6 = new SpawnPointConfig();
        sp6.position = "-1010 -48.5 -1005";
        sp6.radius = 2.5;
        sp6.tierIds.Insert(2);
        sp6.entities = 1;
        sp6.useFixedHeight = true;
        zone2.spawnPoints.Insert(sp6);
        
        container.zones.Insert(zone2);
        
        JsonFileLoader<ZonesContainer>.JsonSaveFile(filePath, container);
        if (m_DebugMode) Print("[MZSS] Created default Zones.json with 2 example zones");
    }
    
    void LoadAllConfigs()
    {
        Print("[MZSS] === LOADING SYSTEM ===");

        m_Tiers.Clear();
        m_ZonesMap.Clear();
        m_ZoneGrid.Clear();
        m_DirtyZones.Clear();
        m_PlayerGridCache.Clear();
        m_PlayerLastPos.Clear();

        bool spawnerSettingsLoaded = LoadSpawnerSettings();
        LoadPlayerRules();
        LoadAntiClipping();
        LoadMigration();
        LoadNests();
        LoadNotifications();

        LoadTiers();
        LoadZones();

        if (m_PersistCooldowns)
            LoadCooldowns();

        if (m_PersistWorldState)
            LoadWorldState();

        ValidateConfiguration();

        BuildSpatialGrid();

        Print("[MZSS] Configuration loaded:");
        if (spawnerSettingsLoaded)
            Print("[MZSS] - Settings source: " + m_SettingsSource);
        else
            Print("[MZSS] - Settings source: Zones.json (legacy)");
        Print("[MZSS] - Tiers source: " + m_TiersSource);
        Print("[MZSS] - Zones source: " + m_ZonesSource);
        Print("[MZSS] - Tiers: " + m_Tiers.Count().ToString());
        Print("[MZSS] - Zones: " + m_ZonesMap.Count().ToString());
        Print("[MZSS] - Debug Mode: " + m_DebugMode.ToString());
        Print("[MZSS] - Player Cache: " + m_PlayerCacheDuration.ToString() + "s");
        Print("[MZSS] - Spawn Budget/Tick: " + m_MaxSpawnsPerTick.ToString());
        Print("[MZSS] - Soft Spawn Interval: " + m_SoftSpawnInterval.ToString() + "s");
        Print("[MZSS] - Despawn Delay: " + m_DespawnDelay.ToString() + "s");
        Print("[MZSS] - Living Entity Despawn Protection: " + m_ProtectLivingEntitiesFromDespawn.ToString() + " (" + m_LivingEntityDespawnProtectionDistance.ToString() + "m)");
        Print("[MZSS] - Player Rules: " + m_PlayerRulesEnabled.ToString() + " (" + m_PlayerRules.Count().ToString() + " rule(s))");
        Print("[MZSS] - Anti-Clipping: " + m_AntiClippingEnabled.ToString());
        Print("[MZSS] - Migration: " + m_MigrationEnabled.ToString() + " (" + m_MigrationLinks.Count().ToString() + " link(s))");
        Print("[MZSS] - Nests: " + m_NestsEnabled.ToString() + " (" + m_Nests.Count().ToString() + " nest(s))");
        Print("[MZSS] - Notifications: " + m_NotificationsEnabled.ToString() + " (" + m_Notifications.Count().ToString() + " notification(s))");
        if (m_MaxTotalSpawnedEntities > 0)
            Print("[MZSS] - Global Entity Limit: " + m_MaxTotalSpawnedEntities.ToString());
        if (m_PersistCooldowns)
            Print("[MZSS] - Persistent Cooldowns: true");
        Print("[MZSS] === INFO ===");
        Print("[MZSS] - MADE BY MrNicerDicer");
        Print("[MZSS] - JOIN EXODUS FORGE discord.gg/Yz8S558D6M");
        Print("[MZSS] === SYSTEM READY ===");
    }


    bool LoadSpawnerSettings()
    {
        EnsureSettingsFile();

        string path = GetSettingsPath();
        m_SettingsSource = "MZSS_Settings.json";
        if (!FileExist(path))
        {
            path = GetLegacySettingsPath();
            m_SettingsSource = "SpawnerSettings.json (legacy)";
        }

        if (!FileExist(path))
            return false;

        ref SpawnerSettingsConfig cfg = new SpawnerSettingsConfig();
        JsonFileLoader<SpawnerSettingsConfig>.JsonLoadFile(path, cfg);

        if (!cfg)
            return false;

        m_SystemEnabled              = cfg.systemEnabled;
        m_CheckInterval              = cfg.checkInterval;
        m_MinSpawnDistanceFromPlayer = cfg.minSpawnDistanceFromPlayer;
        m_MinSpawnDistanceSq         = m_MinSpawnDistanceFromPlayer * m_MinSpawnDistanceFromPlayer;
        m_DebugMode                  = cfg.debugMode;
        m_DiagnosticMode             = cfg.diagnosticMode;
        m_PlayerCacheDuration        = cfg.playerCacheDuration;
        m_MaxTotalSpawnedEntities    = cfg.maxTotalSpawnedEntities;
        m_PersistCooldowns           = cfg.persistCooldowns;
        m_CooldownPersistenceInterval = cfg.cooldownPersistenceInterval;
        m_PersistWorldState           = cfg.persistWorldState;
        m_WorldStateSaveInterval      = cfg.worldStateSaveInterval;
        if (m_WorldStateSaveInterval <= 0)
            m_WorldStateSaveInterval = 60.0;
        m_ProtectLivingEntitiesFromDespawn = cfg.protectLivingEntitiesFromDespawn;
        m_LivingEntityDespawnProtectionDistance = cfg.livingEntityDespawnProtectionDistance;
        if (m_LivingEntityDespawnProtectionDistance <= 0)
            m_LivingEntityDespawnProtectionDistance = 80.0;
        m_LivingEntityDespawnProtectionDistanceSq = m_LivingEntityDespawnProtectionDistance * m_LivingEntityDespawnProtectionDistance;

        if (m_CooldownPersistenceInterval <= 0)
            m_CooldownPersistenceInterval = 60.0;

        m_LogLoad                    = cfg.logLoad;
        m_LogSpawn                   = cfg.logSpawn;
        m_LogDespawn                 = cfg.logDespawn;
        m_LogReload                  = cfg.logReload;
        m_LogValidator               = cfg.logValidator;
        m_LogAdmin                   = cfg.logAdmin;
        m_LogDiagnostics             = cfg.logDiagnostics;

        if (cfg.playerMoveThreshold > 0)
            m_PlayerMoveThresholdSq = cfg.playerMoveThreshold * cfg.playerMoveThreshold;

        if (cfg.spawnBudgetPerTick > 0)
            m_MaxSpawnsPerTick = cfg.spawnBudgetPerTick;
        else
            m_MaxSpawnsPerTick = 10;

        if (cfg.softSpawnInterval > 0)
            m_SoftSpawnInterval = cfg.softSpawnInterval;
        else
            m_SoftSpawnInterval = 0.25;

        if (cfg.despawnDelay > 0)
            m_DespawnDelay = cfg.despawnDelay;
        else
            m_DespawnDelay = 45.0;

        // Keep GlobalSettings object populated so legacy runtime fields continue to work.
        m_GlobalSettings.systemEnabled              = cfg.systemEnabled;
        m_GlobalSettings.checkInterval              = cfg.checkInterval;
        m_GlobalSettings.maxEntitiesPerZone         = cfg.maxEntitiesPerZone;
        m_GlobalSettings.entityLifetime             = cfg.entityLifetime;
        m_GlobalSettings.minSpawnDistanceFromPlayer = cfg.minSpawnDistanceFromPlayer;
        m_GlobalSettings.debugMode                  = cfg.debugMode;
        m_GlobalSettings.playerCacheDuration        = cfg.playerCacheDuration;

        // Backfill newly introduced settings into MZSS_Settings.json while keeping existing values.
        if (m_SettingsSource == "MZSS_Settings.json")
            JsonFileLoader<SpawnerSettingsConfig>.JsonSaveFile(GetSettingsPath(), cfg);

        if (m_DebugMode || m_LogLoad) Print("[MZSS:LOAD] " + m_SettingsSource + " loaded successfully");
        return true;
    }


    void LoadTiers()
    {
        string tiersPath = GetTiersPath();
        m_TiersSource = "MZSS_Tiers.json";
        if (!FileExist(tiersPath))
        {
            tiersPath = GetLegacyTiersPath();
            m_TiersSource = "Tiers.json (legacy)";
        }
        
        if (!FileExist(tiersPath))
        {
            Print("[MZSS] ERROR: No MZSS_Tiers.json or legacy Tiers.json found!");
            return;
        }
        
        ref TiersContainer container = new TiersContainer();
        JsonFileLoader<TiersContainer>.JsonLoadFile(tiersPath, container);
        
        if (container && container.tiers && container.tiers.Count() > 0)
        {
            for (int i = 0; i < container.tiers.Count(); i++)
            {
                int tierId = container.tiers.GetKey(i);
                ref TierConfig config = container.tiers.GetElement(i);
                
                if (config)
                {
                    ref SimpleTier tier = new SimpleTier();
                    tier.name = config.name;
                    
                    for (int j = 0; j < config.classnames.Count(); j++)
                    {
                        string cn = config.classnames.Get(j);
                        if (cn == "")
                            continue;
                        
                        tier.classnames.Insert(cn);
                    }
                    
                    m_Tiers.Set(tierId, tier);
                    if (m_DebugMode) Print("[MZSS] Tier " + tierId.ToString() + " geladen: " + tier.name + " (" + tier.classnames.Count().ToString() + " Classnames)");
                }
            }
        }
    }
    
    void LoadZones(bool skipGlobalSettings = false)
    {        string zonesPath = GetZonesPath();
        m_ZonesSource = "MZSS_Zones.json";
        if (!FileExist(zonesPath))
        {
            zonesPath = GetLegacyZonesPath();
            m_ZonesSource = "Zones.json (legacy)";
        }
        
        if (!FileExist(zonesPath))
        {
            Print("[MZSS] ERROR: No MZSS_Zones.json or legacy Zones.json found!");
            return;
        }
        
        ref ZonesContainer container = new ZonesContainer();
        JsonFileLoader<ZonesContainer>.JsonLoadFile(zonesPath, container);
        
        if (container)
        {
            if (container.zones && container.zones.Count() > 0)
            {
                for (int i = 0; i < container.zones.Count(); i++)
                {
                    ref ZoneConfig config = container.zones.Get(i);
                    if (config)
                    {
                        ref SimpleZone zone = new SimpleZone();
                        zone.name = config.name;
                        zone.enabled = config.enabled;
                        zone.position = config.position.ToVector();
                        zone.triggerRadius = config.triggerRadius;
                        zone.spawnChance = config.spawnChance;
                        zone.despawnOnExit = config.despawnOnExit;
                        zone.despawnDistance = config.despawnDistance;
                        zone.respawnCooldown = config.respawnCooldown;
                        zone.CacheSquaredDistances();
                        
                        int totalEntities = 0;
                        
                        for (int j = 0; j < config.spawnPoints.Count(); j++)
                        {
                            ref SpawnPointConfig spConfig = config.spawnPoints.Get(j);
                            if (spConfig)
                            {
                                ref SimpleSpawnPoint sp = new SimpleSpawnPoint();
                                sp.position = spConfig.position.ToVector();
                                sp.radius = spConfig.radius;
                                sp.entities = spConfig.entities;
                                sp.useFixedHeight = spConfig.useFixedHeight;
                                totalEntities += spConfig.entities;
                                
                                for (int k = 0; k < spConfig.tierIds.Count(); k++)
                                {
                                    sp.tierIds.Insert(spConfig.tierIds.Get(k));
                                }
                                
                                zone.spawnPoints.Insert(sp);
                            }
                        }
                        
                        m_ZonesMap.Set(zone.name, zone);
                        
                        if (m_DebugMode)
                        {
                            Print("[MZSS] Loaded zone: " + zone.name);
                            Print("[MZSS]   Spawn chance: " + (zone.spawnChance * 100).ToString() + "%");
                            Print("[MZSS]   Spawn points: " + zone.spawnPoints.Count().ToString() + " (Total entities: " + totalEntities.ToString() + ")");
                        }
                    }
                }
            }
        }
    }
    
    void ValidateConfiguration()
    {
        int warnings = 0;

        if (!m_Tiers || m_Tiers.Count() == 0)
        {
            warnings++;
            LogValidator("No tiers loaded. Nothing can spawn.");
        }

        if (!m_ZonesMap || m_ZonesMap.Count() == 0)
        {
            warnings++;
            LogValidator("No zones loaded.");
        }

        for (int t = 0; t < m_Tiers.Count(); t++)
        {
            int tierId = m_Tiers.GetKey(t);
            ref SimpleTier tier = m_Tiers.GetElement(t);
            if (!tier)
            {
                warnings++;
                LogValidator("Tier " + tierId.ToString() + " is null.");
                continue;
            }

            if (!tier.classnames || tier.classnames.Count() == 0)
            {
                warnings++;
                LogValidator("Tier " + tierId.ToString() + " has no classnames.");
                continue;
            }

            for (int c = tier.classnames.Count() - 1; c >= 0; c--)
            {
                string classname = tier.classnames.Get(c);
                classname.TrimInPlace();
                if (classname == "")
                {
                    warnings++;
                    LogValidator("Tier " + tierId.ToString() + " contains an empty classname.");
                }
            }
        }

        for (int i = 0; i < m_ZonesMap.Count(); i++)
        {
            string zoneName = m_ZonesMap.GetKey(i);
            ref SimpleZone zone = m_ZonesMap.GetElement(i);
            if (!zone)
            {
                warnings++;
                LogValidator("Zone " + zoneName + " is null.");
                continue;
            }

            if (zone.name == "")
            {
                warnings++;
                LogValidator("A zone has an empty name.");
            }

            if (zone.triggerRadius <= 0)
            {
                warnings++;
                LogValidator("Zone '" + zone.name + "' has triggerRadius <= 0.");
            }

            if (!zone.spawnPoints || zone.spawnPoints.Count() == 0)
            {
                warnings++;
                LogValidator("Zone '" + zone.name + "' has no spawn points.");
                continue;
            }

            for (int sp = 0; sp < zone.spawnPoints.Count(); sp++)
            {
                ref SimpleSpawnPoint point = zone.spawnPoints.Get(sp);
                if (!point)
                {
                    warnings++;
                    LogValidator("Zone '" + zone.name + "' has a null spawn point.");
                    continue;
                }

                if (point.radius < 0)
                {
                    warnings++;
                    LogValidator("Zone '" + zone.name + "' spawn point " + sp.ToString() + " has radius < 0.");
                }

                if (point.entities <= 0)
                {
                    warnings++;
                    LogValidator("Zone '" + zone.name + "' spawn point " + sp.ToString() + " has entities <= 0.");
                }

                if (!point.tierIds || point.tierIds.Count() == 0)
                {
                    warnings++;
                    LogValidator("Zone '" + zone.name + "' spawn point " + sp.ToString() + " has no tierIds.");
                    continue;
                }

                for (int ti = 0; ti < point.tierIds.Count(); ti++)
                {
                    int referencedTierId = point.tierIds.Get(ti);
                    ref SimpleTier checkTier;
                    if (!m_Tiers.Find(referencedTierId, checkTier))
                    {
                        warnings++;
                        LogValidator("Zone '" + zone.name + "' spawn point " + sp.ToString() + " references missing tierId " + referencedTierId.ToString() + ".");
                    }
                }
            }
        }

        if (warnings == 0)
            LogValidator("Configuration validation passed.");
        else
            LogValidator("Configuration validation completed with " + warnings.ToString() + " warning(s).");
    }

    void LogValidator(string message)
    {
        if (m_DebugMode || m_LogValidator)
            Print("[MZSS:VALIDATOR] " + message);
    }

    void LogLoad(string message)
    {
        if (m_DebugMode || m_LogLoad)
            Print("[MZSS:LOAD] " + message);
    }

    void LogSpawn(string message)
    {
        if (m_DebugMode || m_LogSpawn)
            Print("[MZSS:SPAWN] " + message);
    }

    void LogDespawn(string message)
    {
        if (m_DebugMode || m_LogDespawn)
            Print("[MZSS:DESPAWN] " + message);
    }

    void LogReload(string message)
    {
        if (m_DebugMode || m_LogReload)
            Print("[MZSS:RELOAD] " + message);
    }

    void LogAdmin(string message)
    {
        if (m_DebugMode || m_LogAdmin)
            Print("[MZSS:ADMIN] " + message);
    }

    void LogDiagnostics(string message)
    {
        if (m_DiagnosticMode || (m_DebugMode && m_LogDiagnostics))
            Print("[MZSS:DIAG] " + message);
    }


    void LogDiag(string message)
    {
        if (m_DiagnosticMode || m_LogDiagnostics)
            Print("[MZSS:DIAG] " + message);
    }

    int CountTotalSpawnedEntities()
    {
        int count = 0;
        for (int i = 0; i < m_ZonesMap.Count(); i++)
        {
            ref SimpleZone zone = m_ZonesMap.GetElement(i);
            if (!zone || !zone.spawnPoints)
                continue;

            CleanupSpawnPointEntities(zone);

            for (int j = 0; j < zone.spawnPoints.Count(); j++)
            {
                ref SimpleSpawnPoint point = zone.spawnPoints.Get(j);
                if (point && point.spawnedEntities)
                    count += point.spawnedEntities.Count();
            }
        }

        return count;
    }

    void LoadCooldowns()
    {
        string path = GetCooldownsPath();
        if (!FileExist(path))
            return;

        ref MZSS_CooldownsContainer container = new MZSS_CooldownsContainer();
        JsonFileLoader<MZSS_CooldownsContainer>.JsonLoadFile(path, container);
        if (!container || !container.cooldowns)
            return;

        int applied = 0;
        for (int i = 0; i < container.cooldowns.Count(); i++)
        {
            ref MZSS_CooldownEntry entry = container.cooldowns.Get(i);
            if (!entry || entry.zoneName == "")
                continue;

            ref SimpleZone zone;
            if (!m_ZonesMap.Find(entry.zoneName, zone) || !zone)
                continue;

            zone.cooldownTime = entry.cooldownTime;
            zone.hasSpawned = entry.hasSpawned;
            zone.hasRolledChance = entry.hasRolledChance;
            applied++;
        }

        if (applied > 0)
            LogLoad("Restored " + applied.ToString() + " persisted cooldown(s).");
    }

    void SaveCooldowns()
    {
        if (!m_PersistCooldowns)
            return;

        ref MZSS_CooldownsContainer container = new MZSS_CooldownsContainer();

        for (int i = 0; i < m_ZonesMap.Count(); i++)
        {
            ref SimpleZone zone = m_ZonesMap.GetElement(i);
            if (!zone)
                continue;

            if (zone.cooldownTime <= 0 && !zone.hasSpawned)
                continue;

            ref MZSS_CooldownEntry entry = new MZSS_CooldownEntry();
            entry.zoneName = zone.name;
            entry.cooldownTime = zone.cooldownTime;
            entry.hasSpawned = zone.hasSpawned;
            entry.hasRolledChance = zone.hasRolledChance;
            container.cooldowns.Insert(entry);
        }

        JsonFileLoader<MZSS_CooldownsContainer>.JsonSaveFile(GetCooldownsPath(), container);
        LogLoad("Saved cooldown persistence file with " + container.cooldowns.Count().ToString() + " entries.");
    }

    void DiagnoseAtPosition(vector sourcePos)
    {
        Print("[MZSS:DIAG] === DIAGNOSTIC START ===");
        Print("[MZSS:DIAG] Source position: " + sourcePos.ToString());

        ref SimpleZone nearestZone = FindNearestEnabledZone(sourcePos);
        if (!nearestZone)
        {
            Print("[MZSS:DIAG] No enabled zones found.");
            Print("[MZSS:DIAG] === DIAGNOSTIC END ===");
            return;
        }

        float distance = Math.Sqrt(vector.DistanceSq(sourcePos, nearestZone.position));
        Print("[MZSS:DIAG] Nearest zone: " + nearestZone.name + " distance=" + distance.ToString() + "m");
        Print("[MZSS:DIAG] Enabled: " + nearestZone.enabled.ToString());
        Print("[MZSS:DIAG] Trigger radius: " + nearestZone.triggerRadius.ToString());
        Print("[MZSS:DIAG] Despawn distance: " + nearestZone.despawnDistance.ToString());
        Print("[MZSS:DIAG] Cooldown: " + nearestZone.cooldownTime.ToString());
        Print("[MZSS:DIAG] Has spawned: " + nearestZone.hasSpawned.ToString());
        Print("[MZSS:DIAG] Spawn points: " + nearestZone.spawnPoints.Count().ToString());

        if (distance > nearestZone.triggerRadius)
            Print("[MZSS:DIAG] Source is outside trigger radius.");
        else
            Print("[MZSS:DIAG] Source is inside trigger radius.");

        for (int i = 0; i < nearestZone.spawnPoints.Count(); i++)
        {
            ref SimpleSpawnPoint point = nearestZone.spawnPoints.Get(i);
            if (!point)
                continue;

            Print("[MZSS:DIAG] SpawnPoint " + i.ToString() + ": entities=" + point.entities.ToString() + " radius=" + point.radius.ToString() + " tiers=" + point.tierIds.Count().ToString());

            for (int t = 0; t < point.tierIds.Count(); t++)
            {
                int tierId = point.tierIds.Get(t);
                ref SimpleTier tier;
                if (m_Tiers.Find(tierId, tier) && tier)
                    Print("[MZSS:DIAG] - tierId " + tierId.ToString() + " OK classnames=" + tier.classnames.Count().ToString());
                else
                    Print("[MZSS:DIAG] - tierId " + tierId.ToString() + " MISSING");
            }
        }

        Print("[MZSS:DIAG] Total active entities: " + CountTotalSpawnedEntities().ToString());
        Print("[MZSS:DIAG] === DIAGNOSTIC END ===");
    }

    void LoadMigration()
    {
        m_MigrationEnabled = false;
        m_MigrationLinks.Clear();

        EnsureMigrationFile();

        string path = GetMigrationPath();
        if (!FileExist(path))
            return;

        ref MZSS_MigrationConfig cfg = new MZSS_MigrationConfig();
        JsonFileLoader<MZSS_MigrationConfig>.JsonLoadFile(path, cfg);
        if (!cfg)
            return;

        m_MigrationEnabled = cfg.enabled;

        if (cfg.links)
        {
            for (int i = 0; i < cfg.links.Count(); i++)
            {
                ref MZSS_MigrationLinkConfig link = cfg.links.Get(i);
                if (!link || link.name == "")
                    continue;

                m_MigrationLinks.Insert(link);
            }
        }

        LogLoad("Loaded migration config. Enabled=" + m_MigrationEnabled.ToString() + " links=" + m_MigrationLinks.Count().ToString());
    }

    void LoadNests()
    {
        m_NestsEnabled = false;
        m_Nests.Clear();

        EnsureNestsFile();

        string path = GetNestsPath();
        if (!FileExist(path))
            return;

        ref MZSS_NestsConfig cfg = new MZSS_NestsConfig();
        JsonFileLoader<MZSS_NestsConfig>.JsonLoadFile(path, cfg);
        if (!cfg)
            return;

        m_NestsEnabled = cfg.enabled;

        if (cfg.nests)
        {
            for (int i = 0; i < cfg.nests.Count(); i++)
            {
                ref MZSS_NestConfig nest = cfg.nests.Get(i);
                if (!nest || nest.name == "")
                    continue;

                m_Nests.Insert(nest);
            }
        }

        // Backfill newly introduced nest fields while keeping user values.
        JsonFileLoader<MZSS_NestsConfig>.JsonSaveFile(path, cfg);

        LogLoad("Loaded nests config. Enabled=" + m_NestsEnabled.ToString() + " nests=" + m_Nests.Count().ToString());
    }

    void LoadNotifications()
    {
        m_NotificationsEnabled = false;
        m_Notifications.Clear();
        m_NotificationCooldowns.Clear();

        EnsureNotificationsFile();

        string path = GetNotificationsPath();
        if (!FileExist(path))
            return;

        ref MZSS_NotificationsConfig cfg = new MZSS_NotificationsConfig();
        JsonFileLoader<MZSS_NotificationsConfig>.JsonLoadFile(path, cfg);
        if (!cfg)
            return;

        m_NotificationsEnabled = cfg.enabled;
        m_NotificationDefaultCooldown = cfg.defaultCooldown;
        if (m_NotificationDefaultCooldown <= 0)
            m_NotificationDefaultCooldown = 900.0;

        if (cfg.notifications)
        {
            for (int i = 0; i < cfg.notifications.Count(); i++)
            {
                ref MZSS_NotificationEntry entry = cfg.notifications.Get(i);
                if (!entry || entry.zoneName == "")
                    continue;

                m_Notifications.Insert(entry);
            }
        }

        LogLoad("Loaded notifications config. Enabled=" + m_NotificationsEnabled.ToString() + " notifications=" + m_Notifications.Count().ToString());
    }

    void LoadWorldState()
    {
        EnsureWorldStateFile();

        m_MigrationStateMap.Clear();
        m_ActiveMigrationEffects.Clear();
        m_NestStateMap.Clear();
        m_NestEntities.Clear();

        string path = GetWorldStatePath();
        if (!FileExist(path))
            return;

        ref MZSS_WorldStateConfig state = new MZSS_WorldStateConfig();
        JsonFileLoader<MZSS_WorldStateConfig>.JsonLoadFile(path, state);
        if (!state)
            return;

        if (state.migrationStates)
        {
            for (int i = 0; i < state.migrationStates.Count(); i++)
            {
                ref MZSS_MigrationLinkState linkState = state.migrationStates.Get(i);
                if (linkState && linkState.linkName != "")
                    m_MigrationStateMap.Set(linkState.linkName, linkState);
            }
        }

        if (state.activeMigrationEffects)
        {
            for (int j = 0; j < state.activeMigrationEffects.Count(); j++)
            {
                ref MZSS_MigrationEffectState effect = state.activeMigrationEffects.Get(j);
                if (effect && effect.remainingDuration > 0)
                    m_ActiveMigrationEffects.Insert(effect);
            }
        }

        if (state.nestStates)
        {
            for (int n = 0; n < state.nestStates.Count(); n++)
            {
                ref MZSS_NestState nestState = state.nestStates.Get(n);
                if (nestState && nestState.nestName != "")
                    m_NestStateMap.Set(nestState.nestName, nestState);
            }
        }

        LogLoad("Loaded world state. Migration cooldowns=" + m_MigrationStateMap.Count().ToString() + " activeEffects=" + m_ActiveMigrationEffects.Count().ToString() + " nestStates=" + m_NestStateMap.Count().ToString());
    }

    void SaveWorldState()
    {
        if (!m_PersistWorldState)
            return;

        ref MZSS_WorldStateConfig state = new MZSS_WorldStateConfig();
        state.lastSavedUnixTime = GetGame().GetTime() / 1000;

        for (int i = 0; i < m_MigrationStateMap.Count(); i++)
        {
            ref MZSS_MigrationLinkState linkState = m_MigrationStateMap.GetElement(i);
            if (linkState)
                state.migrationStates.Insert(linkState);
        }

        for (int e = 0; e < m_ActiveMigrationEffects.Count(); e++)
        {
            ref MZSS_MigrationEffectState effect = m_ActiveMigrationEffects.Get(e);
            if (effect && effect.remainingDuration > 0)
                state.activeMigrationEffects.Insert(effect);
        }

        for (int n = 0; n < m_NestStateMap.Count(); n++)
        {
            ref MZSS_NestState nestState = m_NestStateMap.GetElement(n);
            if (nestState)
                state.nestStates.Insert(nestState);
        }

        JsonFileLoader<MZSS_WorldStateConfig>.JsonSaveFile(GetWorldStatePath(), state);
    }

    bool StringArrayContains(array<string> values, string value)
    {
        if (!values)
            return false;

        for (int i = 0; i < values.Count(); i++)
        {
            if (values.Get(i) == value)
                return true;
        }

        return false;
    }

    ref MZSS_MigrationLinkState GetMigrationState(string linkName)
    {
        ref MZSS_MigrationLinkState state;
        if (m_MigrationStateMap.Find(linkName, state))
            return state;

        state = new MZSS_MigrationLinkState();
        state.linkName = linkName;
        m_MigrationStateMap.Set(linkName, state);
        return state;
    }

    ref MZSS_NestState GetNestState(string nestName)
    {
        ref MZSS_NestState state;
        if (m_NestStateMap.Find(nestName, state))
            return state;

        state = new MZSS_NestState();
        state.nestName = nestName;
        m_NestStateMap.Set(nestName, state);
        return state;
    }

    ref array<EntityAI> GetNestEntityList(string nestName)
    {
        ref array<EntityAI> list;
        if (m_NestEntities.Find(nestName, list))
            return list;

        list = new array<EntityAI>;
        m_NestEntities.Set(nestName, list);
        return list;
    }

    float GetEffectiveSpawnChance(ref SimpleZone zone)
    {
        float chance = zone.spawnChance;

        if (m_ActiveMigrationEffects)
        {
            for (int i = 0; i < m_ActiveMigrationEffects.Count(); i++)
            {
                ref MZSS_MigrationEffectState effect = m_ActiveMigrationEffects.Get(i);
                if (!effect || effect.remainingDuration <= 0)
                    continue;

                if (effect.targetZoneName == zone.name)
                    chance += effect.spawnChanceBonus;
            }
        }

        if (chance < 0.0)
            chance = 0.0;

        if (chance > 1.0)
            chance = 1.0;

        return chance;
    }

    void ProcessMigrationTrigger(string trigger, ref SimpleZone sourceZone)
    {
        if (!m_MigrationEnabled || !sourceZone || !m_MigrationLinks)
            return;

        for (int i = 0; i < m_MigrationLinks.Count(); i++)
        {
            ref MZSS_MigrationLinkConfig link = m_MigrationLinks.Get(i);
            if (!link || !link.enabled)
                continue;

            if (link.trigger != trigger)
                continue;

            if (!StringArrayContains(link.sourceZoneNames, sourceZone.name))
                continue;

            ref MZSS_MigrationLinkState state = GetMigrationState(link.name);
            if (state && state.cooldownRemaining > 0)
                continue;

            if (state)
                state.cooldownRemaining = link.cooldown;

            float roll = Math.RandomFloat(0.0, 1.0);
            if (roll > link.chance)
            {
                LogDiagnostics("Migration link '" + link.name + "' rolled no effect.");
                continue;
            }

            TriggerMigrationLink(link);
        }

        SaveWorldState();
    }

    void TriggerMigrationLink(ref MZSS_MigrationLinkConfig link)
    {
        if (!link || !link.targetZoneNames || link.targetZoneNames.Count() == 0)
            return;

        if (link.targetSelection == "all")
        {
            for (int i = 0; i < link.targetZoneNames.Count(); i++)
            {
                ApplyMigrationEffect(link, link.targetZoneNames.Get(i));
            }
        }
        else
        {
            int idx = Math.RandomInt(0, link.targetZoneNames.Count());
            ApplyMigrationEffect(link, link.targetZoneNames.Get(idx));
        }

        ref MZSS_MigrationLinkState state = GetMigrationState(link.name);
        if (state)
        {
            state.cooldownRemaining = link.cooldown;
            state.lastTriggeredUnixTime = GetGame().GetTime() / 1000;
        }

        LogDiagnostics("Migration link triggered: " + link.name);
    }

    void ApplyMigrationEffect(ref MZSS_MigrationLinkConfig link, string targetZoneName)
    {
        ref SimpleZone targetZone;
        if (!m_ZonesMap.Find(targetZoneName, targetZone) || !targetZone)
        {
            Print("[MZSS:MIGRATION] WARNING: Target zone '" + targetZoneName + "' does not exist.");
            return;
        }

        ref MZSS_MigrationEffectState effect = new MZSS_MigrationEffectState();
        effect.effectId = link.name + "__" + targetZoneName;
        effect.linkName = link.name;
        effect.targetZoneName = targetZoneName;
        effect.remainingDuration = link.effectDuration;
        effect.spawnChanceBonus = link.targetSpawnChanceBonus;
        effect.cooldownReductionApplied = link.targetCooldownReduction;
        effect.forceTargetReadyApplied = link.forceTargetReady;

        if (link.targetCooldownReduction > 0)
        {
            targetZone.cooldownTime = targetZone.cooldownTime - link.targetCooldownReduction;
            if (targetZone.cooldownTime < 0)
                targetZone.cooldownTime = 0;
        }

        if (link.forceTargetReady && !targetZone.hasSpawned)
        {
            targetZone.cooldownTime = 0;
            targetZone.hasRolledChance = false;
        }

        m_ActiveMigrationEffects.Insert(effect);

        Print("[MZSS:MIGRATION] Activity migrated via '" + link.name + "' to target zone '" + targetZoneName + "'.");
    }

    void TickMigrationEffects(float timeslice)
    {
        for (int i = 0; i < m_MigrationStateMap.Count(); i++)
        {
            ref MZSS_MigrationLinkState state = m_MigrationStateMap.GetElement(i);
            if (state && state.cooldownRemaining > 0)
            {
                state.cooldownRemaining -= timeslice;
                if (state.cooldownRemaining < 0)
                    state.cooldownRemaining = 0;
            }
        }

        for (int e = m_ActiveMigrationEffects.Count() - 1; e >= 0; e--)
        {
            ref MZSS_MigrationEffectState effect = m_ActiveMigrationEffects.Get(e);
            if (!effect)
            {
                m_ActiveMigrationEffects.Remove(e);
                continue;
            }

            effect.remainingDuration -= timeslice;
            if (effect.remainingDuration <= 0)
            {
                LogDiagnostics("Migration effect expired: " + effect.effectId);
                m_ActiveMigrationEffects.Remove(e);
            }
        }
    }

    bool IsPlayerNearPosition(vector position, float radius)
    {
        float radiusSq = radius * radius;

        for (int i = 0; i < m_CachedPlayers.Count(); i++)
        {
            Man player = m_CachedPlayers.Get(i);
            if (!player || !player.IsAlive())
                continue;

            if (!CanPlayerTriggerZones(player))
                continue;

            if (vector.DistanceSq(player.GetPosition(), position) <= radiusSq)
                return true;
        }

        return false;
    }

    void TickNests(float timeslice)
    {
        if (!m_NestsEnabled || !m_Nests)
            return;

        if (m_CachedPlayers.Count() == 0 || m_PlayerCacheTimer >= m_PlayerCacheDuration)
            RefreshPlayerCache();

        for (int i = 0; i < m_Nests.Count(); i++)
        {
            ref MZSS_NestConfig nest = m_Nests.Get(i);
            if (!nest || !nest.enabled)
                continue;

            ref MZSS_NestState state = GetNestState(nest.name);
            if (!state)
                continue;

            if (state.cooldownRemaining > 0)
            {
                state.cooldownRemaining -= timeslice;
                if (state.cooldownRemaining < 0)
                    state.cooldownRemaining = 0;
            }

            if (state.state == "active")
            {
                state.activeDurationRemaining -= timeslice;
                CleanupNestEntities(nest.name);
                state.spawnedAlive = CountNestAlive(nest.name);

                bool timerExpired = state.activeDurationRemaining <= 0;
                bool allKilled = state.spawnedAlive <= 0;

                if (nest.clearMode == "timer" && timerExpired)
                    ClearNest(nest, true);
                else if (nest.clearMode == "killAllSpawned" && allKilled)
                    ClearNest(nest, false);
                else if (nest.clearMode == "killAllSpawnedOrTimer" && (timerExpired || allKilled))
                    ClearNest(nest, timerExpired);

                continue;
            }

            if (state.cooldownRemaining > 0)
            {
                state.state = "cooldown";
                continue;
            }

            state.state = "inactive";

            if (nest.triggerMode == "playerNear")
            {
                vector nestPos = nest.position.ToVector();
                if (IsPlayerNearPosition(nestPos, nest.radius))
                {
                    float roll = Math.RandomFloat(0.0, 1.0);
                    if (roll <= nest.activationChance)
                    {
                        if (m_DiagnosticMode)
                            Print("[MZSS:NEST] Triggered nest '" + nest.name + "' roll=" + roll.ToString() + " chance=" + nest.activationChance.ToString());
                        ActivateNest(nest);
                    }
                }
            }
        }
    }

    bool CanSendNestNotification(Man player, ref MZSS_NestConfig nest)
    {
        if (!player || !nest || !nest.notificationEnabled)
            return false;

        float cooldown = nest.notificationCooldown;
        if (cooldown <= 0)
            cooldown = 900.0;

        string steamId = GetPlayerSteamId(player);
        if (steamId == "")
            steamId = player.ToString();

        string key = steamId + "|NEST|" + nest.name;

        float nextAllowed;
        if (m_NotificationCooldowns.Find(key, nextAllowed))
        {
            if (m_ElapsedTime < nextAllowed)
                return false;
        }

        m_NotificationCooldowns.Set(key, m_ElapsedTime + cooldown);
        return true;
    }

    void SendNestNotification(ref MZSS_NestConfig nest)
    {
        if (!nest || !nest.notificationEnabled)
            return;

        vector nestPos = nest.position.ToVector();
        float radiusSq = nest.radius * nest.radius;

        for (int i = 0; i < m_CachedPlayers.Count(); i++)
        {
            Man player = m_CachedPlayers.Get(i);
            if (!player || !player.IsAlive())
                continue;

            if (vector.DistanceSq(player.GetPosition(), nestPos) > radiusSq)
                continue;

            if (!CanSendNestNotification(player, nest))
                continue;

            PlayerBase playerBase = PlayerBase.Cast(player);
            if (!playerBase)
                continue;

            string text = nest.notificationMessage;
            if (nest.notificationTitle != "")
                text = nest.notificationTitle + ": " + nest.notificationMessage;

            playerBase.MessageImportant(text);
        }
    }

    void ActivateNest(ref MZSS_NestConfig nest)
    {
        if (!nest || !nest.spawnClassnames || nest.spawnClassnames.Count() == 0)
        {
            Print("[MZSS:NEST] WARNING: Nest activation failed. Missing spawnClassnames.");
            return;
        }

        ref MZSS_NestState state = GetNestState(nest.name);
        if (!state)
            return;

        state.state = "active";
        state.cooldownRemaining = 0;
        state.activeDurationRemaining = nest.activeDuration;
        state.lastActivatedUnixTime = GetGame().GetTime() / 1000;
        state.spawnedAlive = 0;
        state.spawnedTotal = 0;

        int minCount = nest.minEntities;
        int maxCount = nest.maxEntities;

        if (minCount < 1)
            minCount = 1;

        if (maxCount < minCount)
            maxCount = minCount;

        int count = Math.RandomInt(minCount, maxCount + 1);

        for (int i = 0; i < count; i++)
        {
            if (SpawnNestEntity(nest))
                state.spawnedTotal++;
        }

        state.spawnedAlive = CountNestAlive(nest.name);

        if (state.spawnedTotal <= 0)
        {
            // Do not put the nest into its full configured cooldown if spawning failed.
            // This usually means wrong classname, blocked placement, or impossible position.
            state.state = "inactive";
            state.cooldownRemaining = 30.0;
            state.activeDurationRemaining = 0.0;
            Print("[MZSS:NEST] WARNING: Nest '" + nest.name + "' tried to activate but spawned 0 entities. Check spawnClassnames, position, useFixedHeight and MZSS_AntiClipping.json. Short retry cooldown: 30s.");
            SaveWorldState();
            return;
        }

        SendNestNotification(nest);

        Print("[MZSS:NEST] Activated nest '" + nest.name + "' spawned=" + state.spawnedTotal.ToString() + " alive=" + state.spawnedAlive.ToString());
        SaveWorldState();
    }

    bool SpawnNestEntity(ref MZSS_NestConfig nest)
    {
        if (!nest || !nest.spawnClassnames || nest.spawnClassnames.Count() == 0)
            return false;

        int classIndex = Math.RandomInt(0, nest.spawnClassnames.Count());
        string entityClass = nest.spawnClassnames.Get(classIndex);
        if (entityClass == "")
            return false;

        vector spawnPos = nest.position.ToVector();

        if (nest.spawnRadius > 0)
        {
            float angle = Math.RandomFloat(0.0, Math.PI2);
            float distance = Math.RandomFloat(0.0, nest.spawnRadius);
            spawnPos[0] = spawnPos[0] + (Math.Cos(angle) * distance);
            spawnPos[2] = spawnPos[2] + (Math.Sin(angle) * distance);
        }

        if (nest.useFixedHeight)
            spawnPos[1] = spawnPos[1] + 0.5;
        else
            spawnPos[1] = GetGame().SurfaceY(spawnPos[0], spawnPos[2]) + 0.5;

        ref SimpleSpawnPoint dummyPoint = new SimpleSpawnPoint();
        dummyPoint.position = nest.position.ToVector();
        dummyPoint.radius = nest.spawnRadius;
        dummyPoint.useFixedHeight = nest.useFixedHeight;

        vector finalSpawnPos;
        if (!ResolveAntiClippingSpawnPosition(dummyPoint, spawnPos, finalSpawnPos))
        {
            Print("[MZSS:NEST] WARNING: Anti-clipping rejected spawn position for nest '" + nest.name + "' at " + spawnPos.ToString());
            return false;
        }

        spawnPos = finalSpawnPos;

        EntityAI entity = CreateMZSSSpawnedEntity(entityClass, spawnPos, "NEST");
        if (!entity)
        {
            Print("[MZSS:NEST] WARNING: Failed to create '" + entityClass + "' in nest '" + nest.name + "'.");
            return false;
        }

        entity.SetHealth("", "", entity.GetMaxHealth("", ""));

        ref array<EntityAI> list = GetNestEntityList(nest.name);
        list.Insert(entity);

        if (m_GlobalSettings.entityLifetime > 0)
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CleanupNestEntity, m_GlobalSettings.entityLifetime * 1000, false, entity);

        return true;
    }

    void CleanupNestEntities(string nestName)
    {
        ref array<EntityAI> list = GetNestEntityList(nestName);

        for (int i = list.Count() - 1; i >= 0; i--)
        {
            EntityAI entity = list.Get(i);
            if (!entity || !entity.IsAlive())
                list.Remove(i);
        }
    }

    int CountNestAlive(string nestName)
    {
        CleanupNestEntities(nestName);

        ref array<EntityAI> list = GetNestEntityList(nestName);
        return list.Count();
    }

    void CleanupNestEntity(EntityAI entity)
    {
        if (!entity)
            return;

        for (int i = 0; i < m_NestEntities.Count(); i++)
        {
            ref array<EntityAI> list = m_NestEntities.GetElement(i);
            if (!list)
                continue;

            int idx = list.Find(entity);
            if (idx != -1)
                list.Remove(idx);
        }

        entity.Delete();
    }

    void DespawnNestEntities(string nestName)
    {
        ref array<EntityAI> list = GetNestEntityList(nestName);

        for (int i = list.Count() - 1; i >= 0; i--)
        {
            EntityAI entity = list.Get(i);
            if (entity && entity.IsAlive())
                entity.Delete();

            list.Remove(i);
        }
    }

    void ClearNest(ref MZSS_NestConfig nest, bool despawnLiving)
    {
        if (!nest)
            return;

        if (despawnLiving)
            DespawnNestEntities(nest.name);

        ref MZSS_NestState state = GetNestState(nest.name);
        if (!state)
            return;

        state.state = "cooldown";
        state.cooldownRemaining = nest.cooldown;
        state.activeDurationRemaining = 0;
        state.lastClearedUnixTime = GetGame().GetTime() / 1000;
        state.spawnedAlive = 0;

        Print("[MZSS:NEST] Cleared nest '" + nest.name + "' cooldown=" + nest.cooldown.ToString());
        SaveWorldState();
    }


    void BuildSpatialGrid()
    {
        for (int i = 0; i < m_ZonesMap.Count(); i++)
        {
            ref SimpleZone zone = m_ZonesMap.GetElement(i);
            
            if (!zone || !zone.enabled)
                continue;
            
            float radius = zone.triggerRadius + zone.despawnDistance;
            int minX = Math.Floor((zone.position[0] - radius) / GRID_SIZE);
            int maxX = Math.Floor((zone.position[0] + radius) / GRID_SIZE);
            int minZ = Math.Floor((zone.position[2] - radius) / GRID_SIZE);
            int maxZ = Math.Floor((zone.position[2] + radius) / GRID_SIZE);
            
            for (int x = minX; x <= maxX; x++)
            {
                for (int z = minZ; z <= maxZ; z++)
                {
                    int gridKey = (x * 10000) + z;
                    
                    ref array<ref SimpleZone> cellZones;
                    if (!m_ZoneGrid.Find(gridKey, cellZones))
                    {
                        cellZones = new array<ref SimpleZone>;
                        m_ZoneGrid.Set(gridKey, cellZones);
                    }
                    cellZones.Insert(zone);
                }
            }
        }
        
        LogLoad("Built spatial grid with " + m_ZoneGrid.Count().ToString() + " cells");
    }
    
    void Update(float timeslice)
    {
        if (!m_SystemEnabled) return;
        
        m_CheckTimer += timeslice;
        m_PlayerCacheTimer += timeslice;
        m_ElapsedTime += timeslice;

        if (m_PersistCooldowns)
        {
            m_CooldownSaveTimer += timeslice;
            if (m_CooldownSaveTimer >= m_CooldownPersistenceInterval)
            {
                m_CooldownSaveTimer = 0.0;
                SaveCooldowns();
            }
        }

        if (m_PersistWorldState)
        {
            m_WorldStateSaveTimer += timeslice;
            if (m_WorldStateSaveTimer >= m_WorldStateSaveInterval)
            {
                m_WorldStateSaveTimer = 0.0;
                SaveWorldState();
            }
        }

        TickMigrationEffects(timeslice);
        TickNests(timeslice);

        m_AdminCommandPollTimer += timeslice;
        if (m_AdminCommandPollTimer >= 1.0)
        {
            m_AdminCommandPollTimer = 0.0;
            CheckAdminCommandFile();
            ApplyPlayerRulesToSpawnedAI();
        }
        
        for (int i = 0; i < m_ZonesMap.Count(); i++)
        {
            ref SimpleZone zone = m_ZonesMap.GetElement(i);
            if (zone && zone.cooldownTime > 0)
            {
                zone.cooldownTime -= timeslice;
                if (zone.cooldownTime <= 0)
                {
                    zone.cooldownTime = 0;
                    zone.hasSpawned = false;
                    zone.hasRolledChance = false;
                }
            }
        }
        
        // Tick Soft Spawn Zonen (zeitversetztes Spawnen)
        TickSoftSpawnZones(timeslice);
        
        // Drain Burst Queue (globales Spawn Budget)
        DrainSpawnQueue();
        
        if (m_CheckTimer >= m_CheckInterval)
        {
            m_CheckTimer = 0.0;
            CheckPlayersOptimized();
        }
    }
    
    // Tick alle Zonen in der Soft Spawn Liste, spawne einzelne Entities zeitversetzt
    void TickSoftSpawnZones(float timeslice)
    {
        for (int i = m_SoftSpawnZones.Count() - 1; i >= 0; i--)
        {
            ref SimpleZone zone = m_SoftSpawnZones.Get(i);
            if (!zone)
            {
                m_SoftSpawnZones.Remove(i);
                continue;
            }
            
            zone.softSpawnTimer -= timeslice;
            if (zone.softSpawnTimer > 0)
                continue;
            
            // Timer abgelaufen – nächste Entity aus dem SpawnQueue für diese Zone spawnen
            bool spawnedOne = false;
            for (int q = 0; q < m_SpawnQueueZones.Count(); q++)
            {
                if (m_SpawnQueueZones.Get(q) == zone)
                {
                    ref SimpleSpawnPoint point = m_SpawnQueuePoints.Get(q);
                    m_SpawnQueuePoints.Remove(q);
                    m_SpawnQueueZones.Remove(q);
                    
                    if (point && SpawnEntityAtPoint(zone, point))
                    {
                        zone.hasSpawned = true;
                        m_DirtyZones.Insert(zone);
                        if (m_DebugMode) Print("[MZSS] Soft Spawn processed from queue: zone=" + zone.name);
                    }
                    spawnedOne = true;
                    break;
                }
            }
            
            if (spawnedOne)
            {
                // Prüfen ob noch weitere Requests für diese Zone in der Queue
                bool moreInQueue = false;
                for (int q2 = 0; q2 < m_SpawnQueueZones.Count(); q2++)
                {
                    if (m_SpawnQueueZones.Get(q2) == zone)
                    {
                        moreInQueue = true;
                        break;
                    }
                }
                
                if (moreInQueue)
                {
                    zone.softSpawnTimer = m_SoftSpawnInterval;
                }
                else
                {
                    // Keine weiteren Requests – Zone aus Soft-Spawn-Liste entfernen
                    m_SoftSpawnZones.Remove(i);
                }
            }
            else
            {
                // Keine Requests mehr für diese Zone
                m_SoftSpawnZones.Remove(i);
            }
        }
    }
    
    // Process up to m_MaxSpawnsPerTick queued spawn requests (Burst-Schutz)
    void DrainSpawnQueue()
    {
        int budget = m_MaxSpawnsPerTick;
        int queueSize = m_SpawnQueuePoints.Count();
        
        if (queueSize == 0)
            return;
        
        if (m_DebugMode && queueSize > m_MaxSpawnsPerTick)
            Print("[MZSS] Spawn queued (budget reached): " + queueSize.ToString() + " pending, processing " + budget.ToString() + " this tick");
        
        while (budget > 0 && m_SpawnQueuePoints.Count() > 0)
        {
            ref SimpleSpawnPoint point = m_SpawnQueuePoints.Get(0);
            ref SimpleZone zone       = m_SpawnQueueZones.Get(0);
            m_SpawnQueuePoints.Remove(0);
            m_SpawnQueueZones.Remove(0);
            
            if (!zone || !point)
                continue;
            
            if (SpawnEntityAtPoint(zone, point))
            {
                zone.hasSpawned = true;
                m_DirtyZones.Insert(zone);
                budget--;
                
                if (m_DebugMode) Print("[MZSS] Spawn processed from queue: zone=" + zone.name);
            }
        }
    }
    
    string BuildNotificationCooldownKey(Man player, string zoneName, string title)
    {
        string steamId = GetPlayerSteamId(player);
        if (steamId == "")
            steamId = player.ToString();

        return steamId + "|" + zoneName + "|" + title;
    }

    bool CanSendNotification(Man player, ref MZSS_NotificationEntry notification)
    {
        if (!player || !notification)
            return false;

        float cooldown = notification.cooldown;
        if (cooldown <= 0)
            cooldown = m_NotificationDefaultCooldown;

        string key = BuildNotificationCooldownKey(player, notification.zoneName, notification.title);

        float nextAllowed;
        if (m_NotificationCooldowns.Find(key, nextAllowed))
        {
            if (m_ElapsedTime < nextAllowed)
                return false;
        }

        m_NotificationCooldowns.Set(key, m_ElapsedTime + cooldown);
        return true;
    }

    void SendPersonalNotification(Man player, ref MZSS_NotificationEntry notification)
    {
        if (!player || !notification)
            return;

        PlayerBase playerBase = PlayerBase.Cast(player);
        if (!playerBase)
            return;

        string text = notification.message;
        if (notification.title != "")
            text = notification.title + ": " + notification.message;

        // Personal message only. This is not broadcast to global chat.
        // MessageImportant is used because it is more visible than a normal status message.
        playerBase.MessageImportant(text);
    }

    void ProcessZoneNotifications(ref SimpleZone zone)
    {
        if (!m_NotificationsEnabled || !zone || !zone.playersInside)
            return;

        if (!m_Notifications || m_Notifications.Count() == 0)
            return;

        for (int n = 0; n < m_Notifications.Count(); n++)
        {
            ref MZSS_NotificationEntry notification = m_Notifications.Get(n);
            if (!notification || !notification.enabled)
                continue;

            if (notification.zoneName != zone.name)
                continue;

            if (notification.trigger != "onEnter")
                continue;

            for (int p = 0; p < zone.playersInside.Count(); p++)
            {
                Man player = zone.playersInside.Get(p);
                if (!player || !player.IsAlive())
                    continue;

                if (CanSendNotification(player, notification))
                    SendPersonalNotification(player, notification);
            }
        }
    }

    string GetPlayerSteamId(Man player)
    {
        PlayerBase playerBase = PlayerBase.Cast(player);
        if (!playerBase)
            return "";

        PlayerIdentity identity = playerBase.GetIdentity();
        if (!identity)
            return "";

        return identity.GetPlainId();
    }

    ref MZSS_PlayerRuleEntry GetPlayerRule(Man player)
    {
        if (!m_PlayerRulesEnabled || !player)
            return null;

        string steamId = GetPlayerSteamId(player);
        if (steamId == "")
            return null;

        ref MZSS_PlayerRuleEntry rule;
        if (m_PlayerRules.Find(steamId, rule))
            return rule;

        return null;
    }

    bool CanPlayerTriggerZones(Man player)
    {
        ref MZSS_PlayerRuleEntry rule = GetPlayerRule(player);
        if (!rule)
            return true;

        return rule.canTriggerZones;
    }

    bool IsPlayerIgnoredBySpawnedAI(Man player)
    {
        ref MZSS_PlayerRuleEntry rule = GetPlayerRule(player);
        if (!rule)
            return false;

        return rule.ignoredBySpawnedAI;
    }

    void ApplyPlayerRulesToSpawnedAI()
    {
        if (!m_PlayerRulesEnabled || !m_HasIgnoredBySpawnedAIRules)
            return;

        if (!m_CachedPlayers || m_CachedPlayers.Count() == 0)
            return;

        for (int i = 0; i < m_ZonesMap.Count(); i++)
        {
            ref SimpleZone zone = m_ZonesMap.GetElement(i);
            if (!zone || !zone.spawnPoints)
                continue;

            for (int sp = 0; sp < zone.spawnPoints.Count(); sp++)
            {
                ref SimpleSpawnPoint point = zone.spawnPoints.Get(sp);
                if (!point || !point.spawnedEntities)
                    continue;

                for (int e = 0; e < point.spawnedEntities.Count(); e++)
                {
                    EntityAI entity = point.spawnedEntities.Get(e);
                    if (!entity || !entity.IsAlive())
                        continue;

                    for (int p = 0; p < m_CachedPlayers.Count(); p++)
                    {
                        Man player = m_CachedPlayers.Get(p);
                        if (!player || !player.IsAlive())
                            continue;

                        if (!IsPlayerIgnoredBySpawnedAI(player))
                            continue;

                        // Best-effort target clearing for MZSS-spawned AI only.
                        // Different AI classes/mods expose different target APIs. CallFunction keeps this optional.
                        GetGame().GameScript.CallFunction(entity, "RemoveTarget", null, new Param1<Object>(player));
                        GetGame().GameScript.CallFunction(entity, "OnTargetLost", null, new Param1<Object>(player));
                    }
                }
            }
        }
    }

    void RefreshPlayerCache()
    {
        m_CachedPlayers.Clear();
        GetGame().GetPlayers(m_CachedPlayers);
        
        CleanupPlayerCaches();
        
        m_PlayerCacheTimer = 0.0;
        
        if (m_DebugMode) Print("[MZSS] Player cache refreshed: " + m_CachedPlayers.Count().ToString() + " players");
    }
    
    void CleanupPlayerCaches()
    {
        ref array<Man> validPlayers = new array<Man>;
        
        for (int i = 0; i < m_CachedPlayers.Count(); i++)
        {
            Man player = m_CachedPlayers.Get(i);
            if (player && player.IsAlive())
            {
                validPlayers.Insert(player);
            }
            else
            {
                m_PlayerGridCache.Remove(player);
                m_PlayerLastPos.Remove(player);
            }
        }
    }
    
    int GetPlayerGridKey(Man player, vector playerPos)
    {
        int cachedKey;
        if (m_PlayerGridCache.Find(player, cachedKey))
        {
            vector lastPos;
            if (m_PlayerLastPos.Find(player, lastPos))
            {
                // Skip full grid recompute if player hasn't moved past the threshold
                float moveSq = vector.DistanceSq(playerPos, lastPos);
                if (moveSq < m_PlayerMoveThresholdSq)
                    return cachedKey;
            }
        }
        
        int gridX = Math.Floor(playerPos[0] / GRID_SIZE);
        int gridZ = Math.Floor(playerPos[2] / GRID_SIZE);
        int newKey = (gridX * 10000) + gridZ;
        
        m_PlayerGridCache.Set(player, newKey);
        m_PlayerLastPos.Set(player, playerPos);
        
        return newKey;
    }
    
    void CheckPlayersOptimized()
    {
        if (m_Tiers.Count() == 0 || m_ZonesMap.Count() == 0)
            return;
        
        if (m_PlayerCacheTimer >= m_PlayerCacheDuration || m_CachedPlayers.Count() == 0)
        {
            RefreshPlayerCache();
        }
        
        if (m_CachedPlayers.Count() == 0)
            return;
        
        ref set<ref SimpleZone> checkedZones = new set<ref SimpleZone>;
        
        for (int p = 0; p < m_CachedPlayers.Count(); p++)
        {
            Man player = m_CachedPlayers.Get(p);
            
            if (!player || !player.IsAlive())
                continue;
            
            if (!CanPlayerTriggerZones(player))
                continue;
            
            vector playerPos = player.GetPosition();
            
            int centerGridKey = GetPlayerGridKey(player, playerPos);
            int gridX = Math.Floor(playerPos[0] / GRID_SIZE);
            int gridZ = Math.Floor(playerPos[2] / GRID_SIZE);
            
            for (int dx = -1; dx <= 1; dx++)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    int gridKey = ((gridX + dx) * 10000) + (gridZ + dz);
                    
                    ref array<ref SimpleZone> cellZones;
                    if (m_ZoneGrid.Find(gridKey, cellZones))
                    {
                        for (int z = 0; z < cellZones.Count(); z++)
                        {
                            ref SimpleZone zone = cellZones.Get(z);
                            
                            if (checkedZones.Find(zone) != -1)
                                continue;
                            
                            checkedZones.Insert(zone);
                            
                            if (!zone || !zone.enabled)
                                continue;
                            
                            CheckZone(zone, m_CachedPlayers);
                        }
                    }
                }
            }
        }
    }
    
    void CheckZone(ref SimpleZone zone, array<Man> players)
    {
        // Reuse a single local array rather than allocating a new one every call
        zone.playersInside.Clear();
        bool anyPlayerNearby = false;
        
        for (int i = 0; i < players.Count(); i++)
        {
            Man player = players.Get(i);
            
            if (!player || !player.IsAlive())
                continue;
            
            if (!CanPlayerTriggerZones(player))
                continue;
            
            float distSq = vector.DistanceSq(player.GetPosition(), zone.position);
            
            // Player is within despawn distance – counts as "nearby"
            if (distSq <= zone.despawnDistanceSq)
                anyPlayerNearby = true;
            
            if (distSq <= zone.triggerRadiusSq)
            {
                zone.playersInside.Insert(player);
            }
        }
        
        bool hasPlayersInside = (zone.playersInside.Count() > 0);

        // Extra despawn protection:
        // Zone-center checks are not enough once spawned mutants/zombies chase players
        // away from the zone center. If a living spawned entity is still close to a player,
        // keep the zone alive and reset the empty timer.
        if (!anyPlayerNearby && m_ProtectLivingEntitiesFromDespawn && zone.hasSpawned)
        {
            if (HasPlayerNearLivingSpawnedEntity(zone, players))
                anyPlayerNearby = true;
        }

        if (hasPlayersInside || anyPlayerNearby)
        {
            // Spieler in der Nähe – Grace Timer und emptySince zurücksetzen
            zone.lastPlayerSeenTime = m_ElapsedTime;
            zone.emptySince = -1.0;
        }
        if (hasPlayersInside)
        {
            if (m_DebugMode && !zone.hasSpawned)
                Print("[MZSS] Zone Activated: " + zone.name + " (" + zone.playersInside.Count().ToString() + " players)");
            
            if (!zone.hasSpawned && !zone.hasRolledChance)
                ProcessMigrationTrigger("sourceActivated", zone);
            
            ProcessZoneNotifications(zone);
            TrySpawnInZone(zone, zone.playersInside);
        }
        else if (zone.hasSpawned && zone.despawnOnExit)
        {
            // Zone ist leer – emptySince setzen wenn noch nicht gesetzt
            if (zone.emptySince < 0)
                zone.emptySince = m_ElapsedTime;
            
            float emptyDuration = m_ElapsedTime - zone.emptySince;
            
            if (emptyDuration >= m_DespawnDelay)
            {
                if (m_DebugMode) Print("[MZSS] Zone Despawned: " + zone.name + " (empty for " + emptyDuration.ToString() + "s)");
                DespawnFromZone(zone);
                zone.emptySince = -1.0;
            }
        }
    }
    
    bool HasPlayerNearLivingSpawnedEntity(ref SimpleZone zone, array<Man> players)
    {
        if (!zone || !zone.spawnPoints || !players)
            return false;

        for (int sp = 0; sp < zone.spawnPoints.Count(); sp++)
        {
            ref SimpleSpawnPoint point = zone.spawnPoints.Get(sp);
            if (!point || !point.spawnedEntities)
                continue;

            for (int e = 0; e < point.spawnedEntities.Count(); e++)
            {
                EntityAI entity = point.spawnedEntities.Get(e);
                if (!entity || !entity.IsAlive())
                    continue;

                vector entityPos = entity.GetPosition();

                for (int p = 0; p < players.Count(); p++)
                {
                    Man player = players.Get(p);
                    if (!player || !player.IsAlive())
                        continue;

                    if (!CanPlayerTriggerZones(player))
                        continue;

                    if (vector.DistanceSq(player.GetPosition(), entityPos) <= m_LivingEntityDespawnProtectionDistanceSq)
                    {
                        if (m_DiagnosticMode || (m_DebugMode && m_LogDespawn))
                            Print("[MZSS:DESPAWN] Despawn blocked for zone '" + zone.name + "': living spawned entity is near a player.");

                        return true;
                    }
                }
            }
        }

        return false;
    }

    void TrySpawnInZone(ref SimpleZone zone, array<Man> playersInZone)
    {
        if (zone.cooldownTime > 0)
            return;
        
        if (zone.hasSpawned)
            return;
        
        if (!zone.hasRolledChance)
        {
            float roll = Math.RandomFloat(0.0, 1.0);
            zone.hasRolledChance = true;
            
            if (roll > zone.spawnChance)
            {
                if (m_DebugMode) Print("[MZSS] Zone " + zone.name + " failed spawn chance (" + (zone.spawnChance * 100).ToString() + "%)");
                zone.cooldownTime = zone.respawnCooldown;
                return;
            }
        }
        
        if (!zone.spawnPoints || zone.spawnPoints.Count() == 0)
            return;
        
        CleanupSpawnPointEntities(zone);
        
        if (CountZoneEntities(zone) > 0)
            return;
        
        int queued = 0;
        
        for (int i = 0; i < zone.spawnPoints.Count(); i++)
        {
            ref SimpleSpawnPoint point = zone.spawnPoints.Get(i);
            
            if (!IsSpawnPointSafe(point, playersInZone))
                continue;
            
            for (int j = 0; j < point.entities; j++)
            {
                m_SpawnQueuePoints.Insert(point);
                m_SpawnQueueZones.Insert(zone);
                queued++;
            }
        }
        
        if (queued > 0)
        {
            // Zone für Soft Spawn registrieren (zeitverteiltes Drain)
            if (m_SoftSpawnZones.Find(zone) == -1)
            {
                zone.softSpawnTimer = 0.0; // sofort starten
                m_SoftSpawnZones.Insert(zone);
            }
            
            if (m_DebugMode) Print("[MZSS] Zone Spawned (queued): " + zone.name + " – " + queued.ToString() + " requests, queue total: " + m_SpawnQueuePoints.Count().ToString());
        }
    }
    
    bool IsSpawnPointSafe(ref SimpleSpawnPoint point, array<Man> players)
    {
        for (int i = 0; i < players.Count(); i++)
        {
            Man player = players.Get(i);
            if (player && player.IsAlive())
            {
                float distSq = vector.DistanceSq(player.GetPosition(), point.position);
                if (distSq < m_MinSpawnDistanceSq)
                {
                    return false;
                }
            }
        }
        return true;
    }
    
    bool ResolveAntiClippingSpawnPosition(ref SimpleSpawnPoint point, vector requestedPos, out vector resolvedPos)
    {
        resolvedPos = requestedPos;

        if (!m_AntiClippingEnabled)
            return true;

        if (ValidateSpawnPosition(point, requestedPos, resolvedPos))
            return true;

        for (int i = 0; i < m_AntiClippingRetryAttempts; i++)
        {
            vector candidate = requestedPos;

            float angle = Math.RandomFloat(0.0, Math.PI2);
            float distance = Math.RandomFloat(0.0, m_AntiClippingRetryRadius);

            candidate[0] = requestedPos[0] + (Math.Cos(angle) * distance);
            candidate[2] = requestedPos[2] + (Math.Sin(angle) * distance);

            if (point && point.useFixedHeight)
                candidate[1] = requestedPos[1];

            if (ValidateSpawnPosition(point, candidate, resolvedPos))
            {
                if (m_DiagnosticMode || (m_DebugMode && m_LogSpawn))
                    Print("[MZSS:SPAWN] Anti-clipping adjusted spawn position from " + requestedPos.ToString() + " to " + resolvedPos.ToString());

                return true;
            }
        }

        if (m_AntiClippingFallbackToOriginalPosition)
        {
            resolvedPos = requestedPos;
            if (m_DiagnosticMode || (m_DebugMode && m_LogSpawn))
                Print("[MZSS:SPAWN] Anti-clipping fallback used at " + requestedPos.ToString());
            return true;
        }

        if (m_DiagnosticMode || (m_DebugMode && m_LogSpawn))
            Print("[MZSS:SPAWN] Anti-clipping rejected spawn position " + requestedPos.ToString());

        return false;
    }

    bool ValidateSpawnPosition(ref SimpleSpawnPoint point, vector candidate, out vector resolvedPos)
    {
        resolvedPos = candidate;

        if (!point || !point.useFixedHeight)
        {
            if (m_AntiClippingGroundRaycast)
            {
                vector start = candidate;
                start[1] = start[1] + m_AntiClippingVerticalProbeUp;

                vector end = candidate;
                end[1] = end[1] - m_AntiClippingVerticalProbeDown;

                vector hitPos;
                vector hitNormal;
                int hitComponentIndex;

                bool hit = DayZPhysics.RaycastRV(start, end, hitPos, hitNormal, hitComponentIndex, null, null, null);
                if (!hit)
                    return false;

                resolvedPos = hitPos;
                resolvedPos[1] = resolvedPos[1] + 0.5;
            }
        }

        if (m_AntiClippingClearanceCheck)
        {
            if (!HasSpawnClearance(resolvedPos))
                return false;
        }

        return true;
    }

    bool HasSpawnClearance(vector position)
    {
        vector origin = position;
        origin[1] = origin[1] + 0.8;

        for (int i = 0; i < 8; i++)
        {
            float angle = (Math.PI2 / 8.0) * i;
            vector end = origin;
            end[0] = end[0] + (Math.Cos(angle) * m_AntiClippingMinWallDistance);
            end[2] = end[2] + (Math.Sin(angle) * m_AntiClippingMinWallDistance);

            vector hitPos;
            vector hitNormal;
            int hitComponentIndex;

            bool hit = DayZPhysics.RaycastRV(origin, end, hitPos, hitNormal, hitComponentIndex, null, null, null);
            if (hit)
                return false;
        }

        return true;
    }

    // Extension hook for mods/entities that are not directly spawnable via vanilla CreateObject/CreateObjectEx.
    // Other compatibility mods may mod this method and return a valid EntityAI for custom spawn systems.
    EntityAI CreateMZSSExternalSpawnEntity(string entityClass, vector spawnPos, string context)
    {
        return null;
    }

    string ResolveMZSSSpawnAlias(string entityClass, string context)
    {
        // Best-effort resolver for custom config wrappers. Some mods store AI/loadout entries
        // outside CfgVehicles and expose the real spawnable type through a text field.
        ref array<string> roots = new array<string>;
        roots.Insert("CfgMZSSSpawnAliases");
        roots.Insert("CfgMZSSSpawns");
        roots.Insert("CfgAICharacters");
        roots.Insert("CfgAISpawns");
        roots.Insert("CfgExpansionAI");
        roots.Insert("CfgNonAIVehicles");

        ref array<string> fields = new array<string>;
        fields.Insert("spawnClassname");
        fields.Insert("spawnClass");
        fields.Insert("entityClassname");
        fields.Insert("entityClass");
        fields.Insert("className");
        fields.Insert("classname");
        fields.Insert("type");

        for (int r = 0; r < roots.Count(); r++)
        {
            string root = roots.Get(r);
            for (int f = 0; f < fields.Count(); f++)
            {
                string path = root + " " + entityClass + " " + fields.Get(f);
                if (!GetGame().ConfigIsExisting(path))
                    continue;

                string resolved;
                GetGame().ConfigGetText(path, resolved);
                resolved.TrimInPlace();

                if (resolved != "")
                {
                    Print("[MZSS:" + context + "] Resolved custom spawn alias '" + entityClass + "' -> '" + resolved + "' via " + path);
                    return resolved;
                }
            }
        }

        return entityClass;
    }

    EntityAI CreateMZSSSpawnedEntity(string entityClass, vector spawnPos, string context)
    {
        if (entityClass == "")
            return null;

        string requestedClass = entityClass;
        string resolvedClass = ResolveMZSSSpawnAlias(entityClass, context);

        if (resolvedClass != requestedClass)
            entityClass = resolvedClass;

        if (!GetGame().ConfigIsExisting("CfgVehicles " + entityClass))
        {
            Print("[MZSS:" + context + "] INFO: Classname '" + entityClass + "' was not found in CfgVehicles. Trying external hook and vanilla factories anyway.");
        }

        EntityAI externalEntity = CreateMZSSExternalSpawnEntity(requestedClass, spawnPos, context);
        if (!externalEntity && resolvedClass != requestedClass)
            externalEntity = CreateMZSSExternalSpawnEntity(resolvedClass, spawnPos, context);

        if (externalEntity)
            return externalEntity;

        Object obj = GetGame().CreateObject(entityClass, spawnPos, false, true, true);
        EntityAI entity = EntityAI.Cast(obj);

        if (!entity)
        {
            obj = GetGame().CreateObjectEx(entityClass, spawnPos, ECE_PLACE_ON_SURFACE);
            entity = EntityAI.Cast(obj);
        }

        if (!entity && resolvedClass != requestedClass)
        {
            obj = GetGame().CreateObject(requestedClass, spawnPos, false, true, true);
            entity = EntityAI.Cast(obj);

            if (!entity)
            {
                obj = GetGame().CreateObjectEx(requestedClass, spawnPos, ECE_PLACE_ON_SURFACE);
                entity = EntityAI.Cast(obj);
            }
        }

        if (!entity)
        {
            Print("[MZSS:" + context + "] WARNING: Could not create '" + requestedClass + "' at " + spawnPos.ToString() + ". If this classname belongs to a custom AI/loadout system and is not directly spawnable, it needs a spawnable proxy class or a compatibility override for CreateMZSSExternalSpawnEntity().");
            return null;
        }

        return entity;
    }

    bool SpawnEntityAtPoint(ref SimpleZone zone, ref SimpleSpawnPoint point)
    {
        if (point.tierIds.Count() == 0)
            return false;
        
        if (point.spawnedEntities.Count() >= point.entities)
            return false;
        
        if (m_MaxTotalSpawnedEntities > 0 && CountTotalSpawnedEntities() >= m_MaxTotalSpawnedEntities)
        {
            LogSpawn("Global entity limit reached: " + m_MaxTotalSpawnedEntities.ToString());
            return false;
        }

        int tierIndex = Math.RandomInt(0, point.tierIds.Count());
        int tierId = point.tierIds.Get(tierIndex);
        
        ref SimpleTier tier;
        if (!m_Tiers.Find(tierId, tier) || !tier || tier.classnames.Count() == 0)
            return false;
        
        int classIndex = Math.RandomInt(0, tier.classnames.Count());
        string entityClass = tier.classnames.Get(classIndex);
        
        // Letzter Sicherheitscheck – leerer Classname wird übersprungen
        if (entityClass == "")
        {
            Print("[MZSS] WARNUNG: Leerer Classname in Tier " + tierId.ToString() + " – wird übersprungen");
            return false;
        }
        
        vector spawnPos = point.position;
        if (point.radius > 0)
        {
            float angle = Math.RandomFloat(0, Math.PI2);
            float distance = Math.RandomFloat(0, point.radius);
            spawnPos[0] = spawnPos[0] + (Math.Cos(angle) * distance);
            spawnPos[2] = spawnPos[2] + (Math.Sin(angle) * distance);
        }
        
        if (point.useFixedHeight)
        {
            spawnPos[1] = point.position[1] + 0.5;
        }
        else
        {
            spawnPos[1] = GetGame().SurfaceY(spawnPos[0], spawnPos[2]) + 0.5;
        }
        
        vector finalSpawnPos;
        if (!ResolveAntiClippingSpawnPosition(point, spawnPos, finalSpawnPos))
            return false;

        spawnPos = finalSpawnPos;

        EntityAI entity = CreateMZSSSpawnedEntity(entityClass, spawnPos, "SPAWN");
        if (!entity)
            return false;

        entity.SetHealth("", "", entity.GetMaxHealth("", ""));
        point.spawnedEntities.Insert(entity);
        
        if (m_GlobalSettings.entityLifetime > 0)
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CleanupEntity, m_GlobalSettings.entityLifetime * 1000, false, entity);
        }

        LogSpawn("Spawned " + entityClass + " in zone " + zone.name + " at " + spawnPos.ToString());

        return true;
    }
    
    void CleanupSpawnPointEntities(ref SimpleZone zone)
    {
        for (int i = 0; i < zone.spawnPoints.Count(); i++)
        {
            ref SimpleSpawnPoint point = zone.spawnPoints.Get(i);
            
            for (int j = point.spawnedEntities.Count() - 1; j >= 0; j--)
            {
                EntityAI entity = point.spawnedEntities.Get(j);
                if (!entity || !entity.IsAlive())
                {
                    point.spawnedEntities.Remove(j);
                }
            }
        }
    }
    
    int CountZoneEntities(ref SimpleZone zone)
    {
        int count = 0;
        for (int i = 0; i < zone.spawnPoints.Count(); i++)
        {
            ref SimpleSpawnPoint point = zone.spawnPoints.Get(i);
            count += point.spawnedEntities.Count();
        }
        return count;
    }
    
    void DespawnFromZone(ref SimpleZone zone)
    {
        int totalDespawned = 0;
        
        for (int i = 0; i < zone.spawnPoints.Count(); i++)
        {
            ref SimpleSpawnPoint point = zone.spawnPoints.Get(i);
            
            for (int j = point.spawnedEntities.Count() - 1; j >= 0; j--)
            {
                EntityAI entity = point.spawnedEntities.Get(j);
                
                if (!entity)
                {
                    // Null ref – remove the slot
                    point.spawnedEntities.Remove(j);
                    continue;
                }
                
                if (entity.IsAlive())
                {
                    // Only delete living entities; corpses remain for looting
                    entity.Delete();
                    point.spawnedEntities.Remove(j);
                    totalDespawned++;
                }
                // Dead entities stay in the list – entityLifetime callback will clean them up
            }
        }
        
        zone.cooldownTime = zone.respawnCooldown;
        zone.hasSpawned = false;
        zone.hasRolledChance = false;
        
        if (totalDespawned > 0 && m_DebugMode)
            Print("[MZSS] Despawned " + totalDespawned.ToString() + " living entities from zone " + zone.name + " (corpses left in world)");

        ProcessMigrationTrigger("sourceDespawned", zone);
    }
    
    void CleanupDeadEntities()
    {
        ref array<ref SimpleZone> zonesToClean = new array<ref SimpleZone>;
        
        for (int i = 0; i < m_DirtyZones.Count(); i++)
        {
            ref SimpleZone dirtyZone = m_DirtyZones.Get(i);
            if (dirtyZone)
            {
                zonesToClean.Insert(dirtyZone);
            }
        }
        
        for (int j = 0; j < zonesToClean.Count(); j++)
        {
            ref SimpleZone zone = zonesToClean.Get(j);
            
            CleanupSpawnPointEntities(zone);
            
            int entityCount = CountZoneEntities(zone);
            
            if (entityCount == 0 && zone.hasSpawned)
            {
                ProcessMigrationTrigger("sourceCleared", zone);

                zone.cooldownTime = zone.respawnCooldown;
                zone.hasSpawned = false;
                zone.hasRolledChance = false;
                m_DirtyZones.RemoveItem(zone);
                if (m_PersistCooldowns) SaveCooldowns();
            }
        }
        
        if (m_DebugMode && m_DirtyZones.Count() > 0) 
            Print("[MZSS] Lazy cleanup processed " + m_DirtyZones.Count().ToString() + " dirty zones");
    }
    
    void CleanupEntity(EntityAI entity)
    {
        if (!entity)
            return;
        
        // Remove from whichever spawn-point list holds it before deleting
        for (int i = 0; i < m_ZonesMap.Count(); i++)
        {
            ref SimpleZone zone = m_ZonesMap.GetElement(i);
            if (!zone) continue;
            
            for (int j = 0; j < zone.spawnPoints.Count(); j++)
            {
                ref SimpleSpawnPoint point = zone.spawnPoints.Get(j);
                if (!point) continue;
                
                int idx = point.spawnedEntities.Find(entity);
                if (idx != -1)
                {
                    point.spawnedEntities.Remove(idx);
                }
            }
        }
        
        entity.Delete();
    }
    
    string GetAdminCommandPath()
    {
        return "$profile:MutantSpawnSystem/MZSS_AdminCommand.json";
    }

    string GetLegacyAdminCommandPath()
    {
        return "$profile:MutantSpawnSystem/AdminCommand.json";
    }

    void InitAdminCommandState()
    {
        string path = GetAdminCommandPath();
        if (!FileExist(path))
            path = GetLegacyAdminCommandPath();

        if (!FileExist(path))
            return;

        ref MZSS_AdminCommandConfig cmd = new MZSS_AdminCommandConfig();
        JsonFileLoader<MZSS_AdminCommandConfig>.JsonLoadFile(path, cmd);
        if (cmd)
        {
            m_LastAdminCommandId = cmd.commandId;
            Print("[MZSS] Admin command bridge initialized. Last command id: " + m_LastAdminCommandId.ToString());
        }
    }

    void CheckAdminCommandFile()
    {
        string path = GetAdminCommandPath();
        if (!FileExist(path))
            path = GetLegacyAdminCommandPath();

        if (!FileExist(path))
            return;

        ref MZSS_AdminCommandConfig cmd = new MZSS_AdminCommandConfig();
        JsonFileLoader<MZSS_AdminCommandConfig>.JsonLoadFile(path, cmd);
        if (!cmd)
            return;

        if (cmd.commandId <= 0)
            return;

        if (cmd.commandId <= m_LastAdminCommandId)
            return;

        m_LastAdminCommandId = cmd.commandId;

        string action = cmd.action;
        action.ToLower();
        action.TrimInPlace();

        LogAdmin("Admin command received id=" + cmd.commandId.ToString() + " action=" + action + " position=" + cmd.position);

        if (action == "reload" || action == "hot_reload")
        {
            AdminHotReload();
            return;
        }

        if (action == "force_nearest" || action == "force_spawn_nearest" || action == "respawn_nearest")
        {
            string posString = cmd.position;
            posString.Replace(",", " ");
            posString.TrimInPlace();
            vector sourcePos = posString.ToVector();
            ForceSpawnNearestAtPosition(sourcePos);
            return;
        }

        if (action == "diagnose" || action == "dry_run")
        {
            string diagPosString = cmd.position;
            diagPosString.Replace(",", " ");
            diagPosString.TrimInPlace();
            vector diagPos = diagPosString.ToVector();
            DiagnoseAtPosition(diagPos);
            return;
        }

        LogAdmin("Unknown admin command action: " + action);
    }

    void ClearSpawnQueues()
    {
        m_SpawnQueuePoints.Clear();
        m_SpawnQueueZones.Clear();
        m_SoftSpawnZones.Clear();
    }

    void ClearQueuedSpawnsForZone(ref SimpleZone targetZone)
    {
        if (!targetZone)
            return;

        for (int i = m_SpawnQueueZones.Count() - 1; i >= 0; i--)
        {
            if (m_SpawnQueueZones.Get(i) == targetZone)
            {
                m_SpawnQueueZones.Remove(i);
                m_SpawnQueuePoints.Remove(i);
            }
        }

        for (int s = m_SoftSpawnZones.Count() - 1; s >= 0; s--)
        {
            if (m_SoftSpawnZones.Get(s) == targetZone)
                m_SoftSpawnZones.Remove(s);
        }
    }

    void ResetZoneRuntime(ref SimpleZone zone)
    {
        if (!zone)
            return;

        zone.cooldownTime = 0;
        zone.hasSpawned = false;
        zone.hasRolledChance = false;
        zone.playersInside.Clear();
        zone.lastPlayerSeenTime = -9999.0;
        zone.emptySince = -1.0;
        zone.softSpawnTimer = 0.0;

        if (zone.spawnPoints)
        {
            for (int i = 0; i < zone.spawnPoints.Count(); i++)
            {
                ref SimpleSpawnPoint point = zone.spawnPoints.Get(i);
                if (point && point.spawnedEntities)
                    point.spawnedEntities.Clear();
            }
        }
    }

    void DespawnLivingEntitiesForAdminReload()
    {
        for (int i = 0; i < m_ZonesMap.Count(); i++)
        {
            ref SimpleZone zone = m_ZonesMap.GetElement(i);
            if (!zone)
                continue;

            DespawnFromZone(zone);
            ResetZoneRuntime(zone);
        }
    }

    void AdminHotReload()
    {
        Print("[MZSS:RELOAD] === ADMIN HOT RELOAD START ===");
        Print("[MZSS:RELOAD] Runtime before reload: zones=" + m_ZonesMap.Count().ToString() + " tiers=" + m_Tiers.Count().ToString() + " queue=" + m_SpawnQueuePoints.Count().ToString() + " softZones=" + m_SoftSpawnZones.Count().ToString());

        ResetRuntimeForReload();
        LoadAllConfigs();
        RefreshPlayerCache();

        // Make the new config active immediately instead of waiting for movement/cache timers.
        m_CheckTimer = 0.0;
        CheckPlayersOptimized();
        DrainSpawnQueue();

        if (m_PersistCooldowns)
            SaveCooldowns();

        Print("[MZSS:RELOAD] Runtime after reload: zones=" + m_ZonesMap.Count().ToString() + " tiers=" + m_Tiers.Count().ToString() + " grid=" + m_ZoneGrid.Count().ToString() + " players=" + m_CachedPlayers.Count().ToString() + " queue=" + m_SpawnQueuePoints.Count().ToString() + " softZones=" + m_SoftSpawnZones.Count().ToString());
        Print("[MZSS:RELOAD] === ADMIN HOT RELOAD COMPLETE ===");
    }

    void ResetRuntimeForReload()
    {
        // Important: reload is an admin operation. The normal runtime logic stays unchanged,
        // but old runtime queues/zone references must be cleared before the JSON is re-read.
        ClearSpawnQueues();
        DespawnLivingEntitiesForAdminReload();
        m_DirtyZones.Clear();
        m_PlayerGridCache.Clear();
        m_PlayerLastPos.Clear();
    }


    ref SimpleZone FindNearestEnabledZone(vector sourcePos)
    {
        ref SimpleZone nearestZone = null;
        float nearestDistSq = 999999999.0;

        for (int i = 0; i < m_ZonesMap.Count(); i++)
        {
            ref SimpleZone zone = m_ZonesMap.GetElement(i);
            if (!zone || !zone.enabled)
                continue;

            float distSq = vector.DistanceSq(sourcePos, zone.position);
            if (distSq < nearestDistSq)
            {
                nearestDistSq = distSq;
                nearestZone = zone;
            }
        }

        return nearestZone;
    }

    int ForceQueueZoneSpawns(ref SimpleZone zone)
    {
        if (!zone || !zone.spawnPoints || zone.spawnPoints.Count() == 0)
            return 0;

        ClearQueuedSpawnsForZone(zone);
        DespawnFromZone(zone);
        CleanupSpawnPointEntities(zone);

        zone.cooldownTime = 0;
        zone.hasSpawned = false;
        zone.hasRolledChance = true;
        zone.emptySince = -1.0;
        zone.lastPlayerSeenTime = m_ElapsedTime;
        zone.softSpawnTimer = 0.0;

        int queued = 0;
        for (int i = 0; i < zone.spawnPoints.Count(); i++)
        {
            ref SimpleSpawnPoint point = zone.spawnPoints.Get(i);
            if (!point)
                continue;

            if (point.spawnedEntities)
                point.spawnedEntities.Clear();

            for (int j = 0; j < point.entities; j++)
            {
                m_SpawnQueuePoints.Insert(point);
                m_SpawnQueueZones.Insert(zone);
                queued++;
            }
        }

        if (queued > 0 && m_SoftSpawnZones.Find(zone) == -1)
            m_SoftSpawnZones.Insert(zone);

        return queued;
    }

    bool ForceSpawnNearestAtPosition(vector sourcePos)
    {
        ref SimpleZone nearestZone = FindNearestEnabledZone(sourcePos);
        if (!nearestZone)
        {
            Print("[MZSS] Admin force spawn failed: no enabled zones found.");
            return false;
        }

        float distance = Math.Sqrt(vector.DistanceSq(sourcePos, nearestZone.position));
        Print("[MZSS] Admin force spawning nearest zone: " + nearestZone.name + " at " + distance.ToString() + "m");

        int queued = ForceQueueZoneSpawns(nearestZone);
        if (queued <= 0)
        {
            Print("[MZSS] Admin force spawn failed: selected zone has no spawn requests.");
            return false;
        }

        DrainSpawnQueue();
        Print("[MZSS] Admin force spawn queued " + queued.ToString() + " entities for zone " + nearestZone.name);
        return true;
    }

    void ForceReload()
    {
        AdminHotReload();
    }
    
    void ToggleSystem()
    {
        m_SystemEnabled = !m_SystemEnabled;
        Print("[MZSS] System toggled: " + m_SystemEnabled.ToString());
    }
    
    void ToggleDebug()
    {
        m_DebugMode = !m_DebugMode;
        Print("[MZSS] Debug mode toggled: " + m_DebugMode.ToString());
    }
    
    bool IsEnabled()
    {
        return m_SystemEnabled;
    }
    
    void PrintStatus()
    {
        Print("[MZSS] === SYSTEM STATUS ===");
        Print("[MZSS] Enabled: " + m_SystemEnabled.ToString());
        Print("[MZSS] Debug Mode: " + m_DebugMode.ToString());
        Print("[MZSS] Tiers: " + m_Tiers.Count().ToString());
        Print("[MZSS] Zones: " + m_ZonesMap.Count().ToString());
        Print("[MZSS] Grid cells: " + m_ZoneGrid.Count().ToString());
        Print("[MZSS] Dirty zones: " + m_DirtyZones.Count().ToString());
        Print("[MZSS] Cached players: " + m_CachedPlayers.Count().ToString());
        Print("[MZSS] Player cache timer: " + m_PlayerCacheTimer.ToString() + "/" + m_PlayerCacheDuration.ToString() + "s");
        Print("[MZSS] Spawn queue: " + m_SpawnQueuePoints.Count().ToString() + " pending");
        Print("[MZSS] Soft spawn zones: " + m_SoftSpawnZones.Count().ToString() + " active");
        Print("[MZSS] Spawn budget/tick: " + m_MaxSpawnsPerTick.ToString());
        Print("[MZSS] Soft spawn interval: " + m_SoftSpawnInterval.ToString() + "s");
        Print("[MZSS] Despawn delay: " + m_DespawnDelay.ToString() + "s");
        
        int totalSpawnPoints = 0;
        int totalPossibleEntities = 0;
        int totalActiveEntities = 0;
        int zonesWithPlayers = 0;
        
        for (int i = 0; i < m_ZonesMap.Count(); i++)
        {
            string zoneName = m_ZonesMap.GetKey(i);
            ref SimpleZone zone = m_ZonesMap.GetElement(i);
            
            if (zone)
            {
                int zoneEntityCount = CountZoneEntities(zone);
                int zonePossibleEntities = 0;
                
                for (int j = 0; j < zone.spawnPoints.Count(); j++)
                {
                    ref SimpleSpawnPoint point = zone.spawnPoints.Get(j);
                    if (point)
                    {
                        zonePossibleEntities += point.entities;
                    }
                }
                
                totalSpawnPoints += zone.spawnPoints.Count();
                totalPossibleEntities += zonePossibleEntities;
                totalActiveEntities += zoneEntityCount;
                
                if (zone.playersInside.Count() > 0)
                {
                    zonesWithPlayers++;
                    Print("[MZSS] Zone: " + zone.name + " [ACTIVE]");
                    Print("[MZSS]   Players inside: " + zone.playersInside.Count().ToString());
                }
                else
                {
                    Print("[MZSS] Zone: " + zone.name);
                }
                
                Print("[MZSS]   Entities: " + zoneEntityCount.ToString() + "/" + zonePossibleEntities.ToString());
                
                if (zone.cooldownTime > 0)
                {
                    Print("[MZSS]   Cooldown: " + zone.cooldownTime.ToString() + "s");
                }
            }
        }
        
        Print("[MZSS] === TOTALS ===");
        Print("[MZSS] Active zones: " + zonesWithPlayers.ToString() + "/" + m_ZonesMap.Count().ToString());
        Print("[MZSS] Total entities: " + totalActiveEntities.ToString() + "/" + totalPossibleEntities.ToString());
    }
}

// ============= PART 3: GLOBAL FUNCTIONS =============

static
