/*
 * mod-scroll-system - shared declarations.
 *
 * Mage "Comprehension" scroll scribing: from ScrollSystem.ComprehensionLevel
 * onward, a Mage character can use `.scroll create <n>` to scribe one of a
 * fixed set of consumable "Scroll of X" items into their bags. Each scroll
 * casts an existing, verified spell on use (a stat buff, a heal, a mana
 * restore, a conjure, or a teleport) and is consumed on use.
 *
 * v1 scope: Comprehension is a class+level gate only - no passive spell is
 * granted. No existing WotLK 3.3.5a passive was a clean flavor-only fit (every
 * candidate arcane passive found either changes combat stats or is already a
 * talent), and the module brief allows skipping the spell in that case.
 *
 * Released under GNU GPL v2; redistribute/modify under version 2 of the
 * License, or (at your option) any later version.
 */

#ifndef MOD_SCROLL_SYSTEM_H
#define MOD_SCROLL_SYSTEM_H

#include <cstddef>
#include <cstdint>

class Player;

namespace ScrollSystem
{
    // Cached config (populated in WorldScript::OnAfterConfigLoad).
    struct Config
    {
        bool     Enable            = true;
        uint32_t ComprehensionLevel = 20;
        uint32_t GoldCost           = 0; // copper, per scroll scribed; 0 = free
    };

    Config& GetConfig();

    // One scribeable scroll: the `.scroll create <n>` index, the item_template
    // entry it hands out (data/sql/db-world/updates/mod_scroll_system_*.sql),
    // a display name, and the existing spell it casts on use (documented here
    // for `.scroll list`; the item itself carries spellid_1).
    struct ScrollDef
    {
        uint32_t    index;
        uint32_t    itemEntry;
        char const* name;
        uint32_t    spellId;
        char const* effect;
    };

    // The fixed scroll catalogue, in `.scroll list` / `.scroll create <n>` order.
    ScrollDef const* GetScrollTable();
    size_t GetScrollTableSize();

    // Looks up a scroll by its 1-based `.scroll create <n>` index. Returns
    // nullptr if out of range.
    ScrollDef const* FindScrollByIndex(uint32_t index);

    // True if the character currently qualifies to scribe scrolls: a Mage at
    // or above ScrollSystem.ComprehensionLevel. Pure class+level check, no DB
    // hit, no spell requirement (see file header).
    bool HasComprehension(Player* player);
}

#endif // MOD_SCROLL_SYSTEM_H
