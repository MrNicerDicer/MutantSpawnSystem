// ============= CombinedMutantSpawnSystem.c =============
// All components in one file to avoid load order issues

// ============= PART 1: CLASS DEFINITIONS =============

// Data structure classes that match JSON format
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
    bool useFixedHeight;  // Use Y coordinate directly instead of terrain height
    
    void SpawnPointConfig()
    {
        tierIds = new array<int>;
        radius = 2.0;
        entities = 1;
        useFixedHeight = false;  // Default: use terrain height
    }
}

class ZoneConfig : Managed
{
    string name;
    bool enabled;
    string position;
    float triggerRadius;
    float spawnChance;  // Chance to spawn when triggered (0.0 - 1.0)
    bool despawnOnExit;
    float despawnDistance;
    float respawnCooldown;
    ref array<ref SpawnPointConfig> spawnPoints;
    
    void ZoneConfig()
    {
        spawnPoints = new array<ref SpawnPointConfig>;
        enabled = true;
        spawnChance = 1.0;  // Default: 100% chance
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
    
    void GlobalSettingsConfig()
    {
        systemEnabled = true;
        checkInterval = 15.0;
        maxEntitiesPerZone = 20;
        entityLifetime = 1800;
        minSpawnDistanceFromPlayer = 30.0;
    }
}

class ZonesContainer : Managed
{
    ref GlobalSettingsConfig globalSettings;
    ref array<ref ZoneConfig> zones;
    
    void ZonesContainer()
    {
        globalSettings = new GlobalSettingsConfig();
        zones = new array<ref ZoneConfig>;
    }
}

// Runtime classes
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
    float spawnChance;
    bool despawnOnExit;
    float despawnDistance;
    float respawnCooldown;
    ref array<ref SimpleSpawnPoint> spawnPoints;
    
    void SimpleZone()
    {
        spawnPoints = new array<ref SimpleSpawnPoint>;
        enabled = true;
        spawnChance = 1.0;
        despawnOnExit = true;
        respawnCooldown = 300.0;
    }
}

// ============= PART 2: SPAWN MANAGER =============

class SimpleSpawnManager : Managed
{
    bool m_SystemEnabled;
    float m_CheckTimer;
    float m_CheckInterval;
    float m_LastSpawnCheck;
    float m_MinSpawnDistanceFromPlayer;
    
    ref map<int, ref SimpleTier> m_Tiers;
    ref map<string, ref SimpleZone> m_ZonesMap;
    ref GlobalSettingsConfig m_GlobalSettings;
    ref map<string, ref array<EntityAI>> m_SpawnedEntities;
    ref map<string, float> m_ZoneCooldowns;
    ref map<string, bool> m_ZoneSpawnedStatus;
    ref map<string, bool> m_ZoneRolledChance;  // Track if zone already rolled for spawn chance
    
    void SimpleSpawnManager()
    {
        m_SystemEnabled = true;
        m_CheckTimer = 0.0;
        m_CheckInterval = 15.0;
        m_LastSpawnCheck = 0.0;
        m_MinSpawnDistanceFromPlayer = 30.0;
        
        m_Tiers = new map<int, ref SimpleTier>;
        m_ZonesMap = new map<string, ref SimpleZone>;
        m_GlobalSettings = new GlobalSettingsConfig;
        m_SpawnedEntities = new map<string, ref array<EntityAI>>;
        m_ZoneCooldowns = new map<string, float>;
        m_ZoneSpawnedStatus = new map<string, bool>;
        m_ZoneRolledChance = new map<string, bool>;
        
        Print("[SpawnManager] === SPAWN POINT SYSTEM v2.1 INITIALIZED ===");
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CreateDefaultConfigs, 3000, false);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(LoadAllConfigs, 8000, false);
    }
    
    void CreateDefaultConfigs()
    {
        string configDir = "$profile:MutantSpawnSystem";
        
        if (!FileExist(configDir))
        {
            MakeDirectory(configDir);
            Print("[SpawnManager] Created config directory");
        }
        
        string tiersPath = "$profile:MutantSpawnSystem/Tiers.json";
        if (!FileExist(tiersPath))
        {
            CreateDefaultTiers(tiersPath);
        }
        
        string zonesPath = "$profile:MutantSpawnSystem/Zones.json";
        if (!FileExist(zonesPath))
        {
            CreateDefaultZonesWithSpawnPoints(zonesPath);
        }
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
        Print("[SpawnManager] Created default Tiers.json");
    }
    
    void CreateDefaultZonesWithSpawnPoints(string filePath)
    {
        ref ZonesContainer container = new ZonesContainer();
        
        container.globalSettings.systemEnabled = true;
        container.globalSettings.checkInterval = 15.0;
        container.globalSettings.maxEntitiesPerZone = 20;
        container.globalSettings.entityLifetime = 600;
        container.globalSettings.minSpawnDistanceFromPlayer = 30.0;
        
        // Zone 1: Urban Building Example
        ref ZoneConfig zone1 = new ZoneConfig();
        zone1.name = "Test_Zone_Cherno_Building";
        zone1.enabled = true;
        zone1.position = "6560 15 2630";
        zone1.triggerRadius = 100;
        zone1.spawnChance = 0.75;  // 75% chance to spawn
        zone1.despawnOnExit = true;
        zone1.despawnDistance = 150;
        zone1.respawnCooldown = 300;
        
        ref SpawnPointConfig sp1 = new SpawnPointConfig();
        sp1.position = "6555 15 2625";
        sp1.radius = 1.5;
        sp1.tierIds.Insert(1);
        sp1.entities = 2;
        sp1.useFixedHeight = false;  // Use terrain height
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
        sp3.useFixedHeight = true;  // Use exact Y coordinate (for upper floor)
        zone1.spawnPoints.Insert(sp3);
        
        container.zones.Insert(zone1);
        
        // Zone 2: Underground Bunker Example
        ref ZoneConfig zone2 = new ZoneConfig();
        zone2.name = "Test_Zone_Underground_Bunker";
        zone2.enabled = true;
        zone2.position = "-1000 -50 -1000";  // Bunker outside map
        zone2.triggerRadius = 80;
        zone2.spawnChance = 0.5;  // 50% chance - more random
        zone2.despawnOnExit = true;
        zone2.despawnDistance = 120;
        zone2.respawnCooldown = 420;
        
        ref SpawnPointConfig sp4 = new SpawnPointConfig();
        sp4.position = "-1000 -48.5 -995";  // Exact bunker floor height
        sp4.radius = 2.0;
        sp4.tierIds.Insert(2);
        sp4.entities = 2;
        sp4.useFixedHeight = true;  // IMPORTANT for bunker!
        zone2.spawnPoints.Insert(sp4);
        
        ref SpawnPointConfig sp5 = new SpawnPointConfig();
        sp5.position = "-990 -48.5 -1000";
        sp5.radius = 1.5;
        sp5.tierIds.Insert(1);
        sp5.tierIds.Insert(2);
        sp5.entities = 3;
        sp5.useFixedHeight = true;  // IMPORTANT for bunker!
        zone2.spawnPoints.Insert(sp5);
        
        ref SpawnPointConfig sp6 = new SpawnPointConfig();
        sp6.position = "-1010 -48.5 -1005";
        sp6.radius = 2.5;
        sp6.tierIds.Insert(2);
        sp6.entities = 1;
        sp6.useFixedHeight = true;  // IMPORTANT for bunker!
        zone2.spawnPoints.Insert(sp6);
        
        container.zones.Insert(zone2);
        
        JsonFileLoader<ZonesContainer>.JsonSaveFile(filePath, container);
        Print("[SpawnManager] Created default Zones.json with 2 example zones (including bunker)");
    }
    
    void LoadAllConfigs()
    {
        Print("[SpawnManager] === LOADING CONFIGURATIONS ===");
        
        m_Tiers.Clear();
        m_ZonesMap.Clear();
        m_SpawnedEntities.Clear();
        m_ZoneCooldowns.Clear();
        m_ZoneSpawnedStatus.Clear();
        m_ZoneRolledChance.Clear();
        
        LoadTiers();
        LoadZones();
        
        Print("[SpawnManager] Configuration loaded:");
        Print("[SpawnManager] - Tiers: " + m_Tiers.Count().ToString());
        Print("[SpawnManager] - Zones: " + m_ZonesMap.Count().ToString());
        Print("[SpawnManager] === SYSTEM READY ===");
    }
    
    void LoadTiers()
    {
        string tiersPath = "$profile:MutantSpawnSystem/Tiers.json";
        
        if (!FileExist(tiersPath))
        {
            Print("[SpawnManager] ERROR: Tiers.json not found!");
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
                        tier.classnames.Insert(config.classnames.Get(j));
                    }
                    
                    m_Tiers.Set(tierId, tier);
                    Print("[SpawnManager] Loaded tier " + tierId.ToString() + ": " + tier.name);
                }
            }
        }
    }
    
    void LoadZones()
    {
        string zonesPath = "$profile:MutantSpawnSystem/Zones.json";
        
        if (!FileExist(zonesPath))
        {
            Print("[SpawnManager] ERROR: Zones.json not found!");
            return;
        }
        
        ref ZonesContainer container = new ZonesContainer();
        JsonFileLoader<ZonesContainer>.JsonLoadFile(zonesPath, container);
        
        if (container)
        {
            if (container.globalSettings)
            {
                m_GlobalSettings = container.globalSettings;
                m_CheckInterval = m_GlobalSettings.checkInterval;
                m_SystemEnabled = m_GlobalSettings.systemEnabled;
                m_MinSpawnDistanceFromPlayer = m_GlobalSettings.minSpawnDistanceFromPlayer;
                
                Print("[SpawnManager] Loaded global settings");
            }
            
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
                        m_SpawnedEntities.Set(zone.name, new array<EntityAI>);
                        m_ZoneCooldowns.Set(zone.name, 0.0);
                        m_ZoneSpawnedStatus.Set(zone.name, false);
                        m_ZoneRolledChance.Set(zone.name, false);
                        
                        Print("[SpawnManager] Loaded zone: " + zone.name);
                        Print("[SpawnManager]   Spawn chance: " + (zone.spawnChance * 100).ToString() + "%");
                        Print("[SpawnManager]   Spawn points: " + zone.spawnPoints.Count().ToString() + " (Total entities: " + totalEntities.ToString() + ")");
                    }
                }
            }
        }
    }
    
    void Update(float timeslice)
    {
        if (!m_SystemEnabled) return;
        
        m_CheckTimer += timeslice;
        
        for (int i = 0; i < m_ZoneCooldowns.Count(); i++)
        {
            string zoneName = m_ZoneCooldowns.GetKey(i);
            float cooldown = m_ZoneCooldowns.GetElement(i);
            
            if (cooldown > 0)
            {
                cooldown -= timeslice;
                if (cooldown <= 0)
                {
                    cooldown = 0;
                    m_ZoneSpawnedStatus.Set(zoneName, false);
                    m_ZoneRolledChance.Set(zoneName, false);  // Reset chance roll
                }
                m_ZoneCooldowns.Set(zoneName, cooldown);
            }
        }
        
        if (m_CheckTimer >= m_CheckInterval)
        {
            m_CheckTimer = 0.0;
            CheckAllPlayers();
        }
        
        if (GetGame().GetTime() - m_LastSpawnCheck > 30000)
        {
            CleanupDeadEntities();
            m_LastSpawnCheck = GetGame().GetTime();
        }
    }
    
    void CheckAllPlayers()
    {
        if (m_Tiers.Count() == 0 || m_ZonesMap.Count() == 0)
            return;
        
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        if (players.Count() == 0)
            return;
        
        for (int zoneIdx = 0; zoneIdx < m_ZonesMap.Count(); zoneIdx++)
        {
            string zoneName = m_ZonesMap.GetKey(zoneIdx);
            ref SimpleZone zone = m_ZonesMap.GetElement(zoneIdx);
            
            if (!zone || !zone.enabled) continue;
            
            bool anyPlayerInTrigger = false;
            float closestPlayerDistance = 999999.0;
            
            for (int playerIdx = 0; playerIdx < players.Count(); playerIdx++)
            {
                Man player = players.Get(playerIdx);
                if (player && player.IsAlive())
                {
                    float distance = vector.Distance(player.GetPosition(), zone.position);
                    
                    if (distance < closestPlayerDistance)
                        closestPlayerDistance = distance;
                    
                    if (distance <= zone.triggerRadius)
                        anyPlayerInTrigger = true;
                }
            }
            
            if (anyPlayerInTrigger)
            {
                CheckAndSpawnInZone(zone, players);
            }
            else if (zone.despawnOnExit && closestPlayerDistance > zone.despawnDistance)
            {
                DespawnFromZone(zone);
            }
        }
    }
    
    void CheckAndSpawnInZone(ref SimpleZone zone, array<Man> players)
    {
        float cooldown;
        if (m_ZoneCooldowns.Find(zone.name, cooldown) && cooldown > 0)
            return;
        
        bool hasSpawned;
        if (m_ZoneSpawnedStatus.Find(zone.name, hasSpawned) && hasSpawned)
            return;
        
        // Check if we already rolled for spawn chance
        bool hasRolled;
        if (m_ZoneRolledChance.Find(zone.name, hasRolled) && hasRolled)
            return;
        
        // Roll spawn chance
        float roll = Math.RandomFloat(0.0, 1.0);
        m_ZoneRolledChance.Set(zone.name, true);
        
        if (roll > zone.spawnChance)
        {
            // Failed spawn chance roll
            Print("[SpawnManager] Zone " + zone.name + " failed spawn chance (rolled " + (roll * 100).ToString() + "%, needed <" + (zone.spawnChance * 100).ToString() + "%)");
            m_ZoneCooldowns.Set(zone.name, zone.respawnCooldown);  // Set cooldown even on failed roll
            return;
        }
        
        Print("[SpawnManager] Zone " + zone.name + " passed spawn chance (rolled " + (roll * 100).ToString() + "%)");
        
        if (!zone.spawnPoints || zone.spawnPoints.Count() == 0)
            return;
        
        CleanupSpawnPointEntities(zone);
        
        int currentTotal = CountZoneEntities(zone);
        if (currentTotal > 0)
            return;
        
        int totalSpawned = 0;
        
        // Spawn exact number of entities at each spawn point
        for (int i = 0; i < zone.spawnPoints.Count(); i++)
        {
            ref SimpleSpawnPoint point = zone.spawnPoints.Get(i);
            
            // Check if spawn point is safe from players
            if (!IsSpawnPointSafe(point, players))
            {
                Print("[SpawnManager] Spawn point " + (i+1).ToString() + " too close to player, skipping");
                continue;
            }
            
            // Spawn exact number of entities defined for this point
            for (int j = 0; j < point.entities; j++)
            {
                if (SpawnEntityAtPoint(zone, point))
                {
                    totalSpawned++;
                }
            }
        }
        
        if (totalSpawned > 0)
        {
            m_ZoneSpawnedStatus.Set(zone.name, true);
            Print("[SpawnManager] Spawned " + totalSpawned.ToString() + " entities in zone " + zone.name);
        }
        else
        {
            Print("[SpawnManager] Could not spawn in zone " + zone.name + " (all spawn points too close to players)");
        }
    }
    
    bool IsSpawnPointSafe(ref SimpleSpawnPoint point, array<Man> players)
    {
        for (int i = 0; i < players.Count(); i++)
        {
            Man player = players.Get(i);
            if (player && player.IsAlive())
            {
                float distance = vector.Distance(player.GetPosition(), point.position);
                if (distance < m_MinSpawnDistanceFromPlayer)
                {
                    return false;
                }
            }
        }
        return true;
    }
    
    bool SpawnEntityAtPoint(ref SimpleZone zone, ref SimpleSpawnPoint point)
    {
        if (point.tierIds.Count() == 0)
            return false;
        
        // Check if point already has max entities
        if (point.spawnedEntities.Count() >= point.entities)
            return false;
        
        int tierIndex = Math.RandomInt(0, point.tierIds.Count());
        int tierId = point.tierIds.Get(tierIndex);
        
        ref SimpleTier tier;
        if (!m_Tiers.Find(tierId, tier) || !tier || tier.classnames.Count() == 0)
            return false;
        
        int classIndex = Math.RandomInt(0, tier.classnames.Count());
        string entityClass = tier.classnames.Get(classIndex);
        
        vector spawnPos = point.position;
        if (point.radius > 0)
        {
            float angle = Math.RandomFloat(0, Math.PI2);
            float distance = Math.RandomFloat(0, point.radius);
            spawnPos[0] = spawnPos[0] + (Math.Cos(angle) * distance);
            spawnPos[2] = spawnPos[2] + (Math.Sin(angle) * distance);
        }
        
        // Use fixed height or terrain height
        if (point.useFixedHeight)
        {
            // Keep the Y coordinate from the config (for bunkers, upper floors, etc.)
            spawnPos[1] = point.position[1] + 0.5;
            Print("[SpawnManager] Using fixed height: " + spawnPos[1].ToString());
        }
        else
        {
            // Use terrain height (normal spawning)
            spawnPos[1] = GetGame().SurfaceY(spawnPos[0], spawnPos[2]) + 0.5;
        }
        
        EntityAI entity = EntityAI.Cast(GetGame().CreateObject(entityClass, spawnPos, false, true, true));
        
        if (entity)
        {
            entity.SetHealth("", "", entity.GetMaxHealth("", ""));
            
            point.spawnedEntities.Insert(entity);
            
            ref array<EntityAI> zoneEntities;
            if (!m_SpawnedEntities.Find(zone.name, zoneEntities))
            {
                zoneEntities = new array<EntityAI>;
                m_SpawnedEntities.Set(zone.name, zoneEntities);
            }
            zoneEntities.Insert(entity);
            
            if (m_GlobalSettings.entityLifetime > 0)
            {
                GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CleanupEntity, m_GlobalSettings.entityLifetime * 1000, false, entity);
            }
            
            Print("[SpawnManager] Spawned " + entityClass + " at point (" + point.spawnedEntities.Count().ToString() + "/" + point.entities.ToString() + ")");
            return true;
        }
        
        return false;
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
            
            for (int j = 0; j < point.spawnedEntities.Count(); j++)
            {
                EntityAI entity = point.spawnedEntities.Get(j);
                if (entity)
                {
                    entity.Delete();
                    totalDespawned++;
                }
            }
            point.spawnedEntities.Clear();
        }
        
        ref array<EntityAI> zoneEntities;
        if (m_SpawnedEntities.Find(zone.name, zoneEntities))
        {
            zoneEntities.Clear();
        }
        
        if (totalDespawned > 0)
        {
            m_ZoneCooldowns.Set(zone.name, zone.respawnCooldown);
            m_ZoneSpawnedStatus.Set(zone.name, false);
            m_ZoneRolledChance.Set(zone.name, false);  // Reset chance roll
        }
    }
    
    void CleanupDeadEntities()
    {
        for (int i = 0; i < m_ZonesMap.Count(); i++)
        {
            ref SimpleZone zone = m_ZonesMap.GetElement(i);
            if (zone)
            {
                CleanupSpawnPointEntities(zone);
                
                if (CountZoneEntities(zone) == 0)
                {
                    bool wasSpawned;
                    if (m_ZoneSpawnedStatus.Find(zone.name, wasSpawned) && wasSpawned)
                    {
                        m_ZoneCooldowns.Set(zone.name, zone.respawnCooldown);
                        m_ZoneSpawnedStatus.Set(zone.name, false);
                        m_ZoneRolledChance.Set(zone.name, false);  // Reset chance roll
                    }
                }
            }
        }
    }
    
    void CleanupEntity(EntityAI entity)
    {
        if (entity)
        {
            entity.Delete();
        }
    }
    
    void ForceReload()
    {
        Print("[SpawnManager] === FORCED RELOAD ===");
        LoadAllConfigs();
    }
    
    void ToggleSystem()
    {
        m_SystemEnabled = !m_SystemEnabled;
        Print("[SpawnManager] System toggled: " + m_SystemEnabled.ToString());
    }
    
    bool IsEnabled()
    {
        return m_SystemEnabled;
    }
    
    void PrintStatus()
    {
        Print("[SpawnManager] === SYSTEM STATUS ===");
        Print("[SpawnManager] Enabled: " + m_SystemEnabled.ToString());
        Print("[SpawnManager] Tiers: " + m_Tiers.Count().ToString());
        Print("[SpawnManager] Zones: " + m_ZonesMap.Count().ToString());
        
        int totalSpawnPoints = 0;
        int totalPossibleEntities = 0;
        int totalActiveEntities = 0;
        
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
                
                Print("[SpawnManager] Zone: " + zone.name);
                Print("[SpawnManager]   Trigger: " + zone.triggerRadius.ToString() + "m, Chance: " + (zone.spawnChance * 100).ToString() + "%");
                Print("[SpawnManager]   Points: " + zone.spawnPoints.Count().ToString());
                Print("[SpawnManager]   Entities: " + zoneEntityCount.ToString() + "/" + zonePossibleEntities.ToString());
                
                float cooldown;
                if (m_ZoneCooldowns.Find(zoneName, cooldown) && cooldown > 0)
                {
                    Print("[SpawnManager]   Cooldown: " + cooldown.ToString() + "s");
                }
            }
        }
        
        Print("[SpawnManager] === TOTALS ===");
        Print("[SpawnManager] Total Spawn Points: " + totalSpawnPoints.ToString());
        Print("[SpawnManager] Total Active: " + totalActiveEntities.ToString() + "/" + totalPossibleEntities.ToString());
    }
}

// ============= PART 3: GLOBAL FUNCTIONS =============

static ref SimpleSpawnManager g_SpawnManager;

void InitMutantSpawnSystem()
{
    if (g_SpawnManager)
    {
        Print("[MutantWorld] Spawn system already initialized");
        return;
    }
    
    Print("[MutantWorld] === INITIALIZING SPAWN POINT SYSTEM ===");
    g_SpawnManager = new SimpleSpawnManager();
}

void UpdateMutantSpawnSystem(float timeslice)
{
    if (g_SpawnManager)
    {
        g_SpawnManager.Update(timeslice);
    }
}

void ShutdownMutantSpawnSystem()
{
    if (g_SpawnManager)
    {
        Print("[MutantWorld] Shutting down spawn system...");
        g_SpawnManager = null;
    }
}

// ============= PART 4: DEBUG COMMANDS =============

void TestSpawnManagerStatus()
{
    Print("[DEBUG] === SPAWN MANAGER STATUS ===");
    
    if (!g_SpawnManager)
    {
        Print("[DEBUG] ERROR: Spawn Manager not initialized!");
        return;
    }
    
    g_SpawnManager.PrintStatus();
}

void ForceSpawnInNearestZone()
{
    Print("[DEBUG] === FORCING SPAWN IN NEAREST ZONE ===");
    
    if (!g_SpawnManager)
    {
        Print("[DEBUG] ERROR: Spawn Manager not initialized!");
        return;
    }
    
    array<Man> players = new array<Man>;
    GetGame().GetPlayers(players);
    
    if (players.Count() == 0)
    {
        Print("[DEBUG] No players online!");
        return;
    }
    
    Man player = players.Get(0);
    vector playerPos = player.GetPosition();
    
    string nearestZoneName = "";
    float nearestDistance = 999999.0;
    ref SimpleZone nearestZone = null;
    
    for (int i = 0; i < g_SpawnManager.m_ZonesMap.Count(); i++)
    {
        string zoneName = g_SpawnManager.m_ZonesMap.GetKey(i);
        ref SimpleZone zone = g_SpawnManager.m_ZonesMap.GetElement(i);
        
        if (zone && zone.enabled)
        {
            float distance = vector.Distance(playerPos, zone.position);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                nearestZoneName = zoneName;
                nearestZone = zone;
            }
        }
    }
    
    if (nearestZone)
    {
        Print("[DEBUG] Nearest zone: " + nearestZoneName + " at " + nearestDistance.ToString() + "m");
        
        // Clear zone status
        g_SpawnManager.m_ZoneCooldowns.Set(nearestZoneName, 0.0);
        g_SpawnManager.m_ZoneSpawnedStatus.Set(nearestZoneName, false);
        g_SpawnManager.m_ZoneRolledChance.Set(nearestZoneName, false);
        
        // Clear existing entities in all spawn points
        for (int j = 0; j < nearestZone.spawnPoints.Count(); j++)
        {
            ref SimpleSpawnPoint point = nearestZone.spawnPoints.Get(j);
            if (point)
            {
                point.spawnedEntities.Clear();
            }
        }
        
        // Clear zone entity list
        ref array<EntityAI> entities;
        if (g_SpawnManager.m_SpawnedEntities.Find(nearestZoneName, entities))
        {
            entities.Clear();
        }
        
        // Set spawn chance to 100% temporarily for forced spawn
        float originalChance = nearestZone.spawnChance;
        nearestZone.spawnChance = 1.0;
        
        // Force spawn
        g_SpawnManager.CheckAndSpawnInZone(nearestZone, players);
        
        // Restore original spawn chance
        nearestZone.spawnChance = originalChance;
    }
    else
    {
        Print("[DEBUG] No zones found!");
    }
}

void ClearAllSpawnedEntities()
{
    Print("[DEBUG] === CLEARING ALL SPAWNED ENTITIES ===");
    
    if (!g_SpawnManager)
    {
        Print("[DEBUG] ERROR: Spawn Manager not initialized!");
        return;
    }
    
    int totalCleared = 0;
    
    // Clear all zones
    for (int i = 0; i < g_SpawnManager.m_ZonesMap.Count(); i++)
    {
        string zoneName = g_SpawnManager.m_ZonesMap.GetKey(i);
        ref SimpleZone zone = g_SpawnManager.m_ZonesMap.GetElement(i);
        
        if (zone)
        {
            // Clear spawn point entities
            for (int j = 0; j < zone.spawnPoints.Count(); j++)
            {
                ref SimpleSpawnPoint point = zone.spawnPoints.Get(j);
                if (point)
                {
                    for (int k = 0; k < point.spawnedEntities.Count(); k++)
                    {
                        EntityAI entity = point.spawnedEntities.Get(k);
                        if (entity)
                        {
                            entity.Delete();
                            totalCleared++;
                        }
                    }
                    point.spawnedEntities.Clear();
                }
            }
        }
        
        // Clear zone entity list
        ref array<EntityAI> entities;
        if (g_SpawnManager.m_SpawnedEntities.Find(zoneName, entities))
        {
            entities.Clear();
        }
        
        // Reset zone status
        g_SpawnManager.m_ZoneSpawnedStatus.Set(zoneName, false);
        g_SpawnManager.m_ZoneCooldowns.Set(zoneName, 0.0);
        g_SpawnManager.m_ZoneRolledChance.Set(zoneName, false);
    }
    
    Print("[DEBUG] Cleared " + totalCleared.ToString() + " entities total");
}

// Quick Commands
void QuickStatus() { TestSpawnManagerStatus(); }
void QuickReload() { if (g_SpawnManager) g_SpawnManager.ForceReload(); }
void QuickToggle() { if (g_SpawnManager) g_SpawnManager.ToggleSystem(); }
void QuickSpawn() { ForceSpawnInNearestZone(); }
void QuickClear() { ClearAllSpawnedEntities(); }

// Main Debug Entry Points
void DebugSpawnSystem()
{
    Print("[DEBUG] === SPAWN SYSTEM DEBUG ===");
    Print("[DEBUG] g_SpawnManager exists: " + (g_SpawnManager != null).ToString());
    
    if (g_SpawnManager)
    {
        g_SpawnManager.PrintStatus();
    }
}

void ReloadSpawnSystem()
{
    Print("[DEBUG] === RELOADING SPAWN SYSTEM ===");
    
    if (g_SpawnManager)
    {
        g_SpawnManager.ForceReload();
    }
}

void ToggleSpawnSystem()
{
    Print("[DEBUG] === TOGGLING SPAWN SYSTEM ===");
    
    if (g_SpawnManager)
    {
        g_SpawnManager.ToggleSystem();
    }
}

// ============= PART 5: AUTO-INIT (WITHOUT MISSIONSERVER) =============

// Auto-Init when mod loads
void MutantSpawnSystem_AutoInit()
{
    Print("[MutantSpawn] === AUTO-INIT STARTING ===");
    
    // Wait until game is ready
    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(MutantSpawnSystem_DelayedInit, 10000, false);
}

void MutantSpawnSystem_DelayedInit()
{
    Print("[MutantSpawn] === DELAYED INIT STARTING ===");
    
    // Initialize the spawn system
    InitMutantSpawnSystem();
    
    // Start the update loop
    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(MutantSpawnSystem_UpdateLoop, 1000, true);
    
    Print("[MutantSpawn] === SPAWN SYSTEM ACTIVE ===");
}

void MutantSpawnSystem_UpdateLoop()
{
    UpdateMutantSpawnSystem(1.0);
}

// Automatic start when loading
class MutantSpawnSystemAutoStart
{
    void MutantSpawnSystemAutoStart()
    {
        Print("[MutantSpawn] === AUTO-START CONSTRUCTOR ===");
        MutantSpawnSystem_AutoInit();
    }
}

// Create global instance for auto-start
static ref MutantSpawnSystemAutoStart g_MutantSpawnSystemAutoStart = new MutantSpawnSystemAutoStart();