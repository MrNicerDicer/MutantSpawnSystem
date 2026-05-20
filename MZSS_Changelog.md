# MZSS 2.0 Changelog

MZSS 2.0 is a major foundation update for the Mutant & Zombie Spawn System.

The goal of this update is to keep the old workflow familiar while making the system cleaner, safer, easier to maintain and ready for more advanced server setups.

---

## Important

### PBO Rename

The PBO name changed from:

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

---

## Legacy Support

Existing legacy files are still supported:

```text
SpawnerSettings.json
Tiers.json
Zones.json
```

MZSS can automatically create the new preferred files:

```text
MZSS_Settings.json
MZSS_Tiers.json
MZSS_Zones.json
```

The old files are not deleted.

This means existing servers do not need to rebuild their entire setup.

---

## New File Names

MZSS now uses cleaner file names:

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

## Internal Cleanup

The old single-file script structure has been split into a cleaner internal structure.

This makes the system easier to maintain and expand.

The goal was to improve the foundation without changing the basic way server owners use MZSS.

---

## New: Config Validation

MZSS now validates configs while loading.

It can warn about common problems like:

- missing tiers
- empty classnames
- zones without spawn points
- invalid trigger radius values
- missing or invalid tier IDs
- invalid entity counts
- invalid spawn radius values

Validation warnings are printed in the script log.

---

## New: Startup Summary

MZSS now prints a clear startup summary.

Example:

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

This makes it easier to check if the system loaded correctly.

---

## New: Structured Logs

Logs are now easier to read.

Examples:

```text
[MZSS:LOAD]
[MZSS:SPAWN]
[MZSS:DESPAWN]
[MZSS:RELOAD]
[MZSS:VALIDATOR]
[MZSS:ADMIN]
[MZSS:DIAG]
[MZSS:NEST]
[MZSS:MIGRATION]
```

Most debug categories can be controlled through `MZSS_Settings.json`.

---

## Improved: Hot Reload

Hot reload handling has been improved.

The reload flow is now cleaner and works better with the optional Admin Tool.

Reload can refresh:

- settings
- tiers
- zones
- optional systems
- world state
- spatial/grid lookup
- player cache
- runtime logic

---

## New: Admin Command Bridge

A small server-side command bridge was added.

This allows the optional MZSS Admin Tool to communicate with the main MZSS mod without making the Admin Tool required.

The main MZSS mod still works normally without the Admin Tool.

---

## New: Global Entity Limit

A server-wide entity limit can now be configured.

Example:

```json
"maxTotalSpawnedEntities": 0
```

`0` disables the limit.

Any value above `0` limits the total number of living entities spawned by MZSS across the server.

---

## New: Persistent Cooldowns

Zone cooldowns can now optionally survive server restarts.

Settings:

```json
"persistCooldowns": 0,
"cooldownPersistenceInterval": 60.0
```

When enabled, MZSS uses:

```text
MZSS_Cooldowns.json
```

---

## Improved: Despawn Protection

MZSS now has protection against living entities despawning in front of players.

Settings:

```json
"protectLivingEntitiesFromDespawn": 1,
"livingEntityDespawnProtectionDistance": 80.0
```

This helps when zombies or mutants chase players away from the original zone center.

---

## New: Player Rules

New file:

```text
MZSS_PlayerRules.json
```

This allows specific SteamIDs to:

- not trigger MZSS zones
- be ignored by MZSS-spawned AI

Useful for:

- RP factions
- event characters
- admins
- special server concepts

---

## New: Anti-Clipping

New file:

```text
MZSS_AntiClipping.json
```

This helps reduce spawns inside walls, editor objects and custom structures.

Useful for:

- custom bunkers
- underground areas
- DayZ Editor objects
- custom interiors
- tight rooms

---

## New: Zone Notifications

New file:

```text
MZSS_Notifications.json
```

Zone notifications are personal messages shown only to the player entering a configured zone.

They are not global chat messages.

Example:

```text
Warning: Be careful, dangerous zombies ahead.
```

---

## New: Nest System

New file:

```text
MZSS_Nests.json
```

Nests are logical hotspots that can spawn enemies when players get close.

Useful for:

- mutant nests
- infected houses
- bunker infestations
- boss-style areas
- RP events

Supported clear modes:

```text
killAllSpawned
timer
killAllSpawnedOrTimer
```

Nests can also send personal notifications when they activate.

---

## New: Migration System

New file:

```text
MZSS_Migration.json
```

Migration is a lightweight A-Life style system.

It does not physically move AI across the map.

Instead, one zone can influence another zone.

Example:

```text
A source zone gets cleared.
MZSS rolls a chance.
A target zone temporarily becomes more active.
```

Supported triggers:

```text
sourceActivated
sourceCleared
sourceDespawned
```

---

## New: World State

New file:

```text
MZSS_WorldState.json
```

This file stores runtime data for optional systems like:

- nest cooldowns
- active nest states
- migration cooldowns
- active migration effects

It is not meant to be edited like a normal config file.

---

## Improved: Modded Entity Handling

MZSS now has improved handling and logging for modded entities.

If a classname cannot be created, the logs should now be clearer.

MZSS can spawn direct spawnable entity classnames. If a mod uses its own AI/loadout spawn system, the real spawnable classname or a compatibility approach may still be required.

---

## Optional Admin Tool Compatibility

MZSS 2.0 is prepared for the optional MZSS Admin Tool.

The main MZSS mod remains server-side only.

The Admin Tool is a separate optional client/server addon for in-game editing.

---

## Compatibility

- DayZ 1.29+
- All maps supported
- Main MZSS mod is server-side only
- Existing legacy configs are supported
- Optional Admin Tool is separate
