-- --------------------------------------------------------------------------------------
--  mod-scroll-system
--  10 consumable "Scroll of X" items (entries 9005000-9005009), each casting an existing,
--  verified WotLK 3.3.5a spell on use (a stat buff, a heal, a mana restore, a conjure, or a
--  teleport) and consumed on use - and the "Arcane Scribe" vendor NPC (entry 9005500) that
--  sells all of them to anyone. Mages additionally scribe them for free (or for
--  ScrollSystem.GoldCost) via `.scroll create <n>` once they reach ScrollSystem.
--  ComprehensionLevel - see src/ScrollSystem.cpp.
--
--  Every spell id below is copied from a real item_template row already shipped in this
--  fork's base world DB (data/sql/base/db_world/item_template.sql) - i.e. a spell an
--  existing stock item already casts on this exact server, so it is guaranteed to exist.
--  The source item/entry is noted next to each row. displayid 2616 (the "Scroll of
--  Intellect" roll model) is reused for all ten - an existing, in-client model, no client
--  patch required.
--
--  This file is idempotent: every block deletes its own rows before inserting them, so
--  re-running it (or re-applying the module) is always safe.
-- --------------------------------------------------------------------------------------

-- ----------------------------------------------------------------------------------
--  ITEMS (9005000-9005009)
--  class=0 (Consumable), subclass=4 (Scroll) - matches every real "Scroll of X" item.
--  spelltrigger_1=0 (cast on use), spellcharges_1=-1 (one charge, item is consumed) -
--  the same shape as every real scroll item (e.g. "Scroll of Intellect VI", entry 33458).
--  bonding=0 (not bound) so the vendor-bought copies remain tradeable/sellable.
-- ----------------------------------------------------------------------------------
DELETE FROM `item_template` WHERE `entry` BETWEEN 9005000 AND 9005009;
INSERT INTO `item_template`
    (`entry`, `class`, `subclass`, `name`, `displayid`, `Quality`, `InventoryType`, `ItemLevel`,
     `RequiredLevel`, `BuyPrice`, `SellPrice`, `maxcount`, `stackable`, `bonding`,
     `spellid_1`, `spelltrigger_1`, `spellcharges_1`, `description`)
VALUES
    -- Stat buff scrolls - spell ids from the real "Scroll of X III" items (rank III: a
    -- decent mid-game buff, not the strongest available rank).
    (9005000, 0, 4, 'Scroll of Strength',        2616, 1, 0, 1, 1, 150, 30, 0, 20, 0,
        8120, 0, -1, 'Grants Strength for a time when used.'),   -- spell from Scroll of Strength III (4426)
    (9005001, 0, 4, 'Scroll of Agility',         2616, 1, 0, 1, 1, 150, 30, 0, 20, 0,
        8117, 0, -1, 'Grants Agility for a time when used.'),    -- spell from Scroll of Agility III (4425)
    (9005002, 0, 4, 'Scroll of Intellect',       2616, 1, 0, 1, 1, 150, 30, 0, 20, 0,
        8098, 0, -1, 'Grants Intellect for a time when used.'),  -- spell from Scroll of Intellect III (4419)
    (9005003, 0, 4, 'Scroll of Stamina',         2616, 1, 0, 1, 1, 150, 30, 0, 20, 0,
        8101, 0, -1, 'Grants Stamina for a time when used.'),    -- spell from Scroll of Stamina III (4422)
    (9005004, 0, 4, 'Scroll of Spirit',          2616, 1, 0, 1, 1, 150, 30, 0, 20, 0,
        8114, 0, -1, 'Grants Spirit for a time when used.'),     -- spell from Scroll of Spirit III (4424)
    (9005005, 0, 4, 'Scroll of Protection',      2616, 1, 0, 1, 1, 150, 30, 0, 20, 0,
        8095, 0, -1, 'Grants bonus Armor for a time when used.'),-- spell from Scroll of Protection III (4421)

    -- Utility scrolls - spell ids from real potion/conjure/teleport items.
    (9005006, 0, 4, 'Scroll of Minor Healing',    2616, 1, 0, 1, 1, 100, 20, 0, 20, 0,
        439,  0, -1, 'Restores a small amount of health when used.'),   -- spell from Minor Healing Potion (118)
    (9005007, 0, 4, 'Scroll of Mana Restoration', 2616, 1, 0, 1, 1, 120, 24, 0, 20, 0,
        438,  0, -1, 'Restores a small amount of mana when used.'),     -- spell from Lesser Mana Potion (3385)
    (9005008, 0, 4, 'Scroll of Refreshment',      2616, 1, 0, 1, 1,  80, 16, 0, 20, 0,
        430,  0, -1, 'Conjures a drink when used.'),                    -- spell from Conjured Water (5350)
    (9005009, 0, 4, 'Scroll of Recall',           2616, 1, 0, 1, 1, 300, 60, 0, 20, 0,
        8690, 0, -1, 'Teleports you to your Hearthstone location when used.'); -- spell from Hearthstone (6948)

-- ----------------------------------------------------------------------------------
--  VENDOR NPC - "Arcane Scribe" (entry 9005500)
--  Not spawned by default. Modeled on modules/mod-npc-enchanter's working vendor NPC SQL
--  on this fork: same creature_template column shape, displayid moved out into
--  creature_template_model (this fork does not read displayid from creature_template),
--  plus npc_vendor to actually sell the scrolls. Model reused from mod-npc-enchanter's own
--  enchanter NPC (9353, Undead Necromancer) - already proven to work on this fork.
-- ----------------------------------------------------------------------------------
SET
@Entry      := 9005500,
@Model      := 9353, -- Undead Necromancer (reused from mod-npc-enchanter - no client patch needed)
@Name       := "Arcane Scribe",
@Title      := "Scroll Vendor",
@Icon       := "Vendor",
@GossipMenu := 0,
@MinLevel   := 80,
@MaxLevel   := 80,
@Faction    := 35,   -- Friendly
@NPCFlag    := 128,  -- UNIT_NPC_FLAG_VENDOR
@Type       := 7,    -- CREATURE_TYPE_HUMANOID
@TypeFlags  := 0,
@FlagsExtra := 2,    -- CREATURE_FLAG_EXTRA_CIVILIAN
@AIName     := "",
@Script     := "";

-- NPC CREATURE
DELETE FROM `creature_template` WHERE `entry` = @Entry;
INSERT INTO `creature_template` (`entry`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `unit_class`, `unit_flags`, `type`, `type_flags`, `RegenHealth`, `flags_extra`, `AiName`, `ScriptName`)
VALUES (@Entry, @Name, @Title, @Icon, @GossipMenu, @MinLevel, @MaxLevel, @Faction, @NPCFlag, 1, 1.14286, 1, 2, @Type, @TypeFlags, 1, @FlagsExtra, @AIName, @Script);

-- NPC MODEL
DELETE FROM `creature_template_model` WHERE `CreatureID` = @Entry;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`)
VALUES (@Entry, 0, @Model, 1, 1, 0);

-- NPC TEXT
DELETE FROM `npc_text` WHERE `ID` = @Entry;
INSERT INTO `npc_text` (`ID`, `text0_0`)
VALUES (@Entry, 'A scroll for every occasion, adventurer - no scribing skill required. Mages who have '
    'mastered Comprehension can scribe these themselves with the .scroll command.');

-- ----------------------------------------------------------------------------------
--  VENDOR STOCK - sells all 10 scrolls, unlimited stock, no restock timer.
-- ----------------------------------------------------------------------------------
DELETE FROM `npc_vendor` WHERE `entry` = 9005500;
INSERT INTO `npc_vendor` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`)
VALUES
    (9005500, 1,  9005000, 0, 0, 0),
    (9005500, 2,  9005001, 0, 0, 0),
    (9005500, 3,  9005002, 0, 0, 0),
    (9005500, 4,  9005003, 0, 0, 0),
    (9005500, 5,  9005004, 0, 0, 0),
    (9005500, 6,  9005005, 0, 0, 0),
    (9005500, 7,  9005006, 0, 0, 0),
    (9005500, 8,  9005007, 0, 0, 0),
    (9005500, 9,  9005008, 0, 0, 0),
    (9005500, 10, 9005009, 0, 0, 0);
