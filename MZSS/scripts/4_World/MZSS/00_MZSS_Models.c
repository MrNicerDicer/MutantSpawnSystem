// ============= MZSS Main Mod =============
// Optimized Server-Side Spawn System for DayZ 1.29
// Pure server-side PBO - no client download required
// Refactored package: MZSS.pbo

// ============= PART 1: CLASS DEFINITIONS =============

class TierConfig : Managed
{
    string name;
    ref array<string> classnames;
    
    void TierConfig()
    {
        classnames = new array<string>;
    }
}

class TiersContainer : Managed 
{
    ref map<int, ref TierConfig> tiers;
    
    void TiersContainer()
    {
        tiers = new map<int, ref TierConfig>;
    }
}

class SpawnPointConfig : Managed
{
    string position;
    float radius;
    ref array<int> tierIds;
    int entities;
    bool useFixedHeight;
    
    void SpawnPointConfig()
    {
        tierIds = new array<int>;
        radius = 2.0;
        entities = 1;
        useFixedHeight = false;
    }
}

class ZoneConfig : Managed
{
    string name;
    bool enabled;
    string position;
    float triggerRadius;
    float spawnChance;
    bool despawnOnExit;
    float despawnDistance;
    float respawnCooldown;
    ref array<ref SpawnPointConfig> spawnPoints;
    
    void ZoneConfig()
    {
        spawnPoints = new array<ref SpawnPointConfig>;
        enabled = true;
        spawnChance = 1.0;
        despawnOnExit = true;
        despawnDistance = 400.0;
        respawnCooldown = 300.0;
        triggerRadius = 300.0;
    }
}

class GlobalSettingsConfig : Managed
{
    bool systemEnabled;
    float checkInterval;
    int maxEntitiesPerZone;
    int entityLifetime;
    float minSpawnDistanceFromPlayer;
    bool debugMode;
    float playerCacheDuration;
    // Grace period (seconds) before a zone despawns after all players leave
    float despawnGracePeriod;
    // Max entities spawned per tick to prevent burst spikes
    int maxSpawnsPerTick;
    // Minimum distance a player must move before triggering a new zone check
    float playerMoveThreshold;
    
    void GlobalSettingsConfig()
    {
        systemEnabled = true;
        checkInterval = 5.0;
        maxEntitiesPerZone = 20;
        entityLifetime = 1800;
        minSpawnDistanceFromPlayer = 30.0;
        debugMode = false;
        playerCacheDuration = 15.0;
        despawnGracePeriod = 120.0;
        maxSpawnsPerTick = 5;
        playerMoveThreshold = 5.0;
    }
}

// ============= SpawnerSettings.json container =============
// Neue eigenständige Konfigurationsdatei. Überschreibt globalSettings aus Zones.json.
// Felder mit Kommentar sind neu gegenüber GlobalSettingsConfig.
class SpawnerSettingsConfig : Managed
{
    // --- General ---
    string _help_systemEnabled          = "Enable or disable the entire spawn system.";
    bool systemEnabled;

    string _help_checkInterval          = "How often (in seconds) the system checks players and zones. Lower = more responsive, higher = better performance.";
    float checkInterval;

    string _help_maxEntitiesPerZone     = "Hard cap on how many entities can be alive in a single zone at once.";
    int maxEntitiesPerZone;

    string _help_maxTotalSpawnedEntities = "Optional hard cap for all active entities spawned by MZSS across the whole server. 0 = disabled.";
    int maxTotalSpawnedEntities;

    string _help_entityLifetime         = "Seconds before a spawned entity is automatically deleted, even if still alive. 0 = disabled.";
    int entityLifetime;

    string _help_minSpawnDistFromPlayer = "Minimum distance (metres) between a player and a spawn point. Entities will not spawn closer than this.";
    float minSpawnDistanceFromPlayer;

    string _help_playerCacheDuration    = "How long (in seconds) the player list is cached before being refreshed from the game. Reduces GetPlayers() calls.";
    float playerCacheDuration;

    string _help_playerMoveThreshold    = "Minimum distance (metres) a player must move before their grid cell is recalculated. Reduces redundant zone lookups.";
    float playerMoveThreshold;

    // --- Spawn Budget ---
    string _help_spawnBudgetPerTick     = "Maximum number of entities that can be spawned per update tick. Prevents CPU spikes when many zones activate at once.";
    int spawnBudgetPerTick;

    // --- Soft Spawn ---
    string _help_softSpawnInterval      = "Seconds between each individual entity spawn within a zone. Spreads spawning over time instead of all at once. Example: 0.25 = one entity every 250ms.";
    float softSpawnInterval;

    // --- Despawn ---
    string _help_despawnDelay           = "Seconds a zone must remain empty (no players inside or nearby) before its entities are despawned. Prevents despawn spam on quick re-entries.";
    float despawnDelay;

    string _help_protectLivingEntitiesFromDespawn = "If enabled, a zone will not despawn while any living entity spawned by that zone is close to a player. Prevents mutants/zombies disappearing in front of players after chasing them away from the zone center.";
    bool protectLivingEntitiesFromDespawn;

    string _help_livingEntityDespawnProtectionDistance = "Distance in metres used by protectLivingEntitiesFromDespawn. If any player is within this distance of a living spawned entity, the zone despawn timer is reset.";
    float livingEntityDespawnProtectionDistance;

    // --- Cooldown Persistence ---
    string _help_persistCooldowns       = "If enabled, zone cooldowns are saved to profile and restored after restart. 0 = disabled, 1 = enabled.";
    bool persistCooldowns;

    string _help_cooldownPersistenceInterval = "Seconds between automatic cooldown persistence writes. Only used if persistCooldowns is enabled.";
    float cooldownPersistenceInterval;

    // --- World State Persistence ---
    string _help_persistWorldState      = "If enabled, MZSS saves runtime state for migration links, active migration effects and nests to MZSS_WorldState.json.";
    bool persistWorldState;

    string _help_worldStateSaveInterval = "Seconds between automatic MZSS_WorldState.json writes when persistWorldState is enabled.";
    float worldStateSaveInterval;

    // --- Diagnostics / Debug ---
    string _help_debugMode              = "Master debug toggle. If disabled, only essential startup/reload/admin logs are printed.";
    bool debugMode;

    string _help_diagnosticMode         = "Dry-run / diagnostic mode. Adds extra explanation logs about why zones do or do not spawn.";
    bool diagnosticMode;

    string _help_logLoad                = "Print load/config logs.";
    bool logLoad;

    string _help_logSpawn               = "Print spawn and spawn-queue logs.";
    bool logSpawn;

    string _help_logDespawn             = "Print despawn logs.";
    bool logDespawn;

    string _help_logReload              = "Print reload logs.";
    bool logReload;

    string _help_logValidator           = "Print config validation warnings.";
    bool logValidator;

    string _help_logAdmin               = "Print admin bridge logs.";
    bool logAdmin;

    string _help_logDiagnostics         = "Print diagnostic / dry-run logs.";
    bool logDiagnostics;

    void SpawnerSettingsConfig()
    {
        systemEnabled               = true;
        checkInterval               = 5.0;
        maxEntitiesPerZone          = 40;
        maxTotalSpawnedEntities     = 0;
        entityLifetime              = 1200;
        minSpawnDistanceFromPlayer  = 5.0;
        playerCacheDuration         = 15.0;
        playerMoveThreshold         = 5.0;

        spawnBudgetPerTick          = 10;
        softSpawnInterval           = 0.25;
        despawnDelay                = 45.0;
        protectLivingEntitiesFromDespawn = true;
        livingEntityDespawnProtectionDistance = 80.0;

        persistCooldowns            = false;
        cooldownPersistenceInterval = 60.0;

        persistWorldState           = true;
        worldStateSaveInterval      = 60.0;

        debugMode                   = false;
        diagnosticMode              = false;

        logLoad                     = true;
        logSpawn                    = false;
        logDespawn                  = false;
        logReload                   = true;
        logValidator                = true;
        logAdmin                    = true;
        logDiagnostics              = false;
    }
}


// ============= Optional player rules =============
// Used for special factions/admins such as Monolith/Sin-style players.
// Main mod remains server-side; this only affects how MZSS evaluates players.
class MZSS_PlayerRuleEntry : Managed
{
    string steamId;
    string name;
    bool canTriggerZones;
    bool ignoredBySpawnedAI;

    void MZSS_PlayerRuleEntry()
    {
        steamId = "";
        name = "";
        canTriggerZones = true;
        ignoredBySpawnedAI = false;
    }
}

class MZSS_PlayerRulesConfig : Managed
{
    bool enabled;
    ref array<ref MZSS_PlayerRuleEntry> players;

    void MZSS_PlayerRulesConfig()
    {
        enabled = true;
        players = new array<ref MZSS_PlayerRuleEntry>;
    }
}

// ============= Optional anti-clipping placement =============
// Conservative spawn-position validation for custom map objects/bunkers.
class MZSS_AntiClippingConfig : Managed
{
    bool enabled;
    bool groundRaycast;
    bool clearanceCheck;
    int retryAttempts;
    float retryRadius;
    float minWallDistance;
    float verticalProbeUp;
    float verticalProbeDown;
    bool fallbackToOriginalPosition;

    void MZSS_AntiClippingConfig()
    {
        enabled = true;
        groundRaycast = true;
        clearanceCheck = true;
        retryAttempts = 8;
        retryRadius = 2.5;
        minWallDistance = 0.8;
        verticalProbeUp = 1.5;
        verticalProbeDown = 5.0;
        fallbackToOriginalPosition = true;
    }
}



// ============= Optional migration system =============
// Migration is a lightweight, system-driven "A-Life" style mechanic.
// It does not move AI across the map. Instead, source zone events can temporarily affect target zones.
class MZSS_MigrationLinkConfig : Managed
{
    string name;
    bool enabled;
    ref array<string> sourceZoneNames;
    ref array<string> targetZoneNames;
    string trigger;
    float chance;
    float cooldown;
    string targetSelection;
    float effectDuration;
    float targetSpawnChanceBonus;
    float targetCooldownReduction;
    bool forceTargetReady;

    void MZSS_MigrationLinkConfig()
    {
        name = "Example_Zone_156_to_Zone_157";
        enabled = true;
        sourceZoneNames = new array<string>;
        targetZoneNames = new array<string>;
        sourceZoneNames.Insert("Zone_156");
        targetZoneNames.Insert("Zone_157");
        trigger = "sourceCleared";
        chance = 0.35;
        cooldown = 3600.0;
        targetSelection = "randomOne";
        effectDuration = 1800.0;
        targetSpawnChanceBonus = 0.25;
        targetCooldownReduction = 900.0;
        forceTargetReady = false;
    }
}

class MZSS_MigrationConfig : Managed
{
    bool enabled;
    ref array<ref MZSS_MigrationLinkConfig> links;

    void MZSS_MigrationConfig()
    {
        enabled = false;
        links = new array<ref MZSS_MigrationLinkConfig>;

        ref MZSS_MigrationLinkConfig example = new MZSS_MigrationLinkConfig();
        links.Insert(example);
    }
}

// ============= Optional nest system =============
// A nest is a logical hotspot at a position. The nestClassname is descriptive and useful for admins,
// but MZSS does not require the object to be destructible.
class MZSS_NestConfig : Managed
{
    string name;
    bool enabled;
    string nestClassname;
    string position;
    float radius;
    float activationChance;
    float cooldown;
    float activeDuration;
    string triggerMode;
    string clearMode;
    ref array<string> spawnClassnames;
    int minEntities;
    int maxEntities;
    float spawnRadius;
    bool useFixedHeight;

    bool notificationEnabled;
    string notificationTitle;
    string notificationMessage;
    float notificationCooldown;

    void MZSS_NestConfig()
    {
        name = "Example_Bloodsucker_Nest";
        enabled = true;
        nestClassname = "Land_House_1W01";
        position = "3810.685303 123.246178 6968.397461";
        radius = 35.0;
        activationChance = 0.35;
        cooldown = 7200.0;
        activeDuration = 1800.0;
        triggerMode = "playerNear";
        clearMode = "killAllSpawnedOrTimer";
        spawnClassnames = new array<string>;
        spawnClassnames.Insert("Mutant_Bloodsucker");
        minEntities = 2;
        maxEntities = 5;
        spawnRadius = 12.0;
        useFixedHeight = false;

        notificationEnabled = false;
        notificationTitle = "Warning";
        notificationMessage = "Something is moving nearby...";
        notificationCooldown = 900.0;
    }
}

class MZSS_NestsConfig : Managed
{
    bool enabled;
    ref array<ref MZSS_NestConfig> nests;

    void MZSS_NestsConfig()
    {
        enabled = false;
        nests = new array<ref MZSS_NestConfig>;

        ref MZSS_NestConfig example = new MZSS_NestConfig();
        nests.Insert(example);
    }
}

// ============= Runtime world state =============
// This is not meant as primary config. It is the savegame-like runtime state for MZSS systems.
class MZSS_MigrationLinkState : Managed
{
    string linkName;
    float cooldownRemaining;
    int lastTriggeredUnixTime;

    void MZSS_MigrationLinkState()
    {
        linkName = "";
        cooldownRemaining = 0.0;
        lastTriggeredUnixTime = 0;
    }
}

class MZSS_MigrationEffectState : Managed
{
    string effectId;
    string linkName;
    string targetZoneName;
    float remainingDuration;
    float spawnChanceBonus;
    float cooldownReductionApplied;
    bool forceTargetReadyApplied;

    void MZSS_MigrationEffectState()
    {
        effectId = "";
        linkName = "";
        targetZoneName = "";
        remainingDuration = 0.0;
        spawnChanceBonus = 0.0;
        cooldownReductionApplied = 0.0;
        forceTargetReadyApplied = false;
    }
}

class MZSS_NestState : Managed
{
    string nestName;
    string state;
    float cooldownRemaining;
    float activeDurationRemaining;
    int lastActivatedUnixTime;
    int lastClearedUnixTime;
    int spawnedAlive;
    int spawnedTotal;

    void MZSS_NestState()
    {
        nestName = "";
        state = "inactive";
        cooldownRemaining = 0.0;
        activeDurationRemaining = 0.0;
        lastActivatedUnixTime = 0;
        lastClearedUnixTime = 0;
        spawnedAlive = 0;
        spawnedTotal = 0;
    }
}

class MZSS_WorldStateConfig : Managed
{
    int version;
    int lastSavedUnixTime;
    ref array<ref MZSS_MigrationLinkState> migrationStates;
    ref array<ref MZSS_MigrationEffectState> activeMigrationEffects;
    ref array<ref MZSS_NestState> nestStates;

    void MZSS_WorldStateConfig()
    {
        version = 1;
        lastSavedUnixTime = 0;
        migrationStates = new array<ref MZSS_MigrationLinkState>;
        activeMigrationEffects = new array<ref MZSS_MigrationEffectState>;
        nestStates = new array<ref MZSS_NestState>;
    }
}



// ============= Optional personal zone notifications =============
// Server-side personal messages for players entering configured zones.
// This does not require a custom client UI and is not sent to global chat.
class MZSS_NotificationEntry : Managed
{
    string zoneName;
    bool enabled;
    string trigger;
    string title;
    string message;
    float cooldown;

    void MZSS_NotificationEntry()
    {
        zoneName = "Zone_156";
        enabled = true;
        trigger = "onEnter";
        title = "Warning";
        message = "Be careful, dangerous zombies ahead.";
        cooldown = 900.0;
    }
}

class MZSS_NotificationsConfig : Managed
{
    bool enabled;
    string mode;
    float defaultCooldown;
    ref array<ref MZSS_NotificationEntry> notifications;

    void MZSS_NotificationsConfig()
    {
        enabled = false;
        mode = "personalMessage";
        defaultCooldown = 900.0;
        notifications = new array<ref MZSS_NotificationEntry>;

        ref MZSS_NotificationEntry example = new MZSS_NotificationEntry();
        notifications.Insert(example);
    }
}


// ============= Optional Admin Tool command bridge =============
// Used by MZSSAdminTool when present. If the admin tool is not installed,
// this file is never written and the normal JSON/manual workflow is unchanged.
class MZSS_AdminCommandConfig : Managed
{
    int commandId;
    string action;
    string position;

    void MZSS_AdminCommandConfig()
    {
        commandId = 0;
        action = "";
        position = "";
    }
}


// ============= Optional cooldown persistence =============

class MZSS_CooldownEntry : Managed
{
    string zoneName;
    float cooldownTime;
    bool hasSpawned;
    bool hasRolledChance;

    void MZSS_CooldownEntry()
    {
        zoneName = "";
        cooldownTime = 0.0;
        hasSpawned = false;
        hasRolledChance = false;
    }
}

class MZSS_CooldownsContainer : Managed
{
    ref array<ref MZSS_CooldownEntry> cooldowns;

    void MZSS_CooldownsContainer()
    {
        cooldowns = new array<ref MZSS_CooldownEntry>;
    }
}

class ZonesContainer : Managed
{
    ref array<ref ZoneConfig> zones;
    
    void ZonesContainer()
    {
        zones = new array<ref ZoneConfig>;
    }
}

class SimpleTier : Managed
{
    string name;
    ref array<string> classnames;
    
    void SimpleTier()
    {
        classnames = new array<string>;
    }
}

class SimpleSpawnPoint : Managed
{
    vector position;
    float radius;
    ref array<int> tierIds;
    int entities;
    bool useFixedHeight;
    ref array<EntityAI> spawnedEntities;
    
    void SimpleSpawnPoint()
    {
        tierIds = new array<int>;
        spawnedEntities = new array<EntityAI>;
        radius = 2.0;
        entities = 1;
        useFixedHeight = false;
    }
}

class SimpleZone : Managed
{
    string name;
    bool enabled;
    vector position;
    float triggerRadius;
    float triggerRadiusSq;
    float spawnChance;
    bool despawnOnExit;
    float despawnDistance;
    float despawnDistanceSq;
    float respawnCooldown;
    ref array<ref SimpleSpawnPoint> spawnPoints;
    
    float cooldownTime;
    bool hasSpawned;
    bool hasRolledChance;
    ref array<Man> playersInside;
    // Tracks the last time any player was seen inside or near this zone
    float lastPlayerSeenTime;
    // Zeitpunkt, ab dem die Zone leer ist (für Despawn Delay)
    float emptySince;
    // Soft Spawn: Timer für zeitversetztes Spawnen innerhalb eines SpawnPoints
    float softSpawnTimer;
    
    void SimpleZone()
    {
        spawnPoints = new array<ref SimpleSpawnPoint>;
        playersInside = new array<Man>;
        enabled = true;
        spawnChance = 1.0;
        despawnOnExit = true;
        respawnCooldown = 300.0;
        cooldownTime = 0;
        hasSpawned = false;
        hasRolledChance = false;
        lastPlayerSeenTime = -9999.0;
        emptySince = -1.0;
        softSpawnTimer = 0.0;
    }
    
    void CacheSquaredDistances()
    {
        triggerRadiusSq = triggerRadius * triggerRadius;
        despawnDistanceSq = despawnDistance * despawnDistance;
    }
}

// ============= PART 2: OPTIMIZED SPAWN MANAGER =============
