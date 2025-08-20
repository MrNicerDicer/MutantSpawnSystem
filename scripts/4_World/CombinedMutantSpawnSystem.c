// ============= CombinedMutantSpawnSystem_Optimized.c =============
// Optimierte Version mit intelligentem Spatial Hashing
// Bessere Performance als reines Polling, funktioniert aber zuverlässig

// ============= PART 1: CLASS DEFINITIONS (UNCHANGED) =============

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
    
    void GlobalSettingsConfig()
    {
        systemEnabled = true;
        checkInterval = 5.0;  // Faster checks but optimized
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
    
    // Optimization fields
    float cooldownTime;
    bool hasSpawned;
    bool hasRolledChance;
    ref array<Man> playersInside;
    
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
    }
}

// ============= PART 2: OPTIMIZED SPAWN MANAGER =============

class SimpleSpawnManager : Managed
{
    bool m_SystemEnabled;
    float m_CheckTimer;
    float m_CheckInterval;
    float m_MinSpawnDistanceFromPlayer;
    
    ref map<int, ref SimpleTier> m_Tiers;
    ref map<string, ref SimpleZone> m_ZonesMap;
    ref GlobalSettingsConfig m_GlobalSettings;
    
    // Spatial optimization
    ref map<int, ref array<ref SimpleZone>> m_ZoneGrid;  // Grid-based zone lookup
    static const int GRID_SIZE = 1000;  // 1km grid cells
    
    void SimpleSpawnManager()
    {
        m_SystemEnabled = true;
        m_CheckTimer = 0.0;
        m_CheckInterval = 5.0;  // Check more often but smarter
        m_MinSpawnDistanceFromPlayer = 30.0;
        
        m_Tiers = new map<int, ref SimpleTier>;
        m_ZonesMap = new map<string, ref SimpleZone>;
        m_ZoneGrid = new map<int, ref array<ref SimpleZone>>;
        m_GlobalSettings = new GlobalSettingsConfig;
        
        Print("[SpawnManager] === OPTIMIZED SPAWN SYSTEM v2.0 INITIALIZED ===");
        
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
        container.globalSettings.checkInterval = 5.0;
        container.globalSettings.maxEntitiesPerZone = 20;
        container.globalSettings.entityLifetime = 600;
        container.globalSettings.minSpawnDistanceFromPlayer = 30.0;
        
        // Zone 1: Urban Building Example
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
        
        // Zone 2: Underground Bunker Example
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
        Print("[SpawnManager] Created default Zones.json with 2 example zones");
    }
    
    void LoadAllConfigs()
    {
        Print("[SpawnManager] === LOADING CONFIGURATIONS ===");
        
        m_Tiers.Clear();
        m_ZonesMap.Clear();
        m_ZoneGrid.Clear();
        
        LoadTiers();
        LoadZones();
        BuildSpatialGrid();
        
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
                m_SystemEnabled = m_GlobalSettings.systemEnabled;
                m_CheckInterval = m_GlobalSettings.checkInterval;
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
                        
                        Print("[SpawnManager] Loaded zone: " + zone.name);
                        Print("[SpawnManager]   Spawn chance: " + (zone.spawnChance * 100).ToString() + "%");
                        Print("[SpawnManager]   Spawn points: " + zone.spawnPoints.Count().ToString() + " (Total entities: " + totalEntities.ToString() + ")");
                    }
                }
            }
        }
    }
    
    void BuildSpatialGrid()
    {
        // Build spatial grid for optimized zone lookups
        for (int i = 0; i < m_ZonesMap.Count(); i++)
        {
            ref SimpleZone zone = m_ZonesMap.GetElement(i);
            if (zone)
            {
                // Calculate grid cells that this zone overlaps
                float radius = zone.triggerRadius + zone.despawnDistance;
                int minX = Math.Floor((zone.position[0] - radius) / GRID_SIZE);
                int maxX = Math.Floor((zone.position[0] + radius) / GRID_SIZE);
                int minZ = Math.Floor((zone.position[2] - radius) / GRID_SIZE);
                int maxZ = Math.Floor((zone.position[2] + radius) / GRID_SIZE);
                
                // Add zone to all overlapping grid cells
                for (int x = minX; x <= maxX; x++)
                {
                    for (int z = minZ; z <= maxZ; z++)
                    {
                        int gridKey = (x * 10000) + z;  // Simple hash
                        
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
        }
        
        Print("[SpawnManager] Built spatial grid with " + m_ZoneGrid.Count().ToString() + " cells");
    }
    
    void Update(float timeslice)
    {
        if (!m_SystemEnabled) return;
        
        m_CheckTimer += timeslice;
        
        // Update zone cooldowns
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
        
        // Check zones
        if (m_CheckTimer >= m_CheckInterval)
        {
            m_CheckTimer = 0.0;
            CheckPlayersOptimized();
        }
    }
    
    void CheckPlayersOptimized()
    {
        if (m_Tiers.Count() == 0 || m_ZonesMap.Count() == 0)
            return;
        
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        if (players.Count() == 0)
            return;
        
        // Track which zones have been checked
        ref set<ref SimpleZone> checkedZones = new set<ref SimpleZone>;
        
        // For each player, only check zones in nearby grid cells
        for (int p = 0; p < players.Count(); p++)
        {
            Man player = players.Get(p);
            if (!player || !player.IsAlive())
                continue;
            
            vector playerPos = player.GetPosition();
            
            // Get grid cell for player
            int gridX = Math.Floor(playerPos[0] / GRID_SIZE);
            int gridZ = Math.Floor(playerPos[2] / GRID_SIZE);
            
            // Check surrounding grid cells (3x3 area)
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
                            
                            // Skip if already checked
                            if (checkedZones.Find(zone) != -1)
                                continue;
                            
                            checkedZones.Insert(zone);
                            
                            if (!zone || !zone.enabled)
                                continue;
                            
                            // Check this zone against all players
                            CheckZone(zone, players);
                        }
                    }
                }
            }
        }
    }
    
    void CheckZone(ref SimpleZone zone, array<Man> players)
    {
        array<Man> currentPlayersInside = new array<Man>;
        float closestPlayerDistance = 999999.0;
        
        // Check which players are in zone
        for (int i = 0; i < players.Count(); i++)
        {
            Man player = players.Get(i);
            if (player && player.IsAlive())
            {
                float distSq = vector.DistanceSq(player.GetPosition(), zone.position);
                float dist = Math.Sqrt(distSq);
                
                if (dist < closestPlayerDistance)
                    closestPlayerDistance = dist;
                
                if (dist <= zone.triggerRadius)
                {
                    currentPlayersInside.Insert(player);
                }
            }
        }
        
        // Check for state changes
        bool wasEmpty = (zone.playersInside.Count() == 0);
        bool isEmpty = (currentPlayersInside.Count() == 0);
        
        // Players entered
        if (wasEmpty && !isEmpty)
        {
            zone.playersInside = currentPlayersInside;
            TrySpawnInZone(zone, currentPlayersInside);
        }
        // Players left
        else if (!wasEmpty && isEmpty)
        {
            zone.playersInside.Clear();
            if (zone.despawnOnExit && closestPlayerDistance > zone.despawnDistance)
            {
                DespawnFromZone(zone);
            }
        }
        // Update player list
        else
        {
            zone.playersInside = currentPlayersInside;
        }
    }
    
    void TrySpawnInZone(ref SimpleZone zone, array<Man> playersInZone)
    {
        // Check cooldown
        if (zone.cooldownTime > 0)
            return;
        
        // Check if already spawned
        if (zone.hasSpawned)
            return;
        
        // Roll spawn chance
        if (!zone.hasRolledChance)
        {
            float roll = Math.RandomFloat(0.0, 1.0);
            zone.hasRolledChance = true;
            
            if (roll > zone.spawnChance)
            {
                Print("[SpawnManager] Zone " + zone.name + " failed spawn chance");
                zone.cooldownTime = zone.respawnCooldown;
                return;
            }
        }
        
        if (!zone.spawnPoints || zone.spawnPoints.Count() == 0)
            return;
        
        CleanupSpawnPointEntities(zone);
        
        int currentTotal = CountZoneEntities(zone);
        if (currentTotal > 0)
            return;
        
        int totalSpawned = 0;
        
        for (int i = 0; i < zone.spawnPoints.Count(); i++)
        {
            ref SimpleSpawnPoint point = zone.spawnPoints.Get(i);
            
            if (!IsSpawnPointSafe(point, playersInZone))
                continue;
            
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
            zone.hasSpawned = true;
            Print("[SpawnManager] Spawned " + totalSpawned.ToString() + " entities in zone " + zone.name);
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
        
        if (point.useFixedHeight)
        {
            spawnPos[1] = point.position[1] + 0.5;
        }
        else
        {
            spawnPos[1] = GetGame().SurfaceY(spawnPos[0], spawnPos[2]) + 0.5;
        }
        
        EntityAI entity = EntityAI.Cast(GetGame().CreateObject(entityClass, spawnPos, false, true, true));
        
        if (entity)
        {
            entity.SetHealth("", "", entity.GetMaxHealth("", ""));
            
            point.spawnedEntities.Insert(entity);
            
            if (m_GlobalSettings.entityLifetime > 0)
            {
                GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CleanupEntity, m_GlobalSettings.entityLifetime * 1000, false, entity);
            }
            
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
        
        if (totalDespawned > 0)
        {
            zone.cooldownTime = zone.respawnCooldown;
            zone.hasSpawned = false;
            zone.hasRolledChance = false;
            Print("[SpawnManager] Despawned " + totalDespawned.ToString() + " entities from zone " + zone.name);
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
                
                if (CountZoneEntities(zone) == 0 && zone.hasSpawned)
                {
                    zone.cooldownTime = zone.respawnCooldown;
                    zone.hasSpawned = false;
                    zone.hasRolledChance = false;
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
        Print("[SpawnManager] Grid cells: " + m_ZoneGrid.Count().ToString());
        
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
                    Print("[SpawnManager] Zone: " + zone.name + " [ACTIVE]");
                    Print("[SpawnManager]   Players inside: " + zone.playersInside.Count().ToString());
                }
                else
                {
                    Print("[SpawnManager] Zone: " + zone.name);
                }
                
                Print("[SpawnManager]   Entities: " + zoneEntityCount.ToString() + "/" + zonePossibleEntities.ToString());
                
                if (zone.cooldownTime > 0)
                {
                    Print("[SpawnManager]   Cooldown: " + zone.cooldownTime.ToString() + "s");
                }
            }
        }
        
        Print("[SpawnManager] === TOTALS ===");
        Print("[SpawnManager] Active zones: " + zonesWithPlayers.ToString() + "/" + m_ZonesMap.Count().ToString());
        Print("[SpawnManager] Total entities: " + totalActiveEntities.ToString() + "/" + totalPossibleEntities.ToString());
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
    
    Print("[MutantWorld] === INITIALIZING OPTIMIZED SPAWN SYSTEM ===");
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
        
        // Reset zone state
        nearestZone.cooldownTime = 0;
        nearestZone.hasSpawned = false;
        nearestZone.hasRolledChance = false;
        nearestZone.spawnChance = 1.0;  // Force 100% chance
        
        // Clear existing entities
        for (int j = 0; j < nearestZone.spawnPoints.Count(); j++)
        {
            ref SimpleSpawnPoint point = nearestZone.spawnPoints.Get(j);
            if (point)
            {
                point.spawnedEntities.Clear();
            }
        }
        
        // Force spawn
        array<Man> forcePlayers = new array<Man>;
        forcePlayers.Insert(player);
        g_SpawnManager.TrySpawnInZone(nearestZone, forcePlayers);
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
    
    for (int i = 0; i < g_SpawnManager.m_ZonesMap.Count(); i++)
    {
        string zoneName = g_SpawnManager.m_ZonesMap.GetKey(i);
        ref SimpleZone zone = g_SpawnManager.m_ZonesMap.GetElement(i);
        
        if (zone)
        {
            g_SpawnManager.DespawnFromZone(zone);
            zone.cooldownTime = 0;
            zone.hasSpawned = false;
            zone.hasRolledChance = false;
        }
    }
    
    Print("[DEBUG] Cleared all entities");
}

// Quick Commands
void QuickStatus() { TestSpawnManagerStatus(); }
void QuickReload() { if (g_SpawnManager) g_SpawnManager.ForceReload(); }
void QuickToggle() { if (g_SpawnManager) g_SpawnManager.ToggleSystem(); }
void QuickSpawn() { ForceSpawnInNearestZone(); }
void QuickClear() { ClearAllSpawnedEntities(); }

// ============= PART 5: AUTO-INIT =============

void MutantSpawnSystem_AutoInit()
{
    Print("[MutantSpawn] === AUTO-INIT STARTING ===");
    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(MutantSpawnSystem_DelayedInit, 10000, false);
}

void MutantSpawnSystem_DelayedInit()
{
    Print("[MutantSpawn] === DELAYED INIT STARTING ===");
    InitMutantSpawnSystem();
    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(MutantSpawnSystem_UpdateLoop, 1000, true);
    Print("[MutantSpawn] === OPTIMIZED SPAWN SYSTEM ACTIVE ===");
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