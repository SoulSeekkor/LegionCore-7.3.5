/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
 */

/* ScriptData
SDName: Dark_Portal
SD%Complete: 30
SDComment: Misc NPC's and mobs for instance. Most here far from complete.
SDCategory: Caverns of Time, The Dark Portal
EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "dark_portal.h"

enum Texts
{
    SAY_ENTER                   = 0,     
    SAY_INTRO                   = 1,
    SAY_WEAK75                  = 2,
    SAY_WEAK50                  = 3,
    SAY_WEAK25                  = 4,
    SAY_DEATH                   = 5,
    SAY_WIN                     = 6,
    SAY_ORCS_ENTER              = 7,
    SAY_ORCS_ANSWER             = 8,
};

enum Spells
{
    SPELL_CHANNEL               = 31556,
    SPELL_PORTAL_RUNE           = 32570,
    SPELL_BLACK_CRYSTAL         = 32563,
    SPELL_PORTAL_CRYSTAL        = 32564,
    SPELL_BANISH_PURPLE         = 32566,
    SPELL_BANISH_GREEN          = 32567,
    SPELL_CORRUPT               = 31326,
    SPELL_CORRUPT_AEONUS        = 37853,
    //npc
    C_COUNCIL_ENFORCER          = 17023,
};

Position orcpos[5] =
{
    { -2088.80f, 7132.74f, 34.55f, 6.17f },
    { -2089.09f, 7129.99f, 34.55f, 6.17f },
    { -2089.40f, 7126.99f, 34.55f, 6.17f },
    { -2089.72f, 7123.93f, 34.55f, 6.17f },
    { -2090.06f, 7120.70f, 34.55f, 6.17f },
};

class npc_medivh_bm : public CreatureScript
{
public:
    npc_medivh_bm() : CreatureScript("npc_medivh_bm") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_medivh_bmAI (creature);
    }

    struct npc_medivh_bmAI : public ScriptedAI
    {
        npc_medivh_bmAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        uint32 SpellCorrupt_Timer;
        uint32 Check_Timer;

        bool Life75;
        bool Life50;
        bool Life25;

        void Reset() override
        {
            SpellCorrupt_Timer = 0;

            if (!instance)
                return;

            if (instance->GetData(TYPE_MEDIVH) == IN_PROGRESS)
                DoCast(me, SPELL_CHANNEL, true);
            else if (me->HasAura(SPELL_CHANNEL))
                me->RemoveAura(SPELL_CHANNEL);

            DoCast(me, SPELL_PORTAL_RUNE, true);
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (!instance)
                return;

            if (who->GetTypeId() == TYPEID_PLAYER && me->IsWithinDistInMap(who, 10.0f))
            {
                if (instance->GetData(TYPE_MEDIVH) == IN_PROGRESS || instance->GetData(TYPE_MEDIVH) == DONE)
                    return;

                Talk(SAY_INTRO);
                instance->SetData(TYPE_MEDIVH, IN_PROGRESS);
                DoCast(me, SPELL_CHANNEL, false);
                Check_Timer = 5000;
            }
            else if (who->GetTypeId() == TYPEID_UNIT && me->IsWithinDistInMap(who, 15.0f))
            {
                if (instance->GetData(TYPE_MEDIVH) != IN_PROGRESS)
                    return;

                uint32 entry = who->GetEntry();
                if (entry == C_ASSAS || entry == C_WHELP || entry == C_CHRON || entry == C_EXECU || entry == C_VANQU)
                {
                    who->StopMoving();
                    who->CastSpell(me, SPELL_CORRUPT, false);
                }
                else if (entry == C_AEONUS)
                {
                    who->StopMoving();
                    who->CastSpell(me, SPELL_CORRUPT_AEONUS, false);
                }
            }
        }

        void SpellHit(Unit* /*caster*/, const SpellInfo* spell) override
        {
            if (SpellCorrupt_Timer)
                return;

            if (spell->Id == SPELL_CORRUPT_AEONUS)
                SpellCorrupt_Timer = 1000;

            if (spell->Id == SPELL_CORRUPT)
                SpellCorrupt_Timer = 3000;
        }

        void JustDied(Unit* killer) override
        {
            if (killer->GetEntry() == me->GetEntry())
                return;

            Talk(SAY_DEATH);
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
            case 1:
                me->AddDelayedEvent(10000, [this] {
                    for (uint8 n = 0; n < 5; n++)
                        if (Creature* orc = me->SummonCreature(C_COUNCIL_ENFORCER, orcpos[n]))
                        {
                            float x, y, z;
                            orc->SetReactState(REACT_PASSIVE);
                            orc->GetClosePoint(x, y, z, orc->GetObjectSize() / 3, static_cast<float>(37));
                            orc->GetMotionMaster()->MovePoint(0, x, y, z);
                        }
                });
                me->AddDelayedEvent(12000, [this] {
                    for (uint8 n = 0; n < 5; n++)
                        if (Creature* orc = me->SummonCreature(C_COUNCIL_ENFORCER, orcpos[n]))
                        {
                            float x, y, z;
                            orc->GetClosePoint(x, y, z, orc->GetObjectSize() / 3, static_cast<float>(29));
                            orc->GetMotionMaster()->MovePoint(0, x, y, z);
                        }
                });
                me->AddDelayedEvent(13000, [this] {
                    for (uint8 n = 0; n < 5; n++)
                        if (Creature* orc = me->SummonCreature(C_COUNCIL_ENFORCER, orcpos[n]))
                        {
                            float x, y, z;
                            orc->GetClosePoint(x, y, z, orc->GetObjectSize() / 3, static_cast<float>(17));
                            orc->GetMotionMaster()->MovePoint(0, x, y, z);
                        }
                });
                me->AddDelayedEvent(13000, [this] {
                    for (uint8 n = 0; n < 5; n++)
                        if (Creature* orc = me->SummonCreature(C_COUNCIL_ENFORCER, orcpos[n]))
                        {
                            float x, y, z;
                            orc->GetClosePoint(x, y, z, orc->GetObjectSize() / 3, static_cast<float>(3));
                            orc->GetMotionMaster()->MovePoint(0, x, y, z);
                        }
                });
                me->AddDelayedEvent(18000, [this] {
                    Talk(SAY_ORCS_ENTER);
                });
                me->AddDelayedEvent(23000, [this] {
                    if (Creature* orc = me->FindNearestCreature(C_COUNCIL_ENFORCER, 29.0f, true))
                        orc->AI()->Talk(1);
                    DoAction(2);
                });
                break;
            case 2:
                me->AddDelayedEvent(5000, [this] {
                    std::list<Creature*> creaList;
                    GetCreatureListWithEntryInGrid(creaList, me, C_COUNCIL_ENFORCER, 100.0f);
                    for (std::list<Creature*>::iterator itr = creaList.begin(); itr != creaList.end(); ++itr)
                        if (Creature* orcs = *itr)
                        {
                            float x = -2092.70f;
                            float y = 7126.88f;
                            float z = 34.88f;
                            orcs->GetMotionMaster()->MovePoint(0, x + frand(1,2), y + frand(1,6), z);

                            orcs->DespawnOrUnsummon(5000);
                        }
                });
                
                break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!instance)
                return;

            if (SpellCorrupt_Timer)
            {
                if (SpellCorrupt_Timer <= diff)
                {
                    instance->SetData(TYPE_MEDIVH, SPECIAL);

                    if (me->HasAura(SPELL_CORRUPT_AEONUS))
                        SpellCorrupt_Timer = 1000;
                    else if (me->HasAura(SPELL_CORRUPT))
                        SpellCorrupt_Timer = 3000;
                    else
                        SpellCorrupt_Timer = 0;
                } else SpellCorrupt_Timer -= diff;
            }

            if (Check_Timer)
            {
                if (Check_Timer <= diff)
                {
                    uint32 pct = instance->GetData(DATA_SHIELD);

                    Check_Timer = 5000;

                    if (Life25 && pct <= 25)
                    {
                        Talk(SAY_WEAK25);
                        Life25 = false;
                    }
                    else if (Life50 && pct <= 50)
                    {
                        Talk(SAY_WEAK50);
                        Life50 = false;
                    }
                    else if (Life75 && pct <= 75)
                    {
                        Talk(SAY_WEAK75);
                        Life75 = false;
                    }

                    //if we reach this it means event was running but at some point reset.
                    if (instance->GetData(TYPE_MEDIVH) == NOT_STARTED)
                    {
                        me->DealDamage(me, me->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                        me->RemoveCorpse();
                        me->Respawn();
                        return;
                    }

                    if (instance->GetData(TYPE_RIFT) == DONE)
                    {
                        Talk(SAY_WIN);
                        Check_Timer = 0;

                        if (me->HasAura(SPELL_CHANNEL))
                            me->RemoveAura(SPELL_CHANNEL);

                        instance->SetData(TYPE_MEDIVH, DONE);
                        DoAction(1);
                    }
                } else Check_Timer -= diff;
            }
        }
    };

};

struct Wave
{
    uint32 PortalMob[4];
};

static Wave PortalWaves[]=
{
    { {C_ASSAS, C_WHELP, C_CHRON, 0} },
    { {C_EXECU, C_CHRON, C_WHELP, C_ASSAS} },
    { {C_EXECU, C_VANQU, C_CHRON, C_ASSAS} }
};

class npc_time_rift : public CreatureScript
{
public:
    npc_time_rift() : CreatureScript("npc_time_rift") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_time_riftAI (creature);
    }

    struct npc_time_riftAI : public ScriptedAI
    {
        npc_time_riftAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        uint32 TimeRiftWave_Timer;
        uint8 mRiftWaveCount;
        uint8 mPortalCount;
        uint8 mWaveId;

        void Reset()
        {

            TimeRiftWave_Timer = 15000;
            mRiftWaveCount = 0;

            if (!instance)
                return;

            mPortalCount = instance->GetData(DATA_PORTAL_COUNT);

            if (mPortalCount < 6)
                mWaveId = 0;
            else if (mPortalCount > 12)
                mWaveId = 2;
            else mWaveId = 1;

        }
        void EnterCombat(Unit* /*who*/) {}

        void DoSummonAtRift(uint32 creature_entry)
        {
            if (!creature_entry)
                return;

            if (instance && instance->GetData(TYPE_MEDIVH) != IN_PROGRESS)
            {
                me->InterruptNonMeleeSpells(true);
                me->RemoveAllAuras();
                return;
            }

            Position pos;
            me->GetRandomNearPosition(pos, 10.0f);

            //normalize Z-level if we can, if rift is not at ground level.
            pos.m_positionZ = std::max(me->GetMap()->GetHeight(pos.m_positionX, pos.m_positionY, MAX_HEIGHT), me->GetMap()->GetWaterLevel(pos.m_positionX, pos.m_positionY));

            if (Unit* Summon = DoSummon(creature_entry, pos, 30000, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT))
                if (Unit* temp = Unit::GetUnit(*me, instance ? instance->GetGuidData(DATA_MEDIVH) : ObjectGuid::Empty))
                    Summon->AddThreat(temp, 0.0f);
        }

        void DoSelectSummon()
        {
            if ((mRiftWaveCount > 2 && mWaveId < 1) || mRiftWaveCount > 3)
                mRiftWaveCount = 0;

            uint32 entry = 0;

            entry = PortalWaves[mWaveId].PortalMob[mRiftWaveCount];
            TC_LOG_DEBUG(LOG_FILTER_TSCR, "npc_time_rift: summoning wave Creature (Wave %u, Entry %u).", mRiftWaveCount, entry);

            ++mRiftWaveCount;

            if (entry == C_WHELP)
            {
                for (uint8 i = 0; i < 3; ++i)
                    DoSummonAtRift(entry);
            } else DoSummonAtRift(entry);
        }

        void UpdateAI(uint32 diff)
        {
            if (!instance)
                return;

            if (TimeRiftWave_Timer <= diff)
            {
                DoSelectSummon();
                TimeRiftWave_Timer = 15000;
            } else TimeRiftWave_Timer -= diff;

            if (me->IsNonMeleeSpellCast(false))
                return;

            TC_LOG_DEBUG(LOG_FILTER_TSCR, "npc_time_rift: not casting anylonger, i need to die.");
            me->setDeathState(JUST_DIED);

            if (instance->GetData(TYPE_RIFT) == IN_PROGRESS)
                instance->SetData(TYPE_RIFT, SPECIAL);
        }
    };

};

#define SAY_SAAT_WELCOME        -1269019

#define GOSSIP_ITEM_OBTAIN      "[PH] Obtain Chrono-Beacon"
#define SPELL_CHRONO_BEACON     34975
#define ITEM_CHRONO_BEACON      24289

class npc_saat : public CreatureScript
{
public:
    npc_saat() : CreatureScript("npc_saat") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF+1)
        {
            player->CLOSE_GOSSIP_MENU();
            creature->CastSpell(player, SPELL_CHRONO_BEACON, false);
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_OPENING_PORTAL) == QUEST_STATUS_INCOMPLETE && !player->HasItemCount(ITEM_CHRONO_BEACON))
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_OBTAIN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SEND_GOSSIP_MENU(10000, creature->GetGUID());
            return true;
        }
        else if (player->GetQuestRewardStatus(QUEST_OPENING_PORTAL) && !player->HasItemCount(ITEM_CHRONO_BEACON))
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_OBTAIN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SEND_GOSSIP_MENU(10001, creature->GetGUID());
            return true;
        }

        player->SEND_GOSSIP_MENU(10002, creature->GetGUID());
        return true;
    }

};

void AddSC_dark_portal()
{
    new npc_medivh_bm();
    new npc_time_rift();
    new npc_saat();
}
