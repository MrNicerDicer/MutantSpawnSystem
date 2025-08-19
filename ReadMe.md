# DayZ Mutant Spawn System v1.0

An advanced JSON-based spawn system for DayZ 1.28 that allows spawning zombies, mutants, and animals in configurable zones.

## 🖥️ Server-Side Only
This mod runs **entirely server-side** - players do **NOT** need to download or install anything. Simply install on your server and it works for all players immediately.

## 🌟 Features

- **JSON-based Configuration** - Easy editing without coding knowledge
- **Flexible Tier System** - Categorize entities by difficulty or type
- **Intelligent Spawn Zones** - Trigger-based activation with configurable parameters
- **Spawn Chances** - Control spawn probability (0-100%)
- **Automatic Despawning** - Entities disappear when players leave zones
- **Cooldown System** - Prevents immediate re-spawning
- **Height Control** - Support for bunkers and multi-story buildings
- **Debug Tools** - Comprehensive debug commands for testing
- **Auto-Initialization** - Works without MissionServer modifications

## 📁 Structure

```
$profile:MutantSpawnSystem/
├── Tiers.json          # Define entity categories
└── Zones.json          # Configure spawn zones and points
```

## ⚙️ Configuration

### Tiers.json - Entity Categories

```json
{
  "tiers": {
    "1": {
      "name": "Basic Zombies",
      "classnames": [
        "ZmbM_HunterOld_Autumn",
        "ZmbF_SurvivorNormal_Blue",
        "ZmbM_CitizenASkinny"
      ]
    },
    "2": {
      "name": "Military Zombies", 
      "classnames": [
        "ZmbM_SoldierNormal",
        "ZmbM_PatrolNormal_PautRev"
      ]
    },
    "3": {
      "name": "Wildlife",
      "classnames": [
        "Animal_UrsusArctos",
        "Animal_CanisLupus_Grey"
      ]
    }
  }
}
```

### Zones.json - Spawn Areas

```json
{
  "globalSettings": {
    "systemEnabled": 1,
    "checkInterval": 15.0,
    "maxEntitiesPerZone": 20,
    "entityLifetime": 600,
    "minSpawnDistanceFromPlayer": 30.0
  },
  "zones": [
    {
      "name": "Example_Zone",
      "enabled": 1,
      "position": "6560 15 2630",
      "triggerRadius": 100,
      "spawnChance": 0.75,
      "despawnOnExit": 1,
      "despawnDistance": 150,
      "respawnCooldown": 300,
      "spawnPoints": [
        {
          "position": "6555 15 2625",
          "radius": 1.5,
          "tierIds": [1, 2],
          "entities": 3,
          "useFixedHeight": 0
        }
      ]
    }
  ]
}
```

## 📋 Parameter Reference

### Global Settings

| Parameter | Description | Default |
|-----------|-------------|---------|
| `systemEnabled` | Enable/disable entire system (1/0) | `1` |
| `checkInterval` | Player check interval (seconds) | `15.0` |
| `maxEntitiesPerZone` | Maximum entities per zone | `20` |
| `entityLifetime` | Auto-despawn time (seconds, 0=disabled) | `600` |
| `minSpawnDistanceFromPlayer` | Minimum distance to players when spawning | `30.0` |

### Zone Configuration

| Parameter | Description | Default |
|-----------|-------------|---------|
| `name` | Unique zone name | - |
| `enabled` | Zone enabled/disabled (1/0) | `1` |
| `position` | Zone center "X Y Z" | - |
| `triggerRadius` | Activation radius around zone center | `300.0` |
| `spawnChance` | Spawn probability (0.0-1.0) | `1.0` |
| `despawnOnExit` | Despawn when all players leave zone (1/0) | `1` |
| `despawnDistance` | Despawn distance | `400.0` |
| `respawnCooldown` | Cooldown until next spawn (seconds) | `300.0` |

### Spawn Point Configuration

| Parameter | Description | Default |
|-----------|-------------|---------|
| `position` | Spawn position "X Y Z" | - |
| `radius` | Random spawn radius around position | `2.0` |
| `tierIds` | Available tier IDs for this point | `[]` |
| `entities` | Number of entities to spawn here | `1` |
| `useFixedHeight` | Use exact Y coordinate for bunkers/buildings (1/0) | `0` |

### Tier Configuration

| Parameter | Description |
|-----------|-------------|
| `name` | Tier designation |
| `classnames` | List of available entity classnames |

## 🎮 Debug Commands

The system provides various debug functions via DayZ console:

```cpp
// Show system status
QuickStatus()
TestSpawnManagerStatus()

// Reload configuration
QuickReload()
ReloadSpawnSystem()

// Toggle system on/off
QuickToggle()
ToggleSpawnSystem()

// Force spawn in nearest zone
QuickSpawn()
ForceSpawnInNearestZone()

// Clear all spawned entities
QuickClear()
ClearAllSpawnedEntities()

// General debug info
DebugSpawnSystem()
```

## 📈 Scalability

- **Tiers**: Unlimited (only limited by server memory)
- **Zones**: Unlimited (performance depends on `checkInterval`)
- **Spawn Points**: Unlimited per zone (limited by `maxEntitiesPerZone`)

**Recommended**: Start with 10-20 zones and scale based on server performance.

## 🚀 Advantages

### 1. **Easy Configuration**
- No programming knowledge required
- JSON format is easy to understand
- Hot-reload during server runtime possible

### 2. **Performance Optimized**
- Intelligent checks only when players nearby
- Automatic cleanup routines
- Configurable check intervals

### 3. **Flexibility**
- Supports all DayZ entity types (zombies, animals, NPCs)
- Individual spawn chances per zone
- Different tier combinations per spawn point

### 4. **Realism**
- Spawn protection prevents spawns directly next to players
- Automatic despawning reduces performance impact
- Cooldown system for realistic respawn times

### 5. **Debugging & Maintenance**
- Comprehensive debug tools
- Detailed logging output
- Live status monitoring

## 🔧 Installation

1. **Install Mod** - Place mod files in your server
2. **Start Server** - System initializes automatically
3. **Configure** - Edit JSON files in `$profile:MutantSpawnSystem/`
4. **Reload** - Use `QuickReload()` for live updates

## ⚠️ Important Notes

### Bunkers & Buildings
For underground areas or multi-story buildings, set `"useFixedHeight": 1` to use exact Y coordinates instead of terrain height.

### Spawn Chances
- `0.0` = Never spawn
- `0.5` = 50% chance
- `1.0` = Always spawn (default)

### Performance Tuning
For servers with many players:
- Increase `checkInterval` (e.g., to 30.0)
- Reduce `maxEntitiesPerZone`
- Use smaller `triggerRadius` values

### Supported Entities
This system can spawn **any living entity** in DayZ, including:
- **Vanilla Zombies**: All `ZmbM_*`, `ZmbF_*` classes
- **Vanilla Animals**: All `Animal_*` classes  
- **Modded Creatures**: Any modded zombies, mutants, animals, or NPCs
- **Custom NPCs**: Player-like entities or custom AI
- **Experimental**: Even vehicles with AI (advanced usage)

**Works with ANY mod** - if it's alive and has a classname, this system can spawn it!

## 📂 Sample Configuration

Download the complete example configuration files from the GitHub repository.

## 🔍 Troubleshooting

### Common Issues

**Entities not spawning:**
- Check if zone is enabled
- Verify spawn chance settings
- Ensure players are within trigger radius
- Check debug output with `QuickStatus()`

**Performance problems:**
- Reduce `checkInterval`
- Lower `maxEntitiesPerZone`
- Remove unused zones
- Optimize spawn point counts

**Bunker spawning issues:**
- Always use `"useFixedHeight": true` for underground
- Test exact coordinates in-game first
- Verify Y-coordinate is above bunker floor

## 📝 Credits & License

**Created by**: [Exodus DayZ Forge](https://discord.gg/pWd8adrm)  
**Support**: Join our Discord for help and updates

This system is free to use and modify. If you adapt or redistribute this code, please provide appropriate credits to the original creator (MrNicerDicer).

---

**Created for DayZ 1.28 | Compatible with all maps | Server-side only**
