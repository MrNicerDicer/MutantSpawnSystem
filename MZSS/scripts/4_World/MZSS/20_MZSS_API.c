ref SimpleSpawnManager g_SpawnManager;

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
    Print("[MZSS:DEBUG] === SPAWN MANAGER STATUS ===");
    
    if (!g_SpawnManager)
    {
        Print("[MZSS:DEBUG] ERROR: Spawn Manager not initialized!");
        return;
    }
    
    g_SpawnManager.PrintStatus();
}

void ForceSpawnInNearestZone()
{
    Print("[MZSS:DEBUG] === FORCING SPAWN IN NEAREST ZONE ===");

    if (!g_SpawnManager)
    {
        Print("[MZSS:DEBUG] ERROR: Spawn Manager not initialized!");
        return;
    }

    array<Man> players = new array<Man>;
    GetGame().GetPlayers(players);

    if (players.Count() == 0)
    {
        Print("[MZSS:DEBUG] No players online!");
        return;
    }

    Man player = players.Get(0);
    if (!player)
    {
        Print("[MZSS:DEBUG] First player reference is null!");
        return;
    }

    g_SpawnManager.ForceSpawnNearestAtPosition(player.GetPosition());
}

void MZSS_AdminHotReload()
{
    if (!g_SpawnManager)
    {
        Print("[MZSS API] ERROR: Spawn Manager not initialized!");
        return;
    }

    g_SpawnManager.AdminHotReload();
}

void MZSS_AdminForceSpawnNearestAtPosition(string position)
{
    if (!g_SpawnManager)
    {
        Print("[MZSS API] ERROR: Spawn Manager not initialized!");
        return;
    }

    position.Replace(",", " ");
    position.TrimInPlace();
    vector sourcePos = position.ToVector();
    g_SpawnManager.ForceSpawnNearestAtPosition(sourcePos);
}

void MZSS_AdminDiagnoseAtPosition(string position)
{
    if (!g_SpawnManager) return;

    position.Replace(",", " ");
    position.TrimInPlace();
    vector sourcePos = position.ToVector();
    g_SpawnManager.DiagnoseAtPosition(sourcePos);
}

void ClearAllSpawnedEntities()
{
    Print("[MZSS:DEBUG] === CLEARING ALL SPAWNED ENTITIES ===");
    
    if (!g_SpawnManager)
    {
        Print("[MZSS:DEBUG] ERROR: Spawn Manager not initialized!");
        return;
    }
    
    for (int i = 0; i < g_SpawnManager.m_ZonesMap.Count(); i++)
    {
        ref SimpleZone zone = g_SpawnManager.m_ZonesMap.GetElement(i);
        
        if (zone)
        {
            g_SpawnManager.DespawnFromZone(zone);
            zone.cooldownTime = 0;
            zone.hasSpawned = false;
            zone.hasRolledChance = false;
        }
    }
    
    g_SpawnManager.m_DirtyZones.Clear();
    
    Print("[MZSS:DEBUG] Cleared all entities");
}

void ForcePlayerCacheRefresh()
{
    Print("[MZSS:DEBUG] === FORCING PLAYER CACHE REFRESH ===");
    
    if (!g_SpawnManager)
    {
        Print("[MZSS:DEBUG] ERROR: Spawn Manager not initialized!");
        return;
    }
    
    g_SpawnManager.RefreshPlayerCache();
    Print("[MZSS:DEBUG] Player cache refreshed");
}

// Quick Commands
void QuickStatus() { TestSpawnManagerStatus(); }
void QuickReload() { MZSS_AdminHotReload(); }
void QuickToggle() { if (g_SpawnManager) g_SpawnManager.ToggleSystem(); }
void QuickDebug() { if (g_SpawnManager) g_SpawnManager.ToggleDebug(); }
void QuickSpawn() { ForceSpawnInNearestZone(); }
void QuickClear() { ClearAllSpawnedEntities(); }
void QuickCache() { ForcePlayerCacheRefresh(); }

// ============= PART 5: AUTO-INIT =============

void MutantSpawnSystem_AutoInit()
{
    Print("[MZSS] === AUTO-INIT STARTING ===");
    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(MutantSpawnSystem_DelayedInit, 10000, false);
}

void MutantSpawnSystem_DelayedInit()
{
    Print("[MZSS] === DELAYED INIT STARTING ===");
    InitMutantSpawnSystem();
    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(MutantSpawnSystem_UpdateLoop, 1000, true);
    Print("[MZSS] === OPTIMIZED SPAWN SYSTEM ACTIVE ===");
}

void MutantSpawnSystem_UpdateLoop()
{
    UpdateMutantSpawnSystem(1.0);
}

class MutantSpawnSystemAutoStart
{
    void MutantSpawnSystemAutoStart()
    {
        Print("[MZSS] === AUTO-START CONSTRUCTOR ===");
        MutantSpawnSystem_AutoInit();
    }
}

static ref MutantSpawnSystemAutoStart g_MutantSpawnSystemAutoStart = new MutantSpawnSystemAutoStart();
