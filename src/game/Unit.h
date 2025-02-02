/**
 * This file is part of the OregonCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * @file Unit.cpp
 */

#ifndef __UNIT_H
#define __UNIT_H

#include "Common.h"
#include "Object.h"
#include "Opcodes.h"
#include "SpellAuraDefines.h"
#include "UpdateFields.h"
#include "SharedDefines.h"
#include "ThreatManager.h"
#include "HostileRefManager.h"
#include "FollowerReference.h"
#include "FollowerRefManager.h"
#include "Utilities/EventProcessor.h"
#include "MotionMaster.h"
#include "DBCStructure.h"
#include <list>

#define WORLD_TRIGGER   12999

enum SpellInterruptFlags
{
    SPELL_INTERRUPT_FLAG_MOVEMENT     = 0x01, // why need this for instant?
    SPELL_INTERRUPT_FLAG_PUSH_BACK    = 0x02, // push back
    SPELL_INTERRUPT_FLAG_INTERRUPT    = 0x04, // interrupt
    SPELL_INTERRUPT_FLAG_AUTOATTACK   = 0x08, // no
    SPELL_INTERRUPT_FLAG_DAMAGE       = 0x10  // _complete_ interrupt on direct damage?
};

enum SpellChannelInterruptFlags
{
    CHANNEL_FLAG_DAMAGE      = 0x0002,
    CHANNEL_FLAG_MOVEMENT    = 0x0008,
    CHANNEL_FLAG_TURNING     = 0x0010,
    CHANNEL_FLAG_DAMAGE2     = 0x0080,
    CHANNEL_FLAG_DELAY       = 0x4000
};

enum SpellAuraInterruptFlags
{
    AURA_INTERRUPT_FLAG_HITBYSPELL          = 0x00000001,   // 0    removed when getting hit by a negative spell?
    AURA_INTERRUPT_FLAG_DAMAGE              = 0x00000002,   // 1    removed by any damage
    AURA_INTERRUPT_FLAG_CC                  = 0x00000004,   // 2    crowd control
    AURA_INTERRUPT_FLAG_MOVE                = 0x00000008,   // 3    removed by any movement
    AURA_INTERRUPT_FLAG_TURNING             = 0x00000010,   // 4    removed by any turning
    AURA_INTERRUPT_FLAG_JUMP                = 0x00000020,   // 5    removed by entering combat
    AURA_INTERRUPT_FLAG_NOT_MOUNTED         = 0x00000040,   // 6    removed by unmounting
    AURA_INTERRUPT_FLAG_NOT_ABOVEWATER      = 0x00000080,   // 7    removed by entering water
    AURA_INTERRUPT_FLAG_NOT_UNDERWATER      = 0x00000100,   // 8    removed by leaving water
    AURA_INTERRUPT_FLAG_NOT_SHEATHED        = 0x00000200,   // 9    removed by unsheathing
    AURA_INTERRUPT_FLAG_TALK                = 0x00000400,   // 10   talk to npc / loot? action on creature
    AURA_INTERRUPT_FLAG_USE                 = 0x00000800,   // 11   mine/use/open action on gameobject
    AURA_INTERRUPT_FLAG_ATTACK              = 0x00001000,   // 12   removed by attacking
    AURA_INTERRUPT_FLAG_CAST                = 0x00002000,   // 13   ???
    AURA_INTERRUPT_FLAG_UNK14               = 0x00004000,   // 14
    AURA_INTERRUPT_FLAG_TRANSFORM           = 0x00008000,   // 15   removed by transform?
    AURA_INTERRUPT_FLAG_UNK16               = 0x00010000,   // 16
    AURA_INTERRUPT_FLAG_MOUNT               = 0x00020000,   // 17   misdirect, aspect, swim speed
    AURA_INTERRUPT_FLAG_NOT_SEATED          = 0x00040000,   // 18   removed by standing up
    AURA_INTERRUPT_FLAG_CHANGE_MAP          = 0x00080000,   // 19   leaving map/getting teleported
    AURA_INTERRUPT_FLAG_UNATTACKABLE        = 0x00100000,   // 20   invulnerable or stealth
    AURA_INTERRUPT_FLAG_UNK21               = 0x00200000,   // 21
    AURA_INTERRUPT_FLAG_TELEPORTED          = 0x00400000,   // 22
    AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT    = 0x00800000,   // 23   removed by entering pvp combat
    AURA_INTERRUPT_FLAG_DIRECT_DAMAGE       = 0x01000000,   // 24   removed by any direct damage
    AURA_INTERRUPT_FLAG_NOT_VICTIM = (AURA_INTERRUPT_FLAG_HITBYSPELL | AURA_INTERRUPT_FLAG_DAMAGE | AURA_INTERRUPT_FLAG_DIRECT_DAMAGE),
};

enum SpellPartialResist
{
    SPELL_PARTIAL_RESIST_NONE = 0,  // 0%, full hit
    SPELL_PARTIAL_RESIST_PCT_25,    // 25%
    SPELL_PARTIAL_RESIST_PCT_50,    // 50%
    SPELL_PARTIAL_RESIST_PCT_75,    // 75%
    SPELL_PARTIAL_RESIST_PCT_100,   // 100%, full resist
};

#define NUM_SPELL_PARTIAL_RESISTS 5

enum SpellModOp
{
    SPELLMOD_DAMAGE                 = 0,
    SPELLMOD_DURATION               = 1,
    SPELLMOD_THREAT                 = 2,
    SPELLMOD_EFFECT1                = 3,
    SPELLMOD_CHARGES                = 4,
    SPELLMOD_RANGE                  = 5,
    SPELLMOD_RADIUS                 = 6,
    SPELLMOD_CRITICAL_CHANCE        = 7,
    SPELLMOD_ALL_EFFECTS            = 8,
    SPELLMOD_NOT_LOSE_CASTING_TIME  = 9,
    SPELLMOD_CASTING_TIME           = 10,
    SPELLMOD_COOLDOWN               = 11,
    SPELLMOD_EFFECT2                = 12,
    // spellmod 13 unused
    SPELLMOD_COST                   = 14,
    SPELLMOD_CRIT_DAMAGE_BONUS      = 15,
    SPELLMOD_RESIST_MISS_CHANCE     = 16,
    SPELLMOD_JUMP_TARGETS           = 17,
    SPELLMOD_CHANCE_OF_SUCCESS      = 18,
    SPELLMOD_ACTIVATION_TIME        = 19,
    SPELLMOD_EFFECT_PAST_FIRST      = 20,
    SPELLMOD_GLOBAL_COOLDOWN        = 21,
    SPELLMOD_DOT                    = 22,
    SPELLMOD_EFFECT3                = 23,
    SPELLMOD_SPELL_BONUS_DAMAGE     = 24,
    // spellmod 25, 26 unused
    SPELLMOD_MULTIPLE_VALUE         = 27,
    SPELLMOD_RESIST_DISPEL_CHANCE   = 28
};

#define MAX_SPELLMOD 32

enum SpellValueMod
{
    SPELLVALUE_BASE_POINT0,
    SPELLVALUE_BASE_POINT1,
    SPELLVALUE_BASE_POINT2,
    SPELLVALUE_MAX_TARGETS,
    SPELLVALUE_DURATION
};

typedef std::pair<SpellValueMod, int32>     CustomSpellValueMod;
class CustomSpellValues : public std::vector<CustomSpellValueMod>
{
    public:
        void AddSpellMod(SpellValueMod mod, int32 value)
        {
            push_back(std::make_pair(mod, value));
        }
};

enum SpellFacingFlags
{
    SPELL_FACING_FLAG_INFRONT = 0x0001
};

#define BASE_MINDAMAGE 1.0f
#define BASE_MAXDAMAGE 2.0f
#define BASE_ATTACK_TIME 2000

// byte value (UNIT_FIELD_BYTES_1,0)
enum UnitStandStateType
{
    UNIT_STAND_STATE_STAND             = 0,
    UNIT_STAND_STATE_SIT               = 1,
    UNIT_STAND_STATE_SIT_CHAIR         = 2,
    UNIT_STAND_STATE_SLEEP             = 3,
    UNIT_STAND_STATE_SIT_LOW_CHAIR     = 4,
    UNIT_STAND_STATE_SIT_MEDIUM_CHAIR  = 5,
    UNIT_STAND_STATE_SIT_HIGH_CHAIR    = 6,
    UNIT_STAND_STATE_DEAD              = 7,
    UNIT_STAND_STATE_KNEEL             = 8
};

// byte flag value (UNIT_FIELD_BYTES_1,2)
enum UnitStandFlags
{
    UNIT_STAND_FLAGS_CREEP        = 0x02,
    UNIT_STAND_FLAGS_ALL          = 0xFF
};

// byte flags value (UNIT_FIELD_BYTES_1,3)
enum UnitBytes1_Flags
{
    UNIT_BYTE1_FLAG_ALWAYS_STAND = 0x01,
    UNIT_BYTE1_FLAG_UNTRACKABLE  = 0x04,
    UNIT_BYTE1_FLAG_ALL          = 0xFF
};

// high byte (3 from 0..3) of UNIT_FIELD_BYTES_2
enum ShapeshiftForm
{
    FORM_NONE               = 0x00,
    FORM_CAT                = 0x01,
    FORM_TREE               = 0x02,
    FORM_TRAVEL             = 0x03,
    FORM_AQUA               = 0x04,
    FORM_BEAR               = 0x05,
    FORM_AMBIENT            = 0x06,
    FORM_GHOUL              = 0x07,
    FORM_DIREBEAR           = 0x08,
    FORM_CREATUREBEAR       = 0x0E,
    FORM_CREATURECAT        = 0x0F,
    FORM_GHOSTWOLF          = 0x10,
    FORM_BATTLESTANCE       = 0x11,
    FORM_DEFENSIVESTANCE    = 0x12,
    FORM_BERSERKERSTANCE    = 0x13,
    FORM_TEST               = 0x14,
    FORM_ZOMBIE             = 0x15,
    FORM_MASTER_ANGLER      = 0x1A,
    FORM_FLIGHT_EPIC        = 0x1B,
    FORM_SHADOW             = 0x1C,
    FORM_FLIGHT             = 0x1D,
    FORM_STEALTH            = 0x1E,
    FORM_MOONKIN            = 0x1F,
    FORM_SPIRITOFREDEMPTION = 0x20
};

// low byte (0 from 0..3) of UNIT_FIELD_BYTES_2
enum SheathState
{
    SHEATH_STATE_UNARMED  = 0,                              // non prepared weapon
    SHEATH_STATE_MELEE    = 1,                              // prepared melee weapon
    SHEATH_STATE_RANGED   = 2                               // prepared ranged weapon
};

#define MAX_SHEATH_STATE    3

// byte (1 from 0..3) of UNIT_FIELD_BYTES_2
enum UnitBytes2_Flags
{
    UNIT_BYTE2_FLAG_UNK0        = 0x01,
    UNIT_BYTE2_FLAG_UNK1        = 0x02,
    UNIT_BYTE2_FLAG_FFA_PVP     = 0x04,
    UNIT_BYTE2_FLAG_SANCTUARY   = 0x08,
    UNIT_BYTE2_FLAG_AURAS       = 0x10,                     // show possitive auras as positive, and allow its dispel
    UNIT_BYTE2_FLAG_UNK5        = 0x20,                     // show negative auras as positive, disallowing dispel
    UNIT_BYTE2_FLAG_UNK6        = 0x40,
    UNIT_BYTE2_FLAG_UNK7        = 0x80
};

// byte (2 from 0..3) of UNIT_FIELD_BYTES_2
enum UnitRename
{
    UNIT_RENAME_NOT_ALLOWED = 0x02,
    UNIT_RENAME_ALLOWED     = 0x03
};

#define CREATURE_MAX_SPELLS     4
#define MAX_SPELL_CHARM         4
#define MAX_SPELL_POSSESS       8

#define MAX_AGGRO_RESET_TIME    10     // In Seconds
#define MAX_AGGRO_RADIUS        45.0f  // yards

enum Swing
{
    NOSWING                    = 0,
    SINGLEHANDEDSWING          = 1,
    TWOHANDEDSWING             = 2
};

enum VictimState
{
    VICTIMSTATE_UNKNOWN1       = 0,
    VICTIMSTATE_NORMAL         = 1,
    VICTIMSTATE_DODGE          = 2,
    VICTIMSTATE_PARRY          = 3,
    VICTIMSTATE_INTERRUPT      = 4,
    VICTIMSTATE_BLOCKS         = 5,
    VICTIMSTATE_EVADES         = 6,
    VICTIMSTATE_IS_IMMUNE      = 7,
    VICTIMSTATE_DEFLECTS       = 8
};

enum HitInfo
{
    HITINFO_NORMALSWING         = 0x00000000,
    HITINFO_UNK1                = 0x00000001,               // req correct packet structure
    HITINFO_NORMALSWING2        = 0x00000002,
    HITINFO_LEFTSWING           = 0x00000004,
    HITINFO_MISS                = 0x00000010,
    HITINFO_ABSORB              = 0x00000020,               // plays absorb sound
    HITINFO_RESIST              = 0x00000040,               // resisted at least some damage
    HITINFO_CRITICALHIT         = 0x00000080,
    HITINFO_UNK2                = 0x00000100,               // wotlk?
    HITINFO_UNK3                = 0x00002000,               // wotlk?
    HITINFO_GLANCING            = 0x00004000,
    HITINFO_CRUSHING            = 0x00008000,
    HITINFO_NOACTION            = 0x00010000,
    HITINFO_SWINGNOHITSOUND     = 0x00080000
};

enum UnitBytes0Offsets : uint8
{
    UNIT_BYTES_0_OFFSET_RACE = 0,
    UNIT_BYTES_0_OFFSET_CLASS = 1,
    UNIT_BYTES_0_OFFSET_GENDER = 2,
    UNIT_BYTES_0_OFFSET_POWER_TYPE = 3,
};

enum UnitBytes1Offsets : uint8
{
    UNIT_BYTES_1_OFFSET_STAND_STATE = 0,
    UNIT_BYTES_1_OFFSET_PET_TALENTS = 1,
    UNIT_BYTES_1_OFFSET_VIS_FLAG = 2,
    UNIT_BYTES_1_OFFSET_ANIM_TIER = 3
};

enum UnitBytes2Offsets : uint8
{
    UNIT_BYTES_2_OFFSET_SHEATH_STATE = 0,
    UNIT_BYTES_2_OFFSET_PVP_FLAG = 1,
    UNIT_BYTES_2_OFFSET_PET_FLAGS = 2,
    UNIT_BYTES_2_OFFSET_SHAPESHIFT_FORM = 3
};

//i would like to remove this: (it is defined in item.h
enum InventorySlot
{
    NULL_BAG                   = 0,
    NULL_SLOT                  = 255
};

struct FactionTemplateEntry;
struct Modifier;
struct SpellEntry;
struct SpellValue;

class Aura;
class Creature;
class Spell;
class DynamicObject;
class GameObject;
class Item;
class Pet;
class PetAura;
class UnitAI;
class Minion;
class Guardian;
class ScriptEvent;

struct SpellImmune
{
    uint32 type;
    uint32 spellId;
};

typedef std::list<SpellImmune> SpellImmuneList;

enum UnitModifierType
{
    BASE_VALUE = 0,
    BASE_PCT = 1,
    TOTAL_VALUE = 2,
    TOTAL_PCT = 3,
    MODIFIER_TYPE_END = 4
};

enum WeaponDamageRange
{
    MINDAMAGE,
    MAXDAMAGE
};

enum DamageTypeToSchool
{
    RESISTANCE,
    DAMAGE_DEALT,
    DAMAGE_TAKEN
};

enum AuraRemoveMode
{
    AURA_REMOVE_NONE = 0,
    AURA_REMOVE_BY_DEFAULT = 1,
    AURA_REMOVE_BY_STACK,                                   // at replace by semillar aura
    AURA_REMOVE_BY_CANCEL,
    AURA_REMOVE_BY_DISPEL,
    AURA_REMOVE_BY_EXPIRE,
    AURA_REMOVE_BY_DEATH
};

enum UnitMods
{
    UNIT_MOD_STAT_STRENGTH,                                 // UNIT_MOD_STAT_STRENGTH..UNIT_MOD_STAT_SPIRIT must be in existing order, it's accessed by index values of Stats enum.
    UNIT_MOD_STAT_AGILITY,
    UNIT_MOD_STAT_STAMINA,
    UNIT_MOD_STAT_INTELLECT,
    UNIT_MOD_STAT_SPIRIT,
    UNIT_MOD_HEALTH,
    UNIT_MOD_MANA,                                          // UNIT_MOD_MANA..UNIT_MOD_HAPPINESS must be in existing order, it's accessed by index values of Powers enum.
    UNIT_MOD_RAGE,
    UNIT_MOD_FOCUS,
    UNIT_MOD_ENERGY,
    UNIT_MOD_HAPPINESS,
    UNIT_MOD_ARMOR,                                         // UNIT_MOD_ARMOR..UNIT_MOD_RESISTANCE_ARCANE must be in existing order, it's accessed by index values of SpellSchools enum.
    UNIT_MOD_RESISTANCE_HOLY,
    UNIT_MOD_RESISTANCE_FIRE,
    UNIT_MOD_RESISTANCE_NATURE,
    UNIT_MOD_RESISTANCE_FROST,
    UNIT_MOD_RESISTANCE_SHADOW,
    UNIT_MOD_RESISTANCE_ARCANE,
    UNIT_MOD_ATTACK_POWER,
    UNIT_MOD_ATTACK_POWER_RANGED,
    UNIT_MOD_DAMAGE_MAINHAND,
    UNIT_MOD_DAMAGE_OFFHAND,
    UNIT_MOD_DAMAGE_RANGED,
    UNIT_MOD_END,
    // synonyms
    UNIT_MOD_STAT_START = UNIT_MOD_STAT_STRENGTH,
    UNIT_MOD_STAT_END = UNIT_MOD_STAT_SPIRIT + 1,
    UNIT_MOD_RESISTANCE_START = UNIT_MOD_ARMOR,
    UNIT_MOD_RESISTANCE_END = UNIT_MOD_RESISTANCE_ARCANE + 1,
    UNIT_MOD_POWER_START = UNIT_MOD_MANA,
    UNIT_MOD_POWER_END = UNIT_MOD_HAPPINESS + 1
};

enum BaseModGroup
{
    CRIT_PERCENTAGE,
    RANGED_CRIT_PERCENTAGE,
    OFFHAND_CRIT_PERCENTAGE,
    SHIELD_BLOCK_VALUE,
    BASEMOD_END
};

enum BaseModType
{
    FLAT_MOD,
    PCT_MOD,
    MOD_END
};

enum DeathState
{
    ALIVE           = 0,
    JUST_DIED       = 1,
    CORPSE          = 2,
    DEAD            = 3,
    JUST_RESPAWNED  = 4,
    DEAD_FALLING    = 5
};

enum UnitState
{
    UNIT_STATE_DIED               = 0x00000001,
    UNIT_STATE_MELEE_ATTACKING    = 0x00000002,                     // player is melee attacking someone
    //UNIT_STATE_MELEE_ATTACK_BY  = 0x00000004,                     // player is melee attack by someone
    UNIT_STATE_STUNNED            = 0x00000008,
    UNIT_STATE_ROAMING            = 0x00000010,
    UNIT_STATE_CHASE              = 0x00000020,
    //UNIT_STATE_SEARCHING        = 0x00000040,
    UNIT_STATE_FLEEING            = 0x00000080,
    UNIT_STATE_IN_FLIGHT          = 0x00000100,                     // player is in flight mode
    UNIT_STATE_FOLLOW             = 0x00000200,
    UNIT_STATE_ROOT               = 0x00000400,
    UNIT_STATE_CONFUSED           = 0x00000800,
    UNIT_STATE_DISTRACTED         = 0x00001000,
    UNIT_STATE_ISOLATED           = 0x00002000,                     // area auras do not affect other players
    UNIT_STATE_ATTACK_PLAYER      = 0x00004000,
    UNIT_STATE_CASTING            = 0x00008000,
    UNIT_STATE_POSSESSED          = 0x00010000,
    UNIT_STATE_CHARGING           = 0x00020000,
    UNIT_STATE_JUMPING            = 0x00040000,
    UNIT_STATE_MOVE               = 0x00100000,
    UNIT_STATE_ROTATING           = 0x00200000,
    UNIT_STATE_EVADE              = 0x00400000,
    UNIT_STATE_IGNORE_PATHFINDING = 0x10000000,                 // do not use pathfinding in any MovementGenerator

    UNIT_STATE_UNATTACKABLE    = UNIT_STATE_IN_FLIGHT,
    UNIT_STATE_MOVING          = (UNIT_STATE_ROAMING | UNIT_STATE_CHASE),
    UNIT_STATE_RUNNING_STATE   = (UNIT_STATE_CHASE | UNIT_STATE_FLEEING),
    UNIT_STATE_CONTROLLED      = (UNIT_STATE_CONFUSED | UNIT_STATE_STUNNED | UNIT_STATE_FLEEING),
    UNIT_STATE_LOST_CONTROL    = (UNIT_STATE_CONTROLLED | UNIT_STATE_JUMPING | UNIT_STATE_CHARGING),
    UNIT_STATE_SIGHTLESS       = (UNIT_STATE_LOST_CONTROL | UNIT_STATE_EVADE),
    UNIT_STATE_CANNOT_AUTOATTACK     = (UNIT_STATE_LOST_CONTROL | UNIT_STATE_CASTING),
    UNIT_STATE_CANNOT_TURN     = (UNIT_STATE_LOST_CONTROL | UNIT_STATE_ROTATING),
    UNIT_STATE_ALL_STATE       = 0xffffffff                      //(UNIT_STATEE_STOPPED | UNIT_STATEE_MOVING | UNIT_STATEE_IN_COMBAT | UNIT_STATEE_IN_FLIGHT)
};

enum UnitMoveType
{
    MOVE_WALK           = 0,
    MOVE_RUN            = 1,
    MOVE_RUN_BACK       = 2,
    MOVE_SWIM           = 3,
    MOVE_SWIM_BACK      = 4,
    MOVE_TURN_RATE      = 5,
    MOVE_FLIGHT         = 6,
    MOVE_FLIGHT_BACK    = 7,
};

#define MAX_MOVE_TYPE 8

extern float baseMoveSpeed[MAX_MOVE_TYPE];

enum WeaponAttackType
{
    BASE_ATTACK   = 0,
    OFF_ATTACK    = 1,
    RANGED_ATTACK = 2
};

#define MAX_ATTACK  3

enum CombatRating
{
    CR_WEAPON_SKILL             = 0,
    CR_DEFENSE_SKILL            = 1,
    CR_DODGE                    = 2,
    CR_PARRY                    = 3,
    CR_BLOCK                    = 4,
    CR_HIT_MELEE                = 5,
    CR_HIT_RANGED               = 6,
    CR_HIT_SPELL                = 7,
    CR_CRIT_MELEE               = 8,
    CR_CRIT_RANGED              = 9,
    CR_CRIT_SPELL               = 10,
    CR_HIT_TAKEN_MELEE          = 11,
    CR_HIT_TAKEN_RANGED         = 12,
    CR_HIT_TAKEN_SPELL          = 13,
    CR_CRIT_TAKEN_MELEE         = 14,
    CR_CRIT_TAKEN_RANGED        = 15,
    CR_CRIT_TAKEN_SPELL         = 16,
    CR_HASTE_MELEE              = 17,
    CR_HASTE_RANGED             = 18,
    CR_HASTE_SPELL              = 19,
    CR_WEAPON_SKILL_MAINHAND    = 20,
    CR_WEAPON_SKILL_OFFHAND     = 21,
    CR_WEAPON_SKILL_RANGED      = 22,
    CR_EXPERTISE                = 23
};

#define MAX_COMBAT_RATING         24

enum DamageEffectType
{
    DIRECT_DAMAGE           = 0,                            // used for normal weapon damage (not for class abilities or spells)
    SPELL_DIRECT_DAMAGE     = 1,                            // spell/class abilities damage
    DOT                     = 2,
    HEAL                    = 3,
    NODAMAGE                = 4,                            // used also in case when damage applied to health but not applied to spell channelInterruptFlags/etc
    SELF_DAMAGE             = 5
};

/// internal used flags for marking special auras - for example some dummy-auras
enum UnitAuraFlags
{
    UNIT_AURAFLAG_ALIVE_INVISIBLE   = 0x1,                  // aura which makes unit invisible for alive
};

// Value masks for UNIT_FIELD_FLAGS
enum UnitFlags
{
    UNIT_FLAG_UNK_0                 = 0x00000001,
    UNIT_FLAG_NON_ATTACKABLE        = 0x00000002,           // not attackable
    UNIT_FLAG_REMOVE_CLIENT_CONTROL = 0x00000004,           // This is a legacy flag used to disable movement player's movement while controlling other units, SMSG_CLIENT_CONTROL replaces this functionality clientside now. CONFUSED and FLEEING flags have the same effect on client movement as DISABLE_MOVE_CONTROL in addition to preventing spell casts/autoattack (they all allow climbing steeper hills and emotes while moving)
    UNIT_FLAG_PVP_ATTACKABLE        = 0x00000008,           // allow apply pvp rules to attackable state in addition to faction dependent state
    UNIT_FLAG_RENAME                = 0x00000010,
    UNIT_FLAG_PREPARATION           = 0x00000020,           // don't take reagents for spells with SPELL_ATTR5_NO_REAGENT_WHILE_PREP
    UNIT_FLAG_UNK_6                 = 0x00000040,
    UNIT_FLAG_NOT_ATTACKABLE_1      = 0x00000080,           // ?? (UNIT_FLAG_PVP_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1) is NON_PVP_ATTACKABLE
    UNIT_FLAG_IMMUNE_TO_PC          = 0x00000100,           // disables actions by PlayerCharacters (PC) on unit - see Unit::_IsValidAttackTarget, Unit::_IsValidAssistTarget
    UNIT_FLAG_IMMUNE_TO_NPC         = 0x00000200,           // disables actions by NonPlayerCharacters (NPC) on unit - see Unit::_IsValidAttackTarget, Unit::_IsValidAssistTarget
    UNIT_FLAG_LOOTING               = 0x00000400,           // loot animation
    UNIT_FLAG_PET_IN_COMBAT         = 0x00000800,           // in combat?, 2.0.8
    UNIT_FLAG_PVP                   = 0x00001000,
    UNIT_FLAG_SILENCED              = 0x00002000,           // silenced, 2.1.1
    UNIT_FLAG_CANNOT_SWIM           = 0x00004000,           // 2.0.8
    UNIT_FLAG_UNK_15                = 0x00008000,
    UNIT_FLAG_NOT_PL_SPELL_TARGET   = 0x00010000,           // Unit with that flag can not be direct target of player spell.
    UNIT_FLAG_PACIFIED              = 0x00020000,
    UNIT_FLAG_STUNNED               = 0x00040000,           // stunned, 2.1.1
    UNIT_FLAG_IN_COMBAT             = 0x00080000,
    UNIT_FLAG_TAXI_FLIGHT           = 0x00100000,           // disable casting at client side spell not allowed by taxi flight (mounted?), probably used with 0x4 flag
    UNIT_FLAG_DISARMED              = 0x00200000,           // disable melee spells casting..., "Required melee weapon" added to melee spells tooltip.
    UNIT_FLAG_CONFUSED              = 0x00400000,
    UNIT_FLAG_FLEEING               = 0x00800000,
    UNIT_FLAG_PLAYER_CONTROLLED     = 0x01000000,           // used in spell Eyes of the Beast for pet... let attack by controlled creature
    UNIT_FLAG_NOT_SELECTABLE        = 0x02000000,
    UNIT_FLAG_SKINNABLE             = 0x04000000,
    UNIT_FLAG_MOUNT                 = 0x08000000,
    UNIT_FLAG_UNK_28                = 0x10000000,
    UNIT_FLAG_UNK_29                = 0x20000000,           // used in Feing Death spell
    UNIT_FLAG_SHEATHE               = 0x40000000
};

// Value masks for UNIT_FIELD_FLAGS_2
enum UnitFlags2
{
    UNIT_FLAG2_FEIGN_DEATH          = 0x00000001,
    UNIT_FLAG2_IGNORE_REPUTATION    = 0x00000004,
    UNIT_FLAG2_COMPREHEND_LANG      = 0x00000008,
    UNIT_FLAG2_MIRROR_IMAGE         = 0x00000010,
    UNIT_FLAG2_FORCE_MOVE           = 0x00000040
};

// Non Player Character flags
enum NPCFlags
{
    UNIT_NPC_FLAG_NONE                  = 0x00000000,
    UNIT_NPC_FLAG_GOSSIP                = 0x00000001,       // 100%
    UNIT_NPC_FLAG_QUESTGIVER            = 0x00000002,       // guessed, probably ok
    UNIT_NPC_FLAG_UNK1                  = 0x00000004,
    UNIT_NPC_FLAG_UNK2                  = 0x00000008,
    UNIT_NPC_FLAG_TRAINER               = 0x00000010,       // 100%
    UNIT_NPC_FLAG_TRAINER_CLASS         = 0x00000020,       // 100%
    UNIT_NPC_FLAG_TRAINER_PROFESSION    = 0x00000040,       // 100%
    UNIT_NPC_FLAG_VENDOR                = 0x00000080,       // 100%
    UNIT_NPC_FLAG_VENDOR_AMMO           = 0x00000100,       // 100%, general goods vendor
    UNIT_NPC_FLAG_VENDOR_FOOD           = 0x00000200,       // 100%
    UNIT_NPC_FLAG_VENDOR_POISON         = 0x00000400,       // guessed
    UNIT_NPC_FLAG_VENDOR_REAGENT        = 0x00000800,       // 100%
    UNIT_NPC_FLAG_REPAIR                = 0x00001000,       // 100%
    UNIT_NPC_FLAG_FLIGHTMASTER          = 0x00002000,       // 100%
    UNIT_NPC_FLAG_SPIRITHEALER          = 0x00004000,       // guessed
    UNIT_NPC_FLAG_SPIRITGUIDE           = 0x00008000,       // guessed
    UNIT_NPC_FLAG_INNKEEPER             = 0x00010000,       // 100%
    UNIT_NPC_FLAG_BANKER                = 0x00020000,       // 100%
    UNIT_NPC_FLAG_PETITIONER            = 0x00040000,       // 100% 0xC0000 = guild petitions, 0x40000 = arena team petitions
    UNIT_NPC_FLAG_TABARDDESIGNER        = 0x00080000,       // 100%
    UNIT_NPC_FLAG_BATTLEMASTER          = 0x00100000,       // 100%
    UNIT_NPC_FLAG_AUCTIONEER            = 0x00200000,       // 100%
    UNIT_NPC_FLAG_STABLEMASTER          = 0x00400000,       // 100%
    UNIT_NPC_FLAG_GUILD_BANKER          = 0x00800000,       // cause client to send 997 opcode
    UNIT_NPC_FLAG_SPELLCLICK            = 0x01000000,       // cause client to send 1015 opcode (spell click)
};

enum UnitTypeMask
{
    UNIT_MASK_NONE                  = 0x00000000,
    UNIT_MASK_SUMMON                = 0x00000001,
    UNIT_MASK_MINION                = 0x00000002,
    UNIT_MASK_GUARDIAN              = 0x00000004,
    UNIT_MASK_TOTEM                 = 0x00000008,
    UNIT_MASK_PET                   = 0x00000010,
    UNIT_MASK_PUPPET                = 0x00000040,
    UNIT_MASK_HUNTER_PET            = 0x00000080,
    UNIT_MASK_CONTROLABLE_GUARDIAN  = 0x00000100
};

enum DiminishingLevels
{
    DIMINISHING_LEVEL_1             = 0,
    DIMINISHING_LEVEL_2             = 1,
    DIMINISHING_LEVEL_3             = 2,
    DIMINISHING_LEVEL_IMMUNE        = 3
};

struct DiminishingReturn
{
    DiminishingReturn(DiminishingGroup group, uint32 t, uint32 count) : DRGroup(group), hitTime(t), hitCount(count), stack(0) {}

    DiminishingGroup        DRGroup;
    uint32                  hitTime;
    uint32                  hitCount;
    uint16                  stack;
};

enum MeleeHitOutcome
{
    MELEE_HIT_EVADE, MELEE_HIT_MISS, MELEE_HIT_DODGE, MELEE_HIT_BLOCK, MELEE_HIT_PARRY,
    MELEE_HIT_GLANCING, MELEE_HIT_CRIT, MELEE_HIT_CRUSHING, MELEE_HIT_NORMAL, MELEE_HIT_BLOCK_CRIT
};

struct CleanDamage
{
    CleanDamage(uint32 _damage, WeaponAttackType _attackType, MeleeHitOutcome _hitOutCome) :
        damage(_damage), attackType(_attackType), hitOutCome(_hitOutCome) {}

    uint32 damage;
    WeaponAttackType attackType;
    MeleeHitOutcome hitOutCome;
};

// Struct for use in Unit::CalculateMeleeDamage
// Need create structure like in SMSG_ATTACKERSTATEUPDATE opcode
struct CalcDamageInfo
{
    Unit*  attacker;             // Attacker
    Unit*  target;               // Target for damage
    uint32 damageSchoolMask;
    uint32 damage;
    uint32 absorb;
    uint32 resist;
    uint32 blocked_amount;
    uint32 HitInfo;
    uint32 TargetState;
    // Helper
    WeaponAttackType attackType; //
    uint32 procAttacker;
    uint32 procVictim;
    uint32 procEx;
    uint32 cleanDamage;          // Used only fo rage calcultion
    MeleeHitOutcome hitOutCome;  // @todo remove this field (need use TargetState)
};

// Spell damage info structure based on structure sending in SMSG_SPELLNONMELEEDAMAGELOG opcode
struct SpellNonMeleeDamage
{
    SpellNonMeleeDamage(Unit* _attacker, Unit* _target, uint32 _SpellID, uint32 _schoolMask) :
        target(_target), attacker(_attacker), SpellID(_SpellID), damage(0), schoolMask(_schoolMask),
        absorb(0), resist(0), physicalLog(false), unused(false), blocked(0), HitInfo(0), cleanDamage(0) {}
    Unit*   target;
    Unit*   attacker;
    uint32 SpellID;
    uint32 damage;
    uint32 schoolMask;
    uint32 absorb;
    uint32 resist;
    bool   physicalLog;
    bool   unused;
    uint32 blocked;
    uint32 HitInfo;
    // Used for help
    uint32 cleanDamage;
};

uint32 createProcExtendMask(SpellNonMeleeDamage* damageInfo, SpellMissInfo missCondition);

struct UnitActionBarEntry
{
    uint32 Type;
    uint32 SpellOrAction;
};

#define MAX_DECLINED_NAME_CASES 5

struct DeclinedName
{
    std::string name[MAX_DECLINED_NAME_CASES];
};

enum CurrentSpellTypes
{
    CURRENT_MELEE_SPELL             = 0,
    CURRENT_GENERIC_SPELL           = 1,
    CURRENT_AUTOREPEAT_SPELL        = 2,
    CURRENT_CHANNELED_SPELL         = 3
};

#define CURRENT_FIRST_NON_MELEE_SPELL 1
#define CURRENT_MAX_SPELL             4

enum ActiveStates
{
    ACT_ENABLED  = 0xC100,
    ACT_DISABLED = 0x8100,
    ACT_COMMAND  = 0x0700,
    ACT_REACTION = 0x0600,
    ACT_CAST     = 0x0100,
    ACT_PASSIVE  = 0x0000,
    ACT_DECIDE   = 0x0001
};

/// Pet's behavior
enum ReactStates
{
    REACT_PASSIVE    = 0, //!< pet will not attack
    REACT_DEFENSIVE  = 1, //!< pet will attack only if he or his owner is attacked
    REACT_AGGRESSIVE = 2  //!<
};

/// Pet's Commands
enum CommandStates
{
    COMMAND_STAY    = 0, //!< orders pet to stop moving
    COMMAND_FOLLOW  = 1, //!< orders pet to follow
    COMMAND_ATTACK  = 2, //!< orders pet to attack
    COMMAND_ABANDON = 3  //!< abandons the pet
};

struct CharmSpellEntry
{
    uint16 spellId;
    uint16 active;
};

typedef std::list<Player*> SharedVisionList;

enum CharmType
{
    CHARM_TYPE_CHARM,
    CHARM_TYPE_POSSESS,
};

enum ActionBarIndex
{
    ACTION_BAR_INDEX_START = 0,
    ACTION_BAR_INDEX_PET_SPELL_START = 3,
    ACTION_BAR_INDEX_PET_SPELL_END = 7,
    ACTION_BAR_INDEX_END = 10,
};

#define MAX_UNIT_ACTION_BAR_INDEX (ACTION_BAR_INDEX_END-ACTION_BAR_INDEX_START)

struct CharmInfo
{
    public:
        explicit CharmInfo(Unit* unit);
        ~CharmInfo();
        uint32 GetPetNumber() const
        {
            return m_petnumber;
        }
        void SetPetNumber(uint32 petnumber, bool statwindow);

        void SetCommandState(CommandStates st)
        {
            m_CommandState = st;
        }
        CommandStates GetCommandState()
        {
            return m_CommandState;
        }
        bool HasCommandState(CommandStates state)
        {
            return (m_CommandState == state);
        }
        //void SetReactState(ReactStates st) { m_reactState = st; }
        //ReactStates GetReactState() { return m_reactState; }
        //bool HasReactState(ReactStates state) { return (m_reactState == state); }

        void InitPossessCreateSpells(bool withAttack = true);
        void InitCharmCreateSpells();
        void InitPetActionBar();
        void InitEmptyActionBar(bool withAttack = true);
        //return true if successful
        bool AddSpellToActionBar(uint32 oldid, uint32 newid, ActiveStates newstate = ACT_DECIDE);
        void ToggleCreatureAutocast(uint32 spellid, bool apply);

        UnitActionBarEntry* GetActionBarEntry(uint8 index)
        {
            return &(PetActionBar[index]);
        }
        CharmSpellEntry* GetCharmSpell(uint8 index)
        {
            return &(m_charmspells[index]);
        }

        void SetIsCommandAttack(bool val);
        bool IsCommandAttack();
        void SetIsCommandFollow(bool val);
        bool IsCommandFollow();
        void SetIsAtStay(bool val);
        bool IsAtStay();
        void SetIsFollowing(bool val);
        bool IsFollowing();
        void SetIsReturning(bool val);
        bool IsReturning();
        void SaveStayPosition();
        void GetStayPosition(float& x, float& y, float& z);

    private:
        Unit* m_unit;
        UnitActionBarEntry PetActionBar[10];
        CharmSpellEntry m_charmspells[4];
        CommandStates   m_CommandState;
        //ReactStates     m_reactState;
        uint32          m_petnumber;
        bool            m_barInit;

        //for restoration after charmed
        ReactStates     m_oldReactState;

        bool m_isCommandAttack;
        bool _isCommandFollow;
        bool m_isAtStay;
        bool m_isFollowing;
        bool m_isReturning;
        float m_stayX;
        float m_stayY;
        float m_stayZ;
};

// for clearing special attacks
#define REACTIVE_TIMER_START 4000

enum ReactiveType
{
    REACTIVE_DEFENSE      = 1,
    REACTIVE_HUNTER_PARRY = 2,
    REACTIVE_CRIT         = 3,
    REACTIVE_HUNTER_CRIT  = 4,
    REACTIVE_OVERPOWER    = 5
};

enum
{
    POSITION_UPDATE_DELAY = 400,
};

#define MAX_REACTIVE 6
#define SUMMON_SLOT_PET     0
#define SUMMON_SLOT_TOTEM   1
#define MAX_TOTEM_SLOT      5
#define SUMMON_SLOT_MINIPET 5
#define SUMMON_SLOT_QUEST   6
#define MAX_SUMMON_SLOT     7

// delay time next attack to prevent client attack animation problems
#define ATTACK_DISPLAY_DELAY 200
#define MAX_PLAYER_STEALTH_DETECT_RANGE 30.0f               // max distance for detection targets by player

struct SpellProcEventEntry;                                 // used only privately

class Unit : public WorldObject
{
    friend class ScriptEvent;

    public:
        typedef std::set<Unit*> AttackerSet;
        typedef std::set<Unit*> ControlList;
        typedef std::pair<uint32, uint8> spellEffectPair;
        typedef std::multimap< spellEffectPair, Aura*> AuraMap;
        typedef std::list<Aura*> AuraList;
        typedef std::list<DiminishingReturn> Diminishing;
        typedef std::set<AuraType> AuraTypeSet;
        typedef std::set<uint32> ComboPointHolderSet;

        ~Unit () override;

        void AddToWorld() override;
        void RemoveFromWorld() override;

        void CleanupsBeforeDelete() override;                        // used in ~Creature/~Player (or before mass creature delete to remove cross-references to already deleted units)

        DiminishingLevels GetDiminishing(DiminishingGroup  group);
        void IncrDiminishing(DiminishingGroup group);
        void ApplyDiminishingToDuration(DiminishingGroup  group, int32& duration, Unit* caster, DiminishingLevels Level);
        void ApplyDiminishingAura(DiminishingGroup  group, bool apply);
        void ClearDiminishings()
        {
            m_Diminishing.clear();
        }

        //target dependent checks
        uint32 GetSpellRadiusForTarget(Unit* target, const SpellRadiusEntry* radiusEntry);

        void Update(uint32 time) override;

        void setAttackTimer(WeaponAttackType type, uint32 time)
        {
            m_attackTimer[type] = time;
        }
        void resetAttackTimer(WeaponAttackType type = BASE_ATTACK);
        uint32 getAttackTimer(WeaponAttackType type) const
        {
            return m_attackTimer[type];
        }
        bool isAttackReady(WeaponAttackType type = BASE_ATTACK) const
        {
            return m_attackTimer[type] == 0;
        }
        bool haveOffhandWeapon() const;
		bool UpdateMeleeAttackingState();
		bool CanUseEquippedWeapon(WeaponAttackType attackType) const
		{
			if (IsInFeralForm())
				return false;
			
			switch (attackType)
		    {
		        default:
		        case BASE_ATTACK:
	                return !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED);
		        case OFF_ATTACK:
		        case RANGED_ATTACK:
		        return true;
            }
        }
        bool CanDualWield() const
        {
            return m_canDualWield;
        }
        virtual void SetCanDualWield(bool value) { m_canDualWield = value; }
        float GetCombatReach() const
        {
            return m_floatValues[UNIT_FIELD_COMBATREACH];
        }
        float GetMeleeReach() const
        {
            float reach = m_floatValues[UNIT_FIELD_COMBATREACH];
            return reach > MIN_MELEE_REACH ? reach : MIN_MELEE_REACH;
        }
        bool IsWithinCombatRange(const Unit* obj, float dist2compare) const;
        bool IsWithinMeleeRange(Unit* obj, float dist = MELEE_RANGE) const;
        void GetRandomContactPoint(const Unit* target, float& x, float& y, float& z, float distance2dMin, float distance2dMax) const;
        uint32 m_extraAttacks;
        bool m_canDualWield;

        void _addAttacker(Unit* pAttacker)                  // must be called only from Unit::Attack(Unit*)
        {
            AttackerSet::iterator itr = m_attackers.find(pAttacker);
            if (itr == m_attackers.end())
                m_attackers.insert(pAttacker);
        }
        void _removeAttacker(Unit* pAttacker)               // must be called only from Unit::AttackStop()
        {
            m_attackers.erase(pAttacker);
        }
        Unit* getAttackerForHelper() const;                 // If someone wants to help, who to give them
        bool Attack(Unit* victim, bool meleeAttack);
        void CastStop(uint32 except_spellid = 0);
        bool AttackStop();
        void RemoveAllAttackers();
        AttackerSet const& getAttackers() const
        {
            return m_attackers;
        }
        bool isAttackingPlayer() const;
        Unit* GetVictim() const { return m_attacking; }

        void CombatStop(bool cast = false);
        void CombatStopWithPets(bool cast = false);
        Unit* SelectNearbyTarget(Unit* exclude = NULL, float dist = NOMINAL_MELEE_RANGE) const;
        Player* SelectNearestPlayer(float distance = 0) const;
        void SendMeleeAttackStop(Unit* victim = NULL);
        void SendMeleeAttackStart(Unit* victim);

        // Minipet related functions
        void SetCritterGUID(uint64 guid) { miniPetGuid = guid; }
        uint64 GetCritterGUID() const { return miniPetGuid; }

        void AddUnitState(uint32 f)
        {
            m_state |= f;
        }
        bool HasUnitState(const uint32 f) const
        {
            return (m_state & f);
        }
        void ClearUnitState(uint32 f)
        {
            m_state &= ~f;
        }
        bool CanFreeMove() const
        {
            return !HasUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_FLEEING | UNIT_STATE_IN_FLIGHT |
                                 UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DISTRACTED) && GetOwnerGUID() == 0;
        }

        uint32 HasUnitTypeMask(uint32 mask) const { return mask & m_unitTypeMask; }
        void AddUnitTypeMask(uint32 mask) { m_unitTypeMask |= mask; }
        bool IsSummon() const   { return (m_unitTypeMask & UNIT_MASK_SUMMON) != 0; }
        bool IsGuardian() const { return (m_unitTypeMask & UNIT_MASK_GUARDIAN) != 0; }
        bool IsPet() const      { return (m_unitTypeMask & UNIT_MASK_PET) != 0; }
        bool IsHunterPet() const{ return (m_unitTypeMask & UNIT_MASK_HUNTER_PET) != 0; }
        bool IsTotem() const    { return (m_unitTypeMask & UNIT_MASK_TOTEM) != 0; }

        bool IsDuringRemoveFromWorld() const {return m_duringRemoveFromWorld;}

        Pet* ToPet() { if (IsPet()) return reinterpret_cast<Pet*>(this); else return NULL; }
        Pet const* ToPet() const { if (IsPet()) return reinterpret_cast<Pet const*>(this); else return NULL; }

        uint8 getLevel() const { return uint8(GetUInt32Value(UNIT_FIELD_LEVEL)); }
        uint8 getLevelForTarget(WorldObject const* /*target*/) const override { return getLevel(); }
        void SetLevel(uint32 lvl);
        uint8 getRace() const { return GetByteValue(UNIT_FIELD_BYTES_0, 0); }
        uint32 getRaceMask() const { return 1 << (getRace()-1); }
        uint8 getClass() const { return GetByteValue(UNIT_FIELD_BYTES_0, 1); }
        uint32 getClassMask() const { return 1 << (getClass()-1); }
        uint8 getGender() const { return GetByteValue(UNIT_FIELD_BYTES_0, 2); }

        float GetStat(Stats stat) const { return float(GetUInt32Value(UNIT_FIELD_STAT0+stat)); }
        void SetStat(Stats stat, int32 val) { SetStatInt32Value(UNIT_FIELD_STAT0+stat, val); }
        uint32 GetArmor() const { return GetResistance(SPELL_SCHOOL_NORMAL); }
        void SetArmor(int32 val) { SetResistance(SPELL_SCHOOL_NORMAL, val); }

        uint32 GetResistance(SpellSchools school) const { return GetUInt32Value(UNIT_FIELD_RESISTANCES+school); }
        void SetResistance(SpellSchools school, int32 val) { SetStatInt32Value(UNIT_FIELD_RESISTANCES+school, val); }

        uint32 GetHealth()    const { return GetUInt32Value(UNIT_FIELD_HEALTH); }
        uint32 GetMaxHealth() const { return GetUInt32Value(UNIT_FIELD_MAXHEALTH); }

        bool ShouldRevealHealthTo(Player* player) const;
        void SendHealthUpdateDueToCharm(Player* charmer);

        bool IsFullHealth() const { return GetHealth() == GetMaxHealth(); }
        bool HealthBelowPct(int32 pct) const { return GetHealth() < CountPctFromMaxHealth(pct); }
        bool HealthBelowPctDamaged(int32 pct, uint32 damage) const { return int64(GetHealth()) - int64(damage) < int64(CountPctFromMaxHealth(pct)); }
        bool HealthAbovePct(int32 pct) const { return GetHealth() > CountPctFromMaxHealth(pct); }
        bool HealthAbovePctHealed(int32 pct, uint32 heal) const { return uint64(GetHealth()) + uint64(heal) > CountPctFromMaxHealth(pct); }
        float GetHealthPct() const { return GetMaxHealth() ? 100.f * GetHealth() / GetMaxHealth() : 0.0f; }
        uint32 CountPctFromMaxHealth(int32 pct) const { return CalculatePct(GetMaxHealth(), pct); }
        uint32 CountPctFromCurHealth(int32 pct) const { return CalculatePct(GetHealth(), pct); }

        void SetHealth(  uint32 val);
        void SetMaxHealth(uint32 val);
        inline void SetFullHealth() { SetHealth(GetMaxHealth()); }
        int32 ModifyHealth(int32 val);

        Powers getPowerType() const { return Powers(GetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_POWER_TYPE)); }
        void setPowerType(Powers power);
        uint32 GetPower(Powers power) const { return GetUInt32Value(UNIT_FIELD_POWER1 + power); }
        uint32 GetMaxPower(Powers power) const { return GetUInt32Value(UNIT_FIELD_MAXPOWER1 + power); }
        void SetPower(Powers power, uint32 val);
        void SetMaxPower(Powers power, uint32 val);
        int32 ModifyPower(Powers power, int32 val);
        void ApplyPowerMod(Powers power, uint32 val, bool apply);
        void ApplyMaxPowerMod(Powers power, uint32 val, bool apply);

        uint32 GetAttackTime(WeaponAttackType att) const
        {
            return (uint32)(GetFloatValue(UNIT_FIELD_BASEATTACKTIME + att) / m_modAttackSpeedPct[att]);
        }
        void SetAttackTime(WeaponAttackType att, uint32 val) { SetFloatValue(UNIT_FIELD_BASEATTACKTIME + att, val*m_modAttackSpeedPct[att]); }
        void ApplyAttackTimePercentMod(WeaponAttackType att, float val, bool apply);
        void ApplyCastTimePercentMod(float val, bool apply);

        SheathState GetSheath() const { return SheathState(GetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHEATH_STATE)); }
        virtual void SetSheath(SheathState sheathed) { SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHEATH_STATE, sheathed); }

        // faction template id
        uint32 GetFaction() const { return GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE); }
        void SetFaction(uint32 faction) { SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, faction); }
        FactionTemplateEntry const* GetFactionTemplateEntry() const;

        ReputationRank GetReactionTo(Unit const* target) const;
        ReputationRank static GetFactionReactionTo(FactionTemplateEntry const* factionTemplateEntry, Unit const* target);

        bool IsHostileTo(Unit const* unit) const;
        bool IsHostileToPlayers() const;
        bool IsFriendlyTo(Unit const* unit) const;
        bool IsNeutralToAll() const;
        bool IsInPartyWith(Unit const* unit) const;
        bool IsInRaidWith(Unit const* unit) const;
        void GetPartyMember(std::list<Unit*>& units, float dist);
        void GetRaidMember(std::list<Unit*>& units, float dist);
        bool IsContestedGuard() const
        {
            if (FactionTemplateEntry const* entry = GetFactionTemplateEntry())
                return entry->IsContestedGuardFaction();

            return false;
        }
        bool IsInSanctuary() const;
        bool IsPvP() const
        {
            return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);
        }
        void SetPvP(bool state)
        {
            if (state) SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);
            else RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);
        }
        uint32 GetCreatureType() const;
        uint32 GetCreatureTypeMask() const
        {
            uint32 creatureType = GetCreatureType();
            return (creatureType >= 1) ? (1 << (creatureType - 1)) : 0;
        }

        uint8 GetStandState() const { return GetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_STAND_STATE); }
        bool IsSitState() const;
        bool IsStandState() const;
        void SetStandState(uint8 state);

        void  SetStandFlags(uint8 flags) { SetByteFlag(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_VIS_FLAG, flags); }
        void  RemoveStandFlags(uint8 flags) { RemoveByteFlag(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_VIS_FLAG, flags); }

        bool IsMounted() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT); }
        uint32 GetMountID() const { return GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID); }
        void Mount(uint32 mount, uint32 spellId = 0);
        void Dismount();

        bool HasShapeshiftChangingModel() const;

        uint32 GetMaxSkillValueForLevel(Unit const* target = nullptr) const { return (target ? getLevelForTarget(target) : getLevel()) * 5; }
        void RemoveSpellbyDamageTaken(uint32 damage, uint32 spell);
        uint32 DealDamage(Unit* victim, uint32 damage, CleanDamage const* cleanDamage = NULL, DamageEffectType damagetype = DIRECT_DAMAGE, SpellSchoolMask damageSchoolMask = SPELL_SCHOOL_MASK_NORMAL, SpellEntry const* spellProto = NULL, bool durabilityLoss = true);
        int32 DealHeal(Unit* pVictim, uint32 addhealth, SpellEntry const* spellProto, bool critical = false);
        void Kill(Unit* victim, bool durabilityLoss = true);

        void ProcDamageAndSpell(Unit* victim, uint32 procAttacker, uint32 procVictim, uint32 procEx, uint32 amount, WeaponAttackType attType = BASE_ATTACK, SpellEntry const* procSpell = NULL, bool canTrigger = true);
        void ProcDamageAndSpellFor(bool isVictim, Unit* pTarget, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, SpellEntry const* procSpell, uint32 damage);

        void HandleEmoteCommand(uint32 anim_id);
        void AttackerStateUpdate (Unit* victim, WeaponAttackType attType = BASE_ATTACK, bool extra = false);

        float MeleeMissChanceCalc(const Unit* victim, WeaponAttackType attType) const;

        void CalculateMeleeDamage(Unit* victim, uint32 damage, CalcDamageInfo* damageInfo, WeaponAttackType attackType = BASE_ATTACK);
        void DealMeleeDamage(CalcDamageInfo* damageInfo, bool durabilityLoss);

        void CalculateSpellDamageTaken(SpellNonMeleeDamage* damageInfo, int32 damage, SpellEntry const* spellInfo, WeaponAttackType attackType = BASE_ATTACK, bool crit = false);
        void DealSpellDamage(SpellNonMeleeDamage* damageInfo, bool durabilityLoss);

        float MeleeSpellMissChance(const Unit* victim, WeaponAttackType attType, int32 skillDiff, uint32 spellId) const;
        SpellMissInfo MeleeSpellHitResult(Unit* victim, SpellEntry const* spell, bool cMiss = true);
        SpellMissInfo MagicSpellHitResult(Unit* victim, SpellEntry const* spell);
        SpellMissInfo SpellHitResult(Unit* victim, SpellEntry const* spell, bool canReflect = false, bool canMiss = true);

        float GetUnitDodgeChance()    const;
        float GetUnitParryChance()    const;
        float GetUnitBlockChance()    const;
        float GetUnitCriticalChance(WeaponAttackType attackType, const Unit* victim) const;
        int32 GetMechanicResistChance(const SpellEntry* spell);
        bool CanUseAttackType(uint8 attacktype) const;

        virtual uint32 GetShieldBlockValue() const = 0;
        uint32 GetUnitMeleeSkill(Unit const* target = NULL) const
        {
            return (target ? getLevelForTarget(target) : getLevel()) * 5;
        }
        uint32 GetDefenseSkillValue(Unit const* target = NULL) const;
        uint32 GetWeaponSkillValue(WeaponAttackType attType, Unit const* target = NULL) const;
        float GetWeaponProcChance() const;
        float GetPPMProcChance(uint32 WeaponSpeed, float PPM) const;

        MeleeHitOutcome RollMeleeOutcomeAgainst (const Unit* victim, WeaponAttackType attType) const;
        MeleeHitOutcome RollMeleeOutcomeAgainst (const Unit* victim, WeaponAttackType attType, int32 crit_chance, int32 miss_chance, int32 dodge_chance, int32 parry_chance, int32 block_chance, bool SpellCasted) const;

        bool IsVendor()       const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR); }
        bool IsTrainer()      const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER); }
        bool IsQuestGiver()   const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER); }
        bool IsGossip()       const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP); }
        bool IsTaxi()         const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_FLIGHTMASTER); }
        bool IsGuildMaster()  const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_PETITIONER); }
        bool IsBattleMaster() const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BATTLEMASTER); }
        bool IsBanker()       const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BANKER); }
        bool IsInnkeeper()    const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_INNKEEPER); }
        bool IsSpiritHealer() const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER); }
        bool IsSpiritGuide()  const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITGUIDE); }
        bool IsTabardDesigner()const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TABARDDESIGNER); }
        bool IsAuctioner()    const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_AUCTIONEER); }
        bool IsArmorer()      const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_REPAIR); }
        bool isServiceProvider() const
        {
            return HasFlag(UNIT_NPC_FLAGS,
                           UNIT_NPC_FLAG_VENDOR | UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_FLIGHTMASTER |
                           UNIT_NPC_FLAG_PETITIONER | UNIT_NPC_FLAG_BATTLEMASTER | UNIT_NPC_FLAG_BANKER |
                           UNIT_NPC_FLAG_INNKEEPER | UNIT_NPC_FLAG_SPIRITHEALER |
                           UNIT_NPC_FLAG_SPIRITGUIDE | UNIT_NPC_FLAG_TABARDDESIGNER | UNIT_NPC_FLAG_AUCTIONEER);
        }
        bool IsSpiritService() const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER | UNIT_NPC_FLAG_SPIRITGUIDE); }
        bool IsCritter() const { return GetCreatureType() == CREATURE_TYPE_CRITTER; }
        bool IsPlayer() const{ return GetTypeId() == TYPEID_PLAYER; }
        bool IsInFlight()  const { return HasUnitState(UNIT_STATE_IN_FLIGHT); }

        // Is the unit casting, or has recently casted a combat spell but not in combat yet?
        bool IsInitiatingCombat() const { return m_initiatingCombat; }
        bool IsInCombat()  const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT); }
        bool IsPetInCombat() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT); }
        bool IsInCombatWith(Unit const* who) const;
        void CombatStart(Unit* target, bool initialAggro = true);
        void SetInCombatState(bool PvP, Unit* enemy = NULL);
        void SetInitiatingCombat(bool flag) { m_initiatingCombat = flag; }

        void SetInCombatWith(Unit* enemy);
        void ClearInCombat();
        void ClearInPetCombat();
        uint32 m_CombatTimer;
        uint32 IsInPvPCombat() const { return m_CombatTimer != 0; }

        uint32 GetAuraCount(uint32 spellId) const;
        bool HasAuraType(AuraType auraType) const;
        bool HasAuraTypeWithCaster(AuraType auratype, uint64 caster) const;
        bool HasAuraTypeWithMiscvalue(AuraType auratype, int32 miscvalue) const;
        bool HasAuraTypeWithFamilyFlags(AuraType auraType, uint32 familyName,  uint64 familyFlags) const;
        bool HasNegativeAuraWithInterruptFlag(uint32 flag);
        bool HasAura(uint32 spellId, uint8 effIndex = 0) const { return m_Auras.find(spellEffectPair(spellId, effIndex)) != m_Auras.end(); }
        bool HasHigherRankOfAura(uint32 spellid, uint8 effIndex) const;

        bool virtual HasSpell(uint32 /*spellID*/) const
        {
            return false;
        }

        bool HasBreakableByDamageAuraType(AuraType type, uint32 excludeAura = 0) const;
        bool HasBreakableByDamageCrowdControlAura(Unit* excludeCasterChannel = NULL) const;

        bool HasStealthAura()      const { return HasAuraType(SPELL_AURA_MOD_STEALTH); }
        bool HasInvisibilityAura() const { return HasAuraType(SPELL_AURA_MOD_INVISIBILITY); }
        bool IsFeared()  const { return HasAuraType(SPELL_AURA_MOD_FEAR); }
        bool IsInRoots() const { return HasAuraType(SPELL_AURA_MOD_ROOT); }
        bool IsPolymorphed() const;

        bool isFrozen() const;
        bool canAttack(Unit const* target, bool force = true) const;

        bool isTargetableForAttack(bool checkFakeDeath = true) const;

        bool IsValidAttackTarget(Unit const* target) const;
        bool _IsValidAttackTarget(Unit const* target, SpellEntry const* bySpell, WorldObject const* obj = NULL) const;

        bool isAttackableByAOE(float x = 0, float y = 0, float z = 0, bool LosCheck = false) const;

        bool IsValidAssistTarget(Unit const* target) const;
        bool _IsValidAssistTarget(Unit const* target, SpellEntry const* bySpell) const;

        virtual bool IsInWater() const;
        virtual bool IsUnderWater() const;
        bool isInAccessiblePlaceFor(Creature const* c) const;
        virtual uint32 GetHighestLearnedRankOf(uint32 spellid) const
        {
            return spellid;
        }

        void SendHealSpellLog(Unit* victim, uint32 SpellID, uint32 Damage, bool critical = false);
        void SendEnergizeSpellLog(Unit* victim, uint32 SpellID, uint32 Damage, Powers powertype);
        void EnergizeBySpell(Unit* victim, uint32 SpellID, uint32 Damage, Powers powertype);
        uint32 SpellNonMeleeDamageLog(Unit* victim, uint32 spellID, uint32 damage, bool isTriggeredSpell = false, bool useSpellDamage = true);
        void CastSpell(Unit* Victim, uint32 spellId, bool triggered, Item* castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        void CastSpell(Unit* Victim, SpellEntry const* spellInfo, bool triggered, Item* castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        void CastCustomSpell(Unit* Victim, uint32 spellId, int32 const* bp0, int32 const* bp1, int32 const* bp2, bool triggered, Item* castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        void CastCustomSpell(uint32 spellId, SpellValueMod mod, uint32 value, Unit* Victim = NULL, bool triggered = true, Item* castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        void CastCustomSpell(uint32 spellId, CustomSpellValues const& value, Unit* Victim = NULL, bool triggered = true, Item* castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        void CastSpell(float x, float y, float z, uint32 spellId, bool triggered, Item* castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        void CastSpell(GameObject* go, uint32 spellId, bool triggered, Item* castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        void AddAura(uint32 spellId, Unit* target);

        void DeMorph();

        void SendAttackStateUpdate(CalcDamageInfo* damageInfo);
        void SendAttackStateUpdate(uint32 HitInfo, Unit* target, uint8 SwingType, SpellSchoolMask damageSchoolMask, uint32 Damage, uint32 AbsorbDamage, uint32 Resist, VictimState TargetState, uint32 BlockedAmount);
        void SendSpellNonMeleeDamageLog(SpellNonMeleeDamage* log);
        void SendSpellNonMeleeDamageLog(Unit* target, uint32 SpellID, uint32 Damage, SpellSchoolMask damageSchoolMask, uint32 AbsorbedDamage, uint32 Resist, bool PhysicalDamage, uint32 Blocked, bool CriticalHit = false);
        void SendSpellMiss(Unit* target, uint32 spellID, SpellMissInfo missInfo);
        void SendSpellDamageImmune(Unit* target, uint32 spellId);

        virtual bool SetPosition(float x, float y, float z, float orientation, bool teleport = false);
        bool SetPosition(const Position& pos, bool teleport = false)
        {
            return SetPosition(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), teleport);
        }

        void NearTeleportTo(float x, float y, float z, float orientation, bool casting = false);
        void SendTeleportPacket(Position& pos);

        void SendMovementFlagUpdate(bool self = false);

        void BuildHeartBeatMsg(WorldPacket* data) const;
        void BuildMovementPacket(ByteBuffer *data) const;

        bool isMoving() const
        {
            return HasUnitMovementFlag(MOVEMENTFLAG_MOVING) && !HasUnitMovementFlag(MOVEMENTFLAG_ROOT);
        }
        bool IsAlive() const { return (m_deathState == ALIVE); }
        bool isDying() const { return (m_deathState == JUST_DIED); }
        bool isDead() const { return (m_deathState == DEAD || m_deathState == CORPSE); }
        DeathState getDeathState() const { return m_deathState; }
        virtual void setDeathState(DeathState s);           // overwrited in Creature/Player/Pet

        void SetOwnerGUID(uint64 owner)
        {
            SetUInt64Value(UNIT_FIELD_SUMMONEDBY, owner);
        }
        uint64 GetOwnerGUID() const
        {
            return  GetUInt64Value(UNIT_FIELD_SUMMONEDBY);
        }
        uint64 GetCreatorGUID() const
        {
            return GetUInt64Value(UNIT_FIELD_CREATEDBY);
        }
        void SetCreatorGUID(uint64 creator)
        {
            SetUInt64Value(UNIT_FIELD_CREATEDBY, creator);
        }
        uint64 GetMinionGUID() const
        {
            return GetUInt64Value(UNIT_FIELD_SUMMON);
        }
        void SetMinionGUID(uint64 guid)
        {
            SetUInt64Value(UNIT_FIELD_SUMMON, guid);
        }
        uint64 GetCharmerGUID() const
        {
            return GetUInt64Value(UNIT_FIELD_CHARMEDBY);
        }
        void SetCharmerGUID(uint64 owner)
        {
            SetUInt64Value(UNIT_FIELD_CHARMEDBY, owner);
        }
        uint64 GetCharmGUID() const
        {
            return  GetUInt64Value(UNIT_FIELD_CHARM);
        }
        void SetPetGUID(uint64 guid)
        {
            m_SummonSlot[SUMMON_SLOT_PET] = guid;
        }
        uint64 GetPetGUID() const
        {
            return m_SummonSlot[SUMMON_SLOT_PET];
        }

        bool IsControlledByPlayer() const
        {
            return m_ControlledByPlayer;
        }
        uint64 GetCharmerOrOwnerGUID() const
        {
            return GetCharmerGUID() ? GetCharmerGUID() : GetOwnerGUID();
        }
        uint64 GetCharmerOrOwnerOrOwnGUID() const
        {
            if (uint64 guid = GetCharmerOrOwnerGUID())
                return guid;
            return GetGUID();
        }
        bool IsCharmedOwnedByPlayerOrPlayer() const
        {
            return IS_PLAYER_GUID(GetCharmerOrOwnerOrOwnGUID());
        }
        Player* GetAffectingPlayer() const;

        Player* GetSpellModOwner() const;

        Unit* GetOwner() const;
        Guardian* GetGuardianPet() const;
        Minion* GetFirstMinion() const;
        Unit* GetCharmer() const;
        Unit* GetCharm() const;
        Unit* GetCharmerOrOwner() const
        {
            return GetCharmerGUID() ? GetCharmer() : GetOwner();
        }
        Unit* GetCharmerOrOwnerOrSelf() const
        {
            if (Unit* u = GetCharmerOrOwner())
                return u;

            return (Unit*)this;
        }
        Player* GetCharmerOrOwnerPlayerOrPlayerItself() const;

        void SetMinion(Minion* minion, bool apply);
        void RemoveAllMinionsByEntry(uint32 entry);
        void SetCharm(Unit* target, bool apply);
        void SetCharmedBy(Unit* charmer, CharmType type);
        void RemoveCharmedBy(Unit* charmer);
        void RestoreFaction();

        ControlList m_Controlled;
        Unit* GetFirstControlled() const;
        void RemoveAllControlled();

        bool isCharmed() const
        {
            return GetCharmerGUID() != 0;
        }
        bool isPossessed() const
        {
            return HasUnitState(UNIT_STATE_POSSESSED);
        }
        bool isPossessedByPlayer() const
        {
            return HasUnitState(UNIT_STATE_POSSESSED) && IS_PLAYER_GUID(GetCharmerGUID());
        }
        bool isPossessing() const
        {
            if (Unit* u = GetCharm())
                return u->isPossessed();
            else
                return false;
        }
        bool isPossessing(Unit* u) const
        {
            return u->isPossessed() && GetCharmGUID() == u->GetGUID();
        }

        CharmInfo* GetCharmInfo()
        {
            return m_charmInfo;
        }
        CharmInfo* InitCharmInfo();
        void       DeleteCharmInfo();
        void UpdateCharmAI();
        SharedVisionList const& GetSharedVisionList()
        {
            return m_sharedVision;
        }
        void AddPlayerToVision(Player* plr);
        void RemovePlayerFromVision(Player* plr);
        bool HasSharedVision() const
        {
            return !m_sharedVision.empty();
        }
        void RemoveBindSightAuras();
        void RemoveCharmAuras();

        Pet* CreateTamedPetFrom(Creature* creatureTarget, uint32 spell_id = 0);

        bool AddAura(Aura* aur);

        void RemoveAura(AuraMap::iterator& i, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAura(uint32 spellId, uint32 effindex, Aura* except = NULL);
        void RemoveSingleAuraFromStackByDispel(uint32 spellId);
        void RemoveSingleAuraFromStack(uint32 spellId, uint32 effindex);
        void RemoveAurasDueToSpell(uint32 spellId, Aura* except = NULL);
        void RemoveAurasDueToItemSpell(Item* castItem, uint32 spellId);
        void RemoveAurasByCasterSpell(uint32 spellId, uint64 casterGUID);
        void SetAurasDurationByCasterSpell(uint32 spellId, uint64 casterGUID, int32 duration);
        Aura* GetAuraByCasterSpell(uint32 spellId, uint64 casterGUID);
        void RemoveAurasDueToSpellByDispel(uint32 spellId, uint64 casterGUID, Unit* dispeler);
        void RemoveAurasDueToSpellBySteal(uint32 spellId, uint64 casterGUID, Unit* stealer);
        void RemoveAurasByType(AuraType auraType, uint64 casterGUID = 0, Aura* except = NULL, bool negative = true, bool positive = true);
        void RemoveAurasDueToSpellByCancel(uint32 spellId);
        void RemoveAurasAtChanneledTarget(SpellEntry const* spellInfo, Unit* caster);
        void RemoveNotOwnSingleTargetAuras();

        void RemoveSpellsCausingAura(AuraType auraType);
        void RemoveAuraTypeByCaster(AuraType auraType, uint64 casterGUID);
        void RemoveRankAurasDueToSpell(uint32 spellId);
        bool RemoveNoStackAurasDueToAura(Aura* Aur);
        void RemoveAurasWithInterruptFlags(uint32 flags, uint32 except = 0);
        void RemoveAurasWithAttribute(uint32 flags);
        void RemoveAurasWithDispelType(DispelType type);
        void RemoveMovementImpairingAuras();

        void RemoveAllAuras();
        void RemoveArenaAuras();
        void RemoveAllAurasOnDeath();
        void DelayAura(uint32 spellId, uint32 effindex, int32 delaytime);

        float GetResistanceBuffMods(SpellSchools school, bool positive) const
        {
            return GetFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE + school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE + school);
        }
        void SetResistanceBuffMods(SpellSchools school, bool positive, float val)
        {
            SetFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE + school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE + school, val);
        }
        void ApplyResistanceBuffModsMod(SpellSchools school, bool positive, float val, bool apply)
        {
            ApplyModSignedFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE + school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE + school, val, apply);
        }
        void ApplyResistanceBuffModsPercentMod(SpellSchools school, bool positive, float val, bool apply)
        {
            ApplyPercentModFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE + school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE + school, val, apply);
        }
        void InitStatBuffMods()
        {
            for (int i = STAT_STRENGTH; i < MAX_STATS; ++i) SetFloatValue(UNIT_FIELD_POSSTAT0 + i, 0);
            for (int i = STAT_STRENGTH; i < MAX_STATS; ++i) SetFloatValue(UNIT_FIELD_NEGSTAT0 + i, 0);
        }
        void ApplyStatBuffMod(Stats stat, float val, bool apply)
        {
            ApplyModSignedFloatValue((val > 0 ? UNIT_FIELD_POSSTAT0 + stat : UNIT_FIELD_NEGSTAT0 + stat), val, apply);
        }
        void ApplyStatPercentBuffMod(Stats stat, float val, bool apply)
        {
            ApplyPercentModFloatValue(UNIT_FIELD_POSSTAT0 + stat, val, apply);
            ApplyPercentModFloatValue(UNIT_FIELD_NEGSTAT0 + stat, val, apply);
        }
        void SetCreateStat(Stats stat, float val)
        {
            m_createStats[stat] = val;
        }
        void SetCreateHealth(uint32 val)
        {
            SetUInt32Value(UNIT_FIELD_BASE_HEALTH, val);
        }
        uint32 GetCreateHealth() const
        {
            return GetUInt32Value(UNIT_FIELD_BASE_HEALTH);
        }
        void SetCreateMana(uint32 val)
        {
            SetUInt32Value(UNIT_FIELD_BASE_MANA, val);
        }
        uint32 GetCreateMana() const
        {
            return GetUInt32Value(UNIT_FIELD_BASE_MANA);
        }
        uint32 GetCreatePowers(Powers power) const;
        float GetPosStat(Stats stat) const
        {
            return GetFloatValue(UNIT_FIELD_POSSTAT0 + stat);
        }
        float GetNegStat(Stats stat) const
        {
            return GetFloatValue(UNIT_FIELD_NEGSTAT0 + stat);
        }
        float GetCreateStat(Stats stat) const
        {
            return m_createStats[stat];
        }

        void SetCurrentCastedSpell(Spell* pSpell);
        virtual void ProhibitSpellSchool(SpellSchoolMask /*idSchoolMask*/, uint32 /*unTimeMs*/) { }
        void InterruptSpell(CurrentSpellTypes spellType, bool withDelayed = true, bool withInstant = true);
        void FinishSpell(CurrentSpellTypes spellType, bool ok = true);

        // set withDelayed to true to account delayed spells as casted
        // delayed+channeled spells are always accounted as casted
        // we can skip channeled or delayed checks using flags
        bool IsNonMeleeSpellCast(bool withDelayed, bool skipChanneled = false, bool skipAutorepeat = false) const;

        // set withDelayed to true to interrupt delayed spells too
        // delayed+channeled spells are always interrupted
        void InterruptNonMeleeSpells(bool withDelayed, uint32 spellid = 0, bool withInstant = true);

        uint64 GetTarget() const { return GetUInt64Value(UNIT_FIELD_TARGET); }
        void SetTarget(uint64 guid = 0) { SetUInt64Value(UNIT_FIELD_TARGET, guid); };

        Spell* GetCurrentSpell(CurrentSpellTypes spellType) const
        {
            return m_currentSpells[spellType];
        }
        Spell* GetCurrentSpell(uint32 spellType) const
        {
            return m_currentSpells[spellType];
        }
        Spell* FindCurrentSpellBySpellId(uint32 spell_id) const;
        int32 GetCurrentSpellCastTime(uint32 spell_id) const;

        // Check if our current channel spell has attribute SPELL_ATTR5_CAN_CHANNEL_WHEN_MOVING
        bool CanMoveDuringChannel() const;

        uint64 m_SummonSlot[MAX_SUMMON_SLOT];
        uint64 m_ObjectSlot[4];

        uint32 m_ShapeShiftFormSpellId;
        uint32 m_ShapeShiftModelId;
        ShapeshiftForm m_form;
        bool IsInFeralForm() const
        {
            return m_form == FORM_CAT || m_form == FORM_BEAR || m_form == FORM_DIREBEAR;
        }
        bool IsInTravelForm() const
        {
            return m_form == FORM_TRAVEL;
        }
        bool IsInFlightForm() const
        {
            return m_form == FORM_FLIGHT_EPIC || m_form == FORM_FLIGHT;
        }
        float m_modMeleeHitChance;
        float m_modRangedHitChance;
        float m_modSpellHitChance;
        int32 m_baseSpellCritChance;

        float m_threatModifier[MAX_SPELL_SCHOOL];
        float m_modAttackSpeedPct[3];

        uint32 _oldFactionId;

        // Event handler
        EventProcessor m_Events;

        // stat system
        bool HandleStatModifier(UnitMods unitMod, UnitModifierType modifierType, float amount, bool apply);
        void SetModifierValue(UnitMods unitMod, UnitModifierType modifierType, float value)
        {
            m_auraModifiersGroup[unitMod][modifierType] = value;
        }
        float GetModifierValue(UnitMods unitMod, UnitModifierType modifierType) const;
        float GetTotalStatValue(Stats stat) const;
        float GetTotalAuraModValue(UnitMods unitMod) const;
        SpellSchools GetSpellSchoolByAuraGroup(UnitMods unitMod) const;
        Stats GetStatByAuraGroup(UnitMods unitMod) const;
        Powers GetPowerTypeByAuraGroup(UnitMods unitMod) const;
        bool CanModifyStats() const
        {
            return m_canModifyStats;
        }
        void SetCanModifyStats(bool modifyStats)
        {
            m_canModifyStats = modifyStats;
        }
        virtual bool UpdateStats(Stats stat) = 0;
        virtual bool UpdateAllStats() = 0;
        virtual void UpdateResistances(uint32 school) = 0;
        virtual void UpdateAllResistances();
        virtual void UpdateArmor() = 0;
        virtual void UpdateMaxHealth() = 0;
        virtual void UpdateMaxPower(Powers power) = 0;
        virtual void UpdateAttackPowerAndDamage(bool ranged = false) = 0;
        virtual void UpdateDamagePhysical(WeaponAttackType attType);;
        float GetTotalAttackPowerValue(WeaponAttackType attType) const;
        float GetWeaponDamageRange(WeaponAttackType attType , WeaponDamageRange type) const;
        void SetBaseWeaponDamage(WeaponAttackType attType, WeaponDamageRange damageRange, float value) { m_weaponDamage[attType][damageRange] = value; }
        virtual void CalculateMinMaxDamage(WeaponAttackType attType, bool normalized, bool addTotalPct, float& minDamage, float& maxDamage) = 0;
        uint32 CalculateDamage(WeaponAttackType attType, bool normalized, bool addTotalPct);
        float GetAPMultiplier(WeaponAttackType attType, bool normalized);

        bool isInFrontInMap(Unit const* target, float distance, float arc = M_PI) const;
        void SetInFront(Unit const* target)
        {
            if (!HasUnitState(UNIT_STATE_CANNOT_TURN))
                SetOrientation(GetAngle(target));
        }
        void SetInFront(float x, float y)
        {
            if(!HasUnitState(UNIT_STATE_CANNOT_TURN)) 
                SetOrientation(GetAngle(x,y));
        }

        bool isInBackInMap(Unit const* target, float distance, float arc = M_PI) const;
        void SetFacingTo(float ori);
        void SetFacingToObject(WorldObject* pObject);

        // Visibility system
        bool IsVisible() const { return (m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_GM) > SEC_PLAYER) ? false : true; }
        void SetVisible(bool x);

        // common function for visibility checks for player/creatures with detection code
        bool isValid() const override { return WorldObject::isValid(); }       
        void UpdateObjectVisibility(bool forced = true) override;

        bool isInvisibleForAlive() const;

        AuraList&       GetSingleCastAuras()
        {
            return m_scAuras;
        }
        AuraList const& GetSingleCastAuras() const
        {
            return m_scAuras;
        }

        SpellImmuneList m_spellImmune[MAX_SPELL_IMMUNITY];
        uint32 m_lastSanctuaryTime;

        // Threat related methods
        bool CanHaveThreatList() const;
        void AddThreat(Unit* victim, float threat, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL, SpellEntry const* threatSpell = NULL);
        void AddAssistThreat(Unit* victim, float threatToAdd, SpellEntry const* threatSpell);
        float ApplyTotalThreatModifier(float threat, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL);
        void DeleteThreatList();
        void TauntApply(Unit* victim);
        void TauntFadeOut(Unit* taunter);
        ThreatManager& getThreatManager()
        {
            return m_ThreatManager;
        }
        void addHatedBy(HostileReference* pHostileReference)
        {
            m_HostileRefManager.insertFirst(pHostileReference);
        };
        void removeHatedBy(HostileReference* /*pHostileReference*/)
        {
            /* nothing to do yet */
        }
        HostileRefManager& getHostileRefManager()
        {
            return m_HostileRefManager;
        }

        Aura* GetAura(uint32 spellId, uint32 effindex);
        AuraMap&       GetAuras()
        {
            return m_Auras;
        }
        AuraMap const& GetAuras() const
        {
            return m_Auras;
        }
        AuraList const& GetAurasByType(AuraType type) const
        {
            return m_modAuras[type];
        }
        void ApplyAuraProcTriggerDamage(Aura* aura, bool apply);

        int32 GetTotalAuraModifier(AuraType auratype) const;
        float GetTotalAuraMultiplier(AuraType auratype) const;
        int32 GetMaxPositiveAuraModifier(AuraType auratype) const;
        int32 GetMaxNegativeAuraModifier(AuraType auratype) const;

        int32 GetTotalAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        float GetTotalAuraMultiplierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        int32 GetMaxPositiveAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        int32 GetMaxNegativeAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;

        int32 GetTotalAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
        float GetTotalAuraMultiplierByMiscValue(AuraType auratype, int32 misc_value) const;
        int32 GetMaxPositiveAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
        int32 GetMaxNegativeAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;

        Aura* GetDummyAura(uint32 spell_id) const;

        uint32 m_AuraFlags;

        uint32 GetInterruptMask() const
        {
            return m_interruptMask;
        }
        void AddInterruptMask(uint32 mask)
        {
            m_interruptMask |= mask;
        }
        void UpdateInterruptMask();

        uint32 GetDisplayId()
        {
            return GetUInt32Value(UNIT_FIELD_DISPLAYID);
        }
        void SetDisplayId(uint32 modelId);
        uint32 GetNativeDisplayId()
        {
            return GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID);
        }
        void SetNativeDisplayId(uint32 modelId)
        {
            SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, modelId);
        }
        void setTransForm(uint32 spellid)
        {
            m_transform = spellid;
        }
        uint32 getTransForm() const
        {
            return m_transform;
        }
        // at any changes to scale and/or displayId
        void UpdateModelData();

        void AddDynObject(DynamicObject* dynObj);
        void RemoveDynObject(uint32 spellid);
        void RemoveDynObjectWithGUID(uint64 guid)
        {
            m_dynObjGUIDs.remove(guid);
        }
        void RemoveAllDynObjects();
        void AddGameObject(GameObject* gameObj);
        void RemoveGameObject(GameObject* gameObj, bool del);
        void RemoveGameObject(uint32 spellid, bool del);
        void RemoveAllGameObjects();
        DynamicObject* GetDynObject(uint32 spellId, uint32 effIndex);
        DynamicObject* GetDynObject(uint32 spellId);
        void ModifyAuraState(AuraState flag, bool apply);
        bool HasAuraState(AuraState flag) const
        {
            return HasFlag(UNIT_FIELD_AURASTATE, 1 << (flag - 1));
        }
        void UnsummonAllTotems();
        int32 SpellBaseDamageBonus(SpellSchoolMask schoolMask);
        int32 SpellBaseHealingBonus(SpellSchoolMask schoolMask);
        int32 SpellBaseDamageBonusForVictim(SpellSchoolMask schoolMask, Unit* victim);
        int32 SpellBaseHealingBonusForVictim(SpellSchoolMask schoolMask, Unit* victim);
        uint32 SpellDamageBonus(Unit* victim, SpellEntry const* spellProto, uint32 damage, DamageEffectType damagetype);
        uint32 SpellHealingBonus(SpellEntry const* spellProto, uint32 healamount, DamageEffectType damagetype, Unit* victim);
        bool   isSpellBlocked(Unit* victim, SpellEntry const* spellProto, WeaponAttackType attackType = BASE_ATTACK);
        bool   isSpellCrit(Unit* victim, SpellEntry const* spellProto, SpellSchoolMask schoolMask, WeaponAttackType attackType = BASE_ATTACK);
        uint32 SpellCriticalBonus(SpellEntry const* spellProto, uint32 damage, Unit* victim);

        void SetLastManaUse(uint32 spellCastTime)
        {
            m_lastManaUse = spellCastTime;
        }
        bool IsUnderLastManaUseEffect() const;

        void SetContestedPvP(Player* attackedPlayer = NULL);

        void MeleeDamageBonus(Unit* victim, uint32* damage, WeaponAttackType attType, SpellEntry const* spellProto = NULL);
        uint32 GetCastingTimeForBonus(SpellEntry const* spellProto, DamageEffectType damagetype, uint32 CastingTime);

        void ApplySpellImmune(uint32 spellId, uint32 op, uint32 type, bool apply);
        void ApplySpellDispelImmunity(const SpellEntry* spellProto, DispelType type, bool apply);
        virtual bool IsImmuneToSpell(SpellEntry const* spellInfo, bool useCharges = false); // redefined in Creature
        bool IsImmunedToDamage(SpellSchoolMask meleeSchoolMask, bool useCharges = false);
        bool IsImmunedToDamage(SpellEntry const* spellInfo) const;
        virtual bool IsImmuneToSpellEffect(SpellEntry const* spellInfo, uint32 index, bool castOnSelf) const; // redefined in Creature

        static bool IsDamageReducedByArmor(SpellSchoolMask damageSchoolMask, SpellEntry const* spellInfo = NULL, uint8 effIndex = MAX_SPELL_EFFECTS);
        uint32 CalculateEffectiveMagicResistance(Unit* attacker, SpellSchoolMask schoolMask) const;
        float CalculateMagicResistanceMitigation(Unit* attacker, uint32 resistance, bool binary) const;
        uint32 CalcArmorReducedDamage(Unit* victim, const uint32 damage);
        void CalcAbsorbResist(Unit* victim, SpellSchoolMask schoolMask, DamageEffectType damagetype, const uint32 damage, uint32* absorb, uint32* resist, SpellEntry const* spellInfo = NULL, bool binary = false);

        void  UpdateSpeed(UnitMoveType mtype, bool forced);
        float GetSpeed(UnitMoveType mtype) const;
        float GetSpeedRate(UnitMoveType mtype) const
        {
            return m_speed_rate[mtype];
        }
        void SetSpeed(UnitMoveType mtype, float rate, bool forced = false);

        bool isHover() const
        {
            return HasAuraType(SPELL_AURA_HOVER);
        }

        void _RemoveAllAuraMods();
        void _ApplyAllAuraMods();

        int32 CalculateSpellDamage(SpellEntry const* spellProto, uint8 effect_index, int32 basePoints, Unit const* target);
        int32 CalculateSpellDuration(SpellEntry const* spellProto, uint8 effect_index, Unit const* target);
        float CalculateLevelPenalty(SpellEntry const* spellProto) const;
        void ModSpellCastTime(SpellEntry const* spellProto, int32& castTime, Spell* spell);
        void ModSpellDurationTime(SpellEntry const* spellProto, int32& duration, Spell* spell);

        void addFollower(FollowerReference* pRef)
        {
            m_FollowingRefManager.insertFirst(pRef);
        }
        void removeFollower(FollowerReference* /*pRef*/)
        {
            /* nothing to do yet */
        }
        static Unit* GetUnit(WorldObject& object, uint64 guid);
        static Player* GetPlayer(WorldObject& object, uint64 guid);
        static Creature* GetCreature(WorldObject& object, uint64 guid);

        MotionMaster* GetMotionMaster()
        {
            return &i_motionMaster;
        }

        bool IsStopped() const
        {
            return !(HasUnitState(UNIT_STATE_MOVING));
        }
        void StopMoving();

        void AddUnitMovementFlag(uint32 f) { m_movementInfo.moveFlags |= f; }
        void RemoveUnitMovementFlag(uint32 f) { m_movementInfo.moveFlags &= ~f; }
        bool HasUnitMovementFlag(uint32 f) const { return m_movementInfo.moveFlags & f; }
        uint32 GetUnitMovementFlags() const { return m_movementInfo.moveFlags; }
        void SetUnitMovementFlags(uint32 f) { m_movementInfo.moveFlags = f; }

        Movement::MoveSpline* movespline;

        void SetControlled(bool apply, UnitState state);
        void SetFeared(bool apply/*, uint64 casterGUID = 0, uint32 spellID = 0*/);
        void SetConfused(bool apply/*, uint64 casterGUID = 0, uint32 spellID = 0*/);
        void SetStunned(bool apply);

        void MonsterMoveWithSpeed(float x, float y, float z, float speed, bool generatePath = false, bool forceDestination = false);
        void SendMonsterMoveWithSpeedToCurrentDestination(float speed);

        void AddComboPointHolder(uint32 lowguid)
        {
            m_ComboPointHolders.insert(lowguid);
        }
        void RemoveComboPointHolder(uint32 lowguid)
        {
            m_ComboPointHolders.erase(lowguid);
        }
        void ClearComboPointHolders();

        //----------Pet responses methods-----------------
        void SendPetCastFail(uint32 spellid, SpellCastResult msg);
        void SendPetActionFeedback (uint8 msg);
        void SendPetTalk (uint32 pettalk);
        void SendPetSpellCooldown (uint32 spellid, time_t cooltime);
        void SendPetClearCooldown (uint32 spellid);
        void SendPetAIReaction(uint64 guid);
        //----------End of Pet responses methods----------

        void propagateSpeedChange()
        {
            GetMotionMaster()->propagateSpeedChange();
        }

        // reactive attacks
        void ClearAllReactives();
        void StartReactiveTimer(ReactiveType reactive)
        {
            m_reactiveTimer[reactive] = REACTIVE_TIMER_START;
        }
        void UpdateReactives(uint32 p_time);

        // group updates
        void UpdateAuraForGroup(uint8 slot);

        // pet auras
        typedef std::set<PetAura const*> PetAuraSet;
        PetAuraSet m_petAuras;
        void AddPetAura(PetAura const* petSpell);
        void RemovePetAura(PetAura const* petSpell);

        void SetReducedThreatPercent(uint32 pct, uint64 guid)
        {
            m_reducedThreatPercent = pct;
            m_misdirectionTargetGUID = guid;
        }
        uint32 GetReducedThreatPercent()
        {
            return m_reducedThreatPercent;
        }
        Unit* GetMisdirectionTarget()
        {
            return m_misdirectionTargetGUID ? GetUnit(*this, m_misdirectionTargetGUID) : NULL;
        }
        virtual float GetFollowAngle() const { return static_cast<float>(M_PI/2); }

        bool IsFlying() const   { return m_movementInfo.HasMovementFlag((MovementFlags)(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_LEVITATING)); }
        bool IsFalling() const;
        bool IsWalking() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_WALK_MODE); }
        bool IsRooted() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_ROOT); }

        void SetRooted(bool apply);

        virtual bool SetWalk(bool enable);
        virtual bool SetSwim(bool enable);
        virtual bool SetLevitate(bool apply, bool packetOnly = false);
        virtual bool SetCanFly(bool enable, bool packetOnly = false);
        virtual bool SetWaterWalking(bool enable, bool packetOnly = false);
        virtual bool SetFeatherFall(bool enable, bool packetOnly = false);
        virtual bool SetHover(bool enable, bool packetOnly = false);

        bool IsLevitating() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_LEVITATING); }

        bool IsAIEnabled, NeedChangeAI;
        uint64 LastCharmerGUID;
        bool m_ControlledByPlayer;

        // Part of Evade mechanics
        time_t GetLastDamagedTime() const { return _lastDamagedTime; }
        void SetLastDamagedTime(time_t val) { _lastDamagedTime = val; }

        TempSummon* ToTempSummon() { if (IsSummon()) return reinterpret_cast<TempSummon*>(this); else return NULL; }
        TempSummon const* ToTempSummon() const { if (IsSummon()) return reinterpret_cast<TempSummon const*>(this); else return NULL; }

    protected:
        explicit Unit (bool isWorldObject);

        UnitAI* i_AI, *i_disabledAI;

        void _UpdateSpells(uint32 time);
        void _DeleteAuras();

        void _UpdateAutoRepeatSpell();
        bool m_AutoRepeatFirstCast;

        // Unit is initiating combat by casting a spell at a target
        bool m_initiatingCombat;

        uint32 m_attackTimer[MAX_ATTACK];

        float m_createStats[MAX_STATS];

        AttackerSet m_attackers;
        Unit* m_attacking;

        DeathState m_deathState;

        AuraMap m_Auras;
        AuraMap::iterator m_AurasUpdateIterator;
        uint32 m_removedAurasCount;

        typedef std::list<uint64> DynObjectGUIDs;
        DynObjectGUIDs m_dynObjGUIDs;

        std::list<GameObject*> m_gameObj;
        bool m_isSorted;
        uint32 m_transform;
        AuraList m_removedAuras;

        AuraList m_modAuras[TOTAL_AURAS];
        AuraList m_scAuras;                        // casted singlecast auras
        AuraList m_interruptableAuras;
        AuraList m_ccAuras;
        uint32 m_interruptMask;

        float m_auraModifiersGroup[UNIT_MOD_END][MODIFIER_TYPE_END];
        float m_weaponDamage[MAX_ATTACK][2];
        bool m_canModifyStats;
        //std::list< spellEffectPair > AuraSpells[TOTAL_AURAS];  // @todo use this if ok for mem

        float m_speed_rate[MAX_MOVE_TYPE];

        uint32 m_unitTypeMask;
        CharmInfo* m_charmInfo;
        SharedVisionList m_sharedVision;

        virtual SpellSchoolMask GetMeleeDamageSchoolMask() const;

        MotionMaster i_motionMaster;

        uint64 miniPetGuid;

        uint32 m_reactiveTimer[MAX_REACTIVE];

        ThreatManager m_ThreatManager;

        void DisableSpline();

        bool IsAlwaysVisibleFor(WorldObject const* seer) const override;
        bool IsAlwaysDetectableFor(WorldObject const* seer) const override;

        virtual void SetBaseWalkSpeed(float speed) { m_baseSpeedWalk = speed; }
        virtual void SetBaseRunSpeed(float speed) { m_baseSpeedRun = speed; }
        
        // base speeds set by model/template
        float m_baseSpeedWalk;
        float m_baseSpeedRun;
    private:
        bool IsTriggeredAtSpellProcEvent(Unit* victim, Aura* aura, SpellEntry const* procSpell, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, bool isVictim, bool active, SpellProcEventEntry const*& spellProcEvent);
        bool HandleDummyAuraProc(  Unit* victim, uint32 damage, Aura* triggredByAura, SpellEntry const* procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        bool HandleHasteAuraProc(  Unit* victim, uint32 damage, Aura* triggredByAura, SpellEntry const* procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        bool HandleProcTriggerSpell(Unit* victim, uint32 damage, Aura* triggredByAura, SpellEntry const* procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        bool HandleOverrideClassScriptAuraProc(Unit* victim, Aura* triggredByAura, SpellEntry const* procSpell, uint32 cooldown);
        bool HandleMendingAuraProc(Aura* triggeredByAura);

        uint32 m_state;                                     // Even derived shouldn't modify
        uint32 m_lastManaUse;                               // msecs

        Spell* m_currentSpells[CURRENT_MAX_SPELL];

        TimeTrackerSmall m_movesplineTimer;

        Diminishing m_Diminishing;
        // Manage all Units threatening us
        //        ThreatManager m_ThreatManager;
        // Manage all Units that are threatened by us
        HostileRefManager m_HostileRefManager;

        FollowerRefManager m_FollowingRefManager;

        ComboPointHolderSet m_ComboPointHolders;

        uint32 m_reducedThreatPercent;
        uint64 m_misdirectionTargetGUID;

        bool m_duringRemoveFromWorld; // lock made to not add stuff after begining removing from world

        uint32 m_procDeep;

        time_t _lastDamagedTime; // Part of Evade mechanics

        void UpdateSplineMovement(uint32 t_diff);
};

namespace Oregon
{
template<class T>
void RandomResizeList(std::list<T>& _list, uint32 _size)
{
    while (_list.size() > _size)
    {
        typename std::list<T>::iterator itr = _list.begin();
        advance(itr, urand(0, _list.size() - 1));
        _list.erase(itr);
    }
}
}

#endif

