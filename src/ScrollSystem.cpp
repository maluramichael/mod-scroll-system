/*
 * mod-scroll-system
 *
 * Mage "Comprehension" (WoW-Forever style): from ScrollSystem.ComprehensionLevel
 * onward a Mage can scribe consumable "Scroll of X" items via `.scroll create <n>`.
 * Each scroll (item_template entries 9005000-9005009, see
 * data/sql/db-world/updates/mod_scroll_system_2026_09_22_00.sql) casts an
 * existing, verified WotLK 3.3.5a spell on use and is consumed - see
 * ScrollSystem::GetScrollTable() below for the full list and where each spell
 * id was sourced from.
 *
 * An "Arcane Scribe" vendor NPC (creature_template entry 9005500, same SQL
 * file) sells the same scrolls to anyone, mirroring how real scroll vendors
 * work; `.scroll create` is the free, Comprehension-gated alternative for
 * mages.
 *
 * Released under GNU GPL v2 or (at your option) any later version.
 */

#include "Chat.h"
#include "ChatCommand.h"
#include "Config.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "StringFormat.h"

#include <cstdlib>
#include <iterator>
#include <sstream>

#include "ScrollSystem.h"

using namespace Acore::ChatCommands;

namespace ScrollSystem
{
    Config& GetConfig()
    {
        static Config cfg;
        return cfg;
    }

    // Spell ids below are copied straight off real item_template rows shipped
    // in this fork's base world DB dump (data/sql/base/db_world/item_template.sql),
    // i.e. spells already cast by existing stock items on this exact server -
    // guaranteed to exist, no invented ids.
    static ScrollDef const kScrollTable[] = {
        // idx, item entry,   name,                          spell,  sourced from (real item, entry)
        {  1, 9005000, "Scroll of Strength",         8120, "Strength"          }, // Scroll of Strength III (4426)
        {  2, 9005001, "Scroll of Agility",          8117, "Agility"           }, // Scroll of Agility III (4425)
        {  3, 9005002, "Scroll of Intellect",        8098, "Intellect"         }, // Scroll of Intellect III (4419)
        {  4, 9005003, "Scroll of Stamina",          8101, "Stamina"           }, // Scroll of Stamina III (4422)
        {  5, 9005004, "Scroll of Spirit",           8114, "Spirit"            }, // Scroll of Spirit III (4424)
        {  6, 9005005, "Scroll of Protection",       8095, "Armor"             }, // Scroll of Protection III (4421)
        {  7, 9005006, "Scroll of Minor Healing",     439, "instant heal"      }, // Minor Healing Potion (118)
        {  8, 9005007, "Scroll of Mana Restoration",  438, "instant mana"      }, // Lesser Mana Potion (3385)
        {  9, 9005008, "Scroll of Refreshment",       430, "conjures water"    }, // Conjured Water (5350)
        { 10, 9005009, "Scroll of Recall",           8690, "teleport to home"  }, // Hearthstone (6948)
    };

    ScrollDef const* GetScrollTable()
    {
        return kScrollTable;
    }

    size_t GetScrollTableSize()
    {
        return std::size(kScrollTable);
    }

    ScrollDef const* FindScrollByIndex(uint32_t index)
    {
        for (ScrollDef const& scroll : kScrollTable)
            if (scroll.index == index)
                return &scroll;
        return nullptr;
    }

    bool HasComprehension(Player* player)
    {
        if (!player)
            return false;

        Config const& cfg = GetConfig();
        return player->getClass() == CLASS_MAGE
            && static_cast<uint32_t>(player->GetLevel()) >= cfg.ComprehensionLevel;
    }
}

using namespace ScrollSystem;

// =====================================================================
//  CommandScript: `.scroll list` / `.scroll create <n>`
// =====================================================================
class scrollsystem_commandscript : public CommandScript
{
public:
    scrollsystem_commandscript() : CommandScript("scrollsystem_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable scrollTable =
        {
            { "list",   HandleScrollListCommand,   SEC_PLAYER, Console::No },
            { "create", HandleScrollCreateCommand, SEC_PLAYER, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "scroll", scrollTable },
        };

        return commandTable;
    }

    static bool HandleScrollListCommand(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!player)
            return false;

        Config const& cfg = GetConfig();
        if (!cfg.Enable)
        {
            handler->SendSysMessage("The scroll system is currently disabled on this server.");
            return true;
        }

        bool const canScribe = HasComprehension(player);
        handler->SendSysMessage(canScribe
            ? "Scrollable scrolls (use '.scroll create <n>' to scribe one):"
            : Acore::StringFormat(
                  "Scrollable scrolls - you need to be a Mage of level {} or higher to scribe "
                  "(Comprehension). Listed for reference:",
                  cfg.ComprehensionLevel));

        for (size_t i = 0; i < GetScrollTableSize(); ++i)
        {
            ScrollDef const& scroll = GetScrollTable()[i];
            handler->SendSysMessage(Acore::StringFormat(
                "  {}. {} - {}", scroll.index, scroll.name, scroll.effect));
        }

        if (cfg.GoldCost > 0)
            handler->SendSysMessage(Acore::StringFormat(
                "Cost per scroll: {}c", cfg.GoldCost));

        return true;
    }

    static bool HandleScrollCreateCommand(ChatHandler* handler, Tail args)
    {
        Player* player = handler->GetPlayer();
        if (!player)
            return false;

        Config const& cfg = GetConfig();
        if (!cfg.Enable)
        {
            handler->SendSysMessage("The scroll system is currently disabled on this server.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!HasComprehension(player))
        {
            handler->PSendSysMessage(
                "You need to be a Mage of level {} or higher to scribe scrolls (Comprehension). "
                "Use '.scroll list' to see what you'll be able to scribe.",
                cfg.ComprehensionLevel);
            handler->SetSentErrorMessage(true);
            return false;
        }

        std::string arg(args);
        uint32_t index = 0;
        {
            std::istringstream in(arg);
            in >> index;
        }

        ScrollDef const* scroll = index != 0 ? FindScrollByIndex(index) : nullptr;
        if (!scroll)
        {
            handler->SendSysMessage("Usage: .scroll create <n> - see '.scroll list' for valid n.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (cfg.GoldCost > 0)
        {
            if (!player->HasEnoughMoney(cfg.GoldCost))
            {
                handler->PSendSysMessage("Scribing {} costs {}c and you don't have enough gold.",
                    scroll->name, cfg.GoldCost);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        if (!player->StoreNewItemInBestSlots(scroll->itemEntry, 1))
        {
            handler->SendSysMessage("Not enough bag space to scribe that scroll.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (cfg.GoldCost > 0)
            player->ModifyMoney(-static_cast<int32>(cfg.GoldCost));

        handler->PSendSysMessage("Scribed {}.", scroll->name);
        return true;
    }
};

// =====================================================================
//  WorldScript: config load.
// =====================================================================
class ScrollSystemWorldScript : public WorldScript
{
public:
    ScrollSystemWorldScript() : WorldScript("ScrollSystem_WorldScript") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        Config& cfg = GetConfig();
        cfg.Enable             = sConfigMgr->GetOption<bool>("ScrollSystem.Enable", true);
        cfg.ComprehensionLevel = sConfigMgr->GetOption<uint32>("ScrollSystem.ComprehensionLevel", 20);
        cfg.GoldCost           = sConfigMgr->GetOption<uint32>("ScrollSystem.GoldCost", 0);
    }
};

// =====================================================================
//  PlayerScript: one-time flavor announcement when Comprehension unlocks.
// =====================================================================
class ScrollSystemPlayerScript : public PlayerScript
{
public:
    ScrollSystemPlayerScript() : PlayerScript("ScrollSystem_PlayerScript") { }

    void OnPlayerLevelChanged(Player* player, uint8 /*oldLevel*/) override
    {
        Config const& cfg = GetConfig();
        if (!cfg.Enable || !player)
            return;

        if (player->getClass() != CLASS_MAGE
            || static_cast<uint32_t>(player->GetLevel()) != cfg.ComprehensionLevel)
            return;

        ChatHandler(player->GetSession()).SendSysMessage(
            "|cff69ccf0[Comprehension]|r You now grasp the theory behind scroll scribing. "
            "Use '.scroll list' to see what you can scribe with '.scroll create <n>'.");
    }
};

// =====================================================================
//  Registration
// =====================================================================
void AddScrollSystemScripts()
{
    new scrollsystem_commandscript();
    new ScrollSystemWorldScript();
    new ScrollSystemPlayerScript();
}
