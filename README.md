# mod-scroll-system

An [AzerothCore](https://www.azerothcore.org/) module (WotLK 3.3.5a) that adds a small
set of consumable "Scroll of X" items and lets Mages scribe them themselves once they've
mastered **Comprehension**.

## What it does

- **10 scroll items** (stat buffs, a heal, a mana restore, a drink conjure, and a
  Hearthstone-style teleport). Each one casts an existing spell on use and is consumed —
  no new spells, no client patch, just existing in-game effects packaged as scrolls.
- **Mage Comprehension**: from a configurable level onward, Mages can scribe any of the
  scrolls into their own bags with `.scroll create <n>` (optionally for a small gold
  cost). `.scroll list` shows what's scribeable and the effect of each one.
- **Arcane Scribe vendor NPC** (not spawned by default): sells the same scrolls to
  anyone, regardless of class or level, for players who'd rather just buy them.

## Commands

| Command                | Access | Description                                      |
|-------------------------|--------|---------------------------------------------------|
| `.scroll list`           | Player | Lists all scrolls, with effect and (if gated) cost |
| `.scroll create <n>`     | Player | Scribes scroll `<n>` into your bags (Mage + Comprehension level required) |

## Configuration

`conf/mod_scroll_system.conf.dist`:

| Key                                  | Default | Description                                  |
|----------------------------------------|---------|-----------------------------------------------|
| `ScrollSystem.Enable`                  | `1`     | Master on/off switch                          |
| `ScrollSystem.ComprehensionLevel`      | `20`    | Level at which Mages can start scribing       |
| `ScrollSystem.GoldCost`                | `0`     | Copper cost per scroll scribed (0 = free)     |

## Installation

1. Clone into your AzerothCore `modules/` directory and rebuild the worldserver.
2. Apply the SQL update in `data/sql/db-world/updates/` to your world database.
3. (Optional) Spawn the "Arcane Scribe" NPC (creature entry `9005500`) wherever you'd
   like a scroll vendor.

## License

Released under the GNU GPL v2 (or later).
