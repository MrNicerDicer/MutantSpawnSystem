# MZSS 2.0: Server Owner ReadMe

If you are reading this file, MZSS is probably already installed on your server and you now want to configure it, update it, fix something or understand what all the new files do.

This ReadMe is written from that point of view.

MZSS is a server-side spawn system for DayZ. It can spawn zombies, mutants, animals and other spawnable AI or creature classnames through JSON configuration.

The main mod is server-side only. Players do not need to install the main MZSS mod.

---

## First Check: Did MZSS Load Correctly?

After starting the server, check your script log.

You should see something like:

```text
[MZSS] === LOADING SYSTEM ===
[MZSS] Configuration loaded:
[MZSS] - Settings source: MZSS_Settings.json
[MZSS] - Tiers source: MZSS_Tiers.json
[MZSS] - Zones source: MZSS_Zones.json
[MZSS] - Tiers: 34
[MZSS] - Zones: 468
[MZSS] === SYSTEM READY ===
```

If you see `SYSTEM READY`, the main system loaded.

If something is wrong, search your script log for:

```text
[MZSS]
[MZSS:LOAD]
[MZSS:VALIDATOR]
[MZSS:SPAWN]
```

---

## Where Are The Config Files?

MZSS uses this folder:

```text
$profile:MutantSpawnSystem/
```

This folder name is kept for legacy compatibility.

The main files are:

```text
MZSS_Settings.json
MZSS_Tiers.json
MZSS_Zones.json
```

Optional systems use:

```text
MZSS_PlayerRules.json
MZSS_AntiClipping.json
MZSS_Migration.json
MZSS_Nests.json
MZSS_Notifications.json
MZSS_WorldState.json
MZSS_Cooldowns.json
```

---

## Important Update Note

In MZSS 2.0, the PBO name changed from:

```text
MutantSpawnSystem.pbo
```

to:

```text
MZSS.pbo
```

If you update through Steam Workshop or server managers like Omega Manager, the old PBO should normally be replaced automatically.

If you update manually, make sure you do not run both PBOs at the same time.

Do not keep this:

```text
MutantSpawnSystem.pbo
MZSS.pbo
```

Only keep:

```text
MZSS.pbo
```

Running both can cause compile errors, duplicated scripts or strange behavior.

---

## If You Updated From The Old Version

Old files are still supported:

```text
SpawnerSettings.json
Tiers.json
Zones.json
```

If the new files do not exist yet, MZSS can automatically create them from the old files:

```text
MZSS_Settings.json
MZSS_Tiers.json
MZSS_Zones.json
```

Your old files are not deleted.

After updating, you should edit the new files:

```text
MZSS_Settings.json
MZSS_Tiers.json
MZSS_Zones.json
```

The old files can stay as backup.

---

## The Three Files You Will Use Most

### `MZSS_Settings.json`

Global system behavior.

Use this file for:

- enabling or disabling the system
- debug logs
- spawn budget
- soft spawn interval
- despawn delay
- global entity limit
- cooldown persistence
- living entity despawn protection

---

### `MZSS_Tiers.json`

This file defines what can spawn.

Example:

```json
{
  "tiers": {
    "1": {
      "name": "Basic Zombies",
      "classnames": [
        "ZmbM_CitizenASkinny_Blue",
        "ZmbF_SurvivorNormal_Blue"
      ]
    },
    "2": {
      "name": "Mutants",
      "classnames": [
        "Mutant_Bloodsucker",
        "Mutant_Controller"
      ]
    }
  }
}
```

A tier is just a group of classnames.

---

### `MZSS_Zones.json`

This file defines where and when entities spawn.

Example:

```json
{
  "zones": [
    {
      "name": "Example_Zone",
      "enabled": 1,
      "position": "6560 15 2630",
      "triggerRadius": 100.0,
      "spawnChance": 1.0,
      "despawnOnExit": 1,
      "despawnDistance": 150.0,
      "respawnCooldown": 900.0,
      "spawnPoints": [
        {
          "position": "6555 15 2625",
          "radius": 8.0,
          "tierIds": [1],
          "entities": 4,
          "useFixedHeight": 0
        }
      ]
    }
  ]
}
```

A zone activates when a player enters its trigger radius.

The spawn points inside the zone decide where entities actually spawn.

---

## Common Tasks

### I Want To Add A New Spawn Area

1. Add or choose a tier in `MZSS_Tiers.json`.
2. Add a new zone in `MZSS_Zones.json`.
3. Add at least one spawn point.
4. Set `tierIds` on the spawn point.
5. Restart the server or hot reload.
6. Walk into the trigger radius and test.

---

### I Want To Spawn Mutants In A Bunker

Use fixed height on the spawn point:

```json
"useFixedHeight": 1
```

This tells MZSS to use the exact Y coordinate instead of terrain height.

This is important for:

- bunkers
- rooftops
- underground areas
- multi-floor buildings
- custom interiors

---

### I Want A Rare Encounter

Set a lower spawn chance on the zone:

```json
"spawnChance": 0.25
```

Examples:

```text
0.0 = never
0.25 = 25 percent chance
0.5 = 50 percent chance
1.0 = always
```

---

### I Want A Zone To Respawn Later

Use:

```json
"respawnCooldown": 1800.0
```

This value is in seconds.

Example:

```text
300 = 5 minutes
900 = 15 minutes
1800 = 30 minutes
3600 = 1 hour
```

---

### I Do Not Want AI To Despawn In Front Of Players

Check these settings in `MZSS_Settings.json`:

```json
"protectLivingEntitiesFromDespawn": 1,
"livingEntityDespawnProtectionDistance": 80.0
```

This helps when zombies or mutants chase players away from the original zone center.

---

## Optional Systems

All of these systems are optional. If you do not need them, leave them disabled.

---

## Player Rules

File:

```text
MZSS_PlayerRules.json
```

Use this when certain players should not trigger zones or should be ignored by MZSS-spawned AI.

Example:

```json
{
  "enabled": 1,
  "players": [
    {
      "steamId": "76561198000000000",
      "name": "Example Player",
      "canTriggerZones": 0,
      "ignoredBySpawnedAI": 1
    }
  ]
}
```

Useful for:

- special RP factions
- event characters
- admins
- testing
- groups that should not be attacked by MZSS-spawned AI

---

## Anti-Clipping

File:

```text
MZSS_AntiClipping.json
```

Use this if entities spawn inside walls, editor objects or custom structures.

Recommended starting point:

```json
{
  "enabled": 1,
  "groundRaycast": 1,
  "clearanceCheck": 1,
  "retryAttempts": 8,
  "retryRadius": 2.5,
  "minWallDistance": 0.8,
  "verticalProbeUp": 1.5,
  "verticalProbeDown": 5.0,
  "fallbackToOriginalPosition": 1
}
```

If spawns fail too often, make the settings less strict.

If entities still spawn inside walls, increase `retryAttempts` or move your spawn points slightly.

---

## Zone Notifications

File:

```text
MZSS_Notifications.json
```

Use this when a player should get a personal warning message after entering a zone.

This is not global chat. Only the player entering the zone sees the message.

Example:

```json
{
  "enabled": 1,
  "mode": "personalMessage",
  "defaultCooldown": 900.0,
  "notifications": [
    {
      "zoneName": "Zone_156",
      "enabled": 1,
      "trigger": "onEnter",
      "title": "Warning",
      "message": "Be careful, dangerous zombies ahead.",
      "cooldown": 900.0
    }
  ]
}
```

---

## Nests

File:

```text
MZSS_Nests.json
```

Nests are special hotspots.

They are good for:

- mutant nests
- infected houses
- bunker infestations
- RP events
- boss-style areas

Example:

```json
{
  "enabled": 1,
  "nests": [
    {
      "name": "Bloodsucker_Nest_OldHouse",
      "enabled": 1,
      "nestClassname": "Land_House_1W01",
      "position": "3810.685303 123.246178 6968.397461",
      "radius": 35.0,
      "activationChance": 0.35,
      "cooldown": 7200.0,
      "activeDuration": 1800.0,
      "triggerMode": "playerNear",
      "clearMode": "killAllSpawnedOrTimer",
      "spawnClassnames": [
        "Mutant_Bloodsucker"
      ],
      "minEntities": 2,
      "maxEntities": 5,
      "spawnRadius": 12.0,
      "useFixedHeight": 0,
      "notificationEnabled": 1,
      "notificationTitle": "Warning",
      "notificationMessage": "Something is moving nearby...",
      "notificationCooldown": 900.0
    }
  ]
}
```

Clear modes:

```text
killAllSpawned
timer
killAllSpawnedOrTimer
```

Recommended:

```text
killAllSpawnedOrTimer
```

This prevents nests from getting stuck forever.

---

## Migration

File:

```text
MZSS_Migration.json
```

Migration is a lightweight A-Life style system.

It does not physically move AI across the map. Instead, one zone can influence another zone.

Example:

```json
{
  "enabled": 1,
  "links": [
    {
      "name": "Zone_156_to_Zone_157",
      "enabled": 1,
      "sourceZoneNames": ["Zone_156"],
      "targetZoneNames": ["Zone_157"],
      "trigger": "sourceCleared",
      "chance": 0.35,
      "cooldown": 3600.0,
      "targetSelection": "randomOne",
      "effectDuration": 1800.0,
      "targetSpawnChanceBonus": 0.25,
      "targetCooldownReduction": 900.0,
      "forceTargetReady": 0
    }
  ]
}
```

Supported triggers:

```text
sourceActivated
sourceCleared
sourceDespawned
```

Simple explanation:

```text
A source zone gets activated, cleared or despawned.
MZSS rolls a chance.
A target zone can temporarily become more active.
```

---

## World State

File:

```text
MZSS_WorldState.json
```

This is not a normal config file.

It stores runtime data for systems like:

- nest cooldowns
- active nest states
- migration cooldowns
- active migration effects

You usually do not need to edit it.

If nests or migration feel stuck, stop the server and delete:

```text
MZSS_WorldState.json
```

MZSS will recreate it.

---

## Cooldowns

File:

```text
MZSS_Cooldowns.json
```

This file is used if persistent cooldowns are enabled in `MZSS_Settings.json`.

If you want to reset all zone cooldowns:

1. Stop the server.
2. Delete `MZSS_Cooldowns.json`.
3. Start the server again.

---

## Optional Admin Tool

MZSS can be used without the Admin Tool.

The Admin Tool is a separate optional client/server addon for in-game editing.

It can help with:

- creating zones
- moving zones
- adding spawn points
- editing tiers
- previewing zones and spawn points
- saving changes
- hot reload
- force spawning nearby zones for testing

Important:

The main MZSS mod is server-side only. The Admin Tool has UI, so it is not server-side only.

Only SteamIDs listed in the Admin Tool admin file can open and use it.

---

## Supported Entities

MZSS can spawn vanilla infected, animals, modded mutants and other spawnable AI classnames.

MZSS does not include creatures by itself.

It uses classnames from DayZ or from other mods loaded on your server.

If a modded creature does not spawn, check:

- the classname spelling
- whether the creature mod is loaded server-side
- whether the classname is directly spawnable
- whether the mod uses its own AI or loadout spawn system

---

## Troubleshooting

### Nothing spawns

Check:

- Is MZSS loaded?
- Is the zone enabled?
- Is the player inside `triggerRadius`?
- Is the zone on cooldown?
- Did `spawnChance` fail?
- Does the spawn point have valid `tierIds`?
- Does the tier exist?
- Does the tier contain valid classnames?
- Is the creature mod loaded server-side?
- Is anti-clipping blocking the spawn?

---

### The zone loaded, but the wrong creature spawned

Check:

- the `tierIds` on the spawn point
- the classnames inside that tier
- whether multiple tiers are assigned to the same spawn point

---

### Entities spawn underground or on terrain instead of inside a bunker

Use:

```json
"useFixedHeight": 1
```

Also make sure the Y coordinate is correct.

---

### Entities despawn too early

Check:

```json
"despawnOnExit": 1,
"despawnDistance": 150.0,
"despawnDelay": 45.0,
"protectLivingEntitiesFromDespawn": 1,
"livingEntityDespawnProtectionDistance": 80.0
```

Increase `despawnDistance` or `livingEntityDespawnProtectionDistance` if needed.

---

### Nests trigger but spawn nothing

Check the logs for:

```text
[MZSS:NEST]
```

Most common reasons:

- wrong classname
- classname is not directly spawnable
- required mod is not loaded server-side
- anti-clipping rejected the position
- fixed height Y coordinate is wrong

Try testing with a vanilla infected first:

```text
ZmbM_CitizenASkinny_Blue
```

If vanilla works, the nest system works and the issue is the custom classname or its mod.

---

### Migration or nests are still on cooldown after restart

Check:

```text
MZSS_WorldState.json
```

Delete it while the server is stopped if you want to reset runtime state.

---

## Performance Tips

- Use reasonable trigger radii.
- Avoid too many high-count zones close together.
- Use soft spawning.
- Use the spawn budget system.
- Set a global entity limit if needed.
- Keep respawn cooldowns meaningful.
- Do not spawn huge amounts of AI in one small area.
- Test large setups with multiple players.

---

## Support

If you need help, join the Exodus DayZ Forge Discord:

```text
https://discord.gg/Yz8S558D6M
```

When asking for help, include:

- server script log
- your `MZSS_Settings.json`
- the relevant zone from `MZSS_Zones.json`
- the relevant tier from `MZSS_Tiers.json`
- the classname that does not spawn
- whether the issue happens after restart or hot reload

---

## Credits

Created by:
MrNicerDicer

Community:
Exodus DayZ Forge

Support development:

https://ko-fi.com/mrnicerdicer

MZSS is free to use and free to repack.
If you redistribute modified versions, please credit the original author.

---

## Compatibility

DayZ 1.29+  
All maps supported  
