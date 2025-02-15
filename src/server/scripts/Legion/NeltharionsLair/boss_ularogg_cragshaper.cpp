/*
    Dungeon : Neltharions Lair 100-110
    Encounter: Ularogg Cragshaper
    Normal: 100%, Heroic: 100%, Mythic: 100%
*/

#include "neltharions_lair.h"

enum Says
{
    SAY_AGGRO           = 0,
    SAY_STANCE_EMOTE    = 1,
    SAY_DEATH           = 2,
};

enum Spells
{
    SPELL_SUNDER                    = 198496,
    SPELL_SUNDER_CALL               = 198823, //Conversation
    SPELL_STRIKE_MOUNTAIN           = 198428,
    SPELL_STRIKE_MOUNTAIN_2         = 216290,
    SPELL_STRIKE_MOUNTAIN_AT        = 216292,
    SPELL_MOUNTAIN_CALL             = 198825, //Conversation
    SPELL_BELLOW_DEEPS_1            = 193375,
    SPELL_BELLOW_DEEPS_2            = 193376,
    SPELL_BELLOW_DEEPS_CALL         = 198824,
    //SPELL_CALL                    = 198826, //Conversation
    SPELL_STANCE_MOUNTAIN_JUMP      = 198509,
    SPELL_STANCE_MOUNTAIN_SUM_1     = 198564,
    SPELL_STANCE_MOUNTAIN_SUM_2     = 198565,
    SPELL_STANCE_MOUNTAIN_SUM_3     = 216249, //Heroic+
    SPELL_STANCE_MOUNTAIN_SUM_4     = 216250, //Heroic+
    SPELL_STANCE_MOUNTAIN_MORPH     = 198510,
    SPELL_STANCE_MOUNTAIN_TICK      = 198617,
    SPELL_STANCE_MOUNTAIN_FILTER    = 198619,
    SPELL_STANCE_MOUNTAIN_TELEPORT  = 198630,
    SPELL_FALLING_DEBRIS_ULAROGG    = 198719, //boss
    SPELL_STANCE_MOUNTAIN_END       = 198631,

    SPELL_FALLING_DEBRIS            = 193267, //npc 98081
    SPELL_FALLING_DEBRIS_2          = 198717, //npc 100818
    SPELL_STANCE_MOUNTAIN_MOVE      = 198616,
};

enum eEvents
{
    EVENT_SUNDER                = 1,
    EVENT_STRIKE_MOUNTAIN       = 2,
    EVENT_BELLOW_DEEPS_1        = 3,
    EVENT_BELLOW_DEEPS_2        = 4,
    EVENT_STANCE_MOUNTAIN_1     = 5,
    EVENT_STANCE_MOUNTAIN_2     = 6,
    EVENT_STANCE_MOUNTAIN_3     = 7,
    EVENT_STANCE_MOUNTAIN_4     = 8
};

//91004
struct boss_ularogg_cragshaper : public BossAI
{
    boss_ularogg_cragshaper(Creature* creature) : BossAI(creature, DATA_ULAROGG) {}

    std::map<uint32, std::list<ObjectGuid>> listGuid;
    ObjectGuid stanceGUID;
    bool intro = true;

    void Reset() override
    {
        _Reset();
        me->SetReactState(intro ? REACT_PASSIVE : REACT_AGGRESSIVE);
        me->RemoveAurasDueToSpell(SPELL_STANCE_MOUNTAIN_MORPH);
        me->RemoveAurasDueToSpell(SPELL_STANCE_MOUNTAIN_TICK);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(SAY_AGGRO); //Pay attention, Navarogg. I want you to see your heroes die.
        _EnterCombat();
        DefaultEvent(true);

        if (intro)
        {
            intro = false;
            me->SetHomePosition(2838.71f, 1668.18f, -40.64f, 3.75f);
            me->GetMotionMaster()->MoveJump(2838.71f, 1668.18f, -40.64f, 30.0f, 10.0f);
            me->SetReactState(REACT_AGGRESSIVE, 3000);
        }
    }

    void DefaultEvent(bool enterCombat)
    {
        events.RescheduleEvent(EVENT_SUNDER, 8000);
        events.RescheduleEvent(EVENT_STRIKE_MOUNTAIN, 16000);
        events.RescheduleEvent(EVENT_BELLOW_DEEPS_1, 20000);
        events.RescheduleEvent(EVENT_STANCE_MOUNTAIN_1, enterCombat ? 50000 : 120000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_DEATH);
        _JustDied();
    }

    bool GetObjectData(ObjectGuid const& guid, uint32 type) override
    {
        bool find = false;

        for (auto targetGuid : listGuid[type])
        {
            if (targetGuid == guid)
                find = true;
        }

        if (!find)
            listGuid[type].push_back(guid);

        return find;
    }

    void SpellFinishCast(const SpellInfo* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_STANCE_MOUNTAIN_SUM_1:
                DoCast(SPELL_STANCE_MOUNTAIN_SUM_2);
                break;
            case SPELL_STANCE_MOUNTAIN_SUM_2:
                if (GetDifficultyID() == DIFFICULTY_LFR || GetDifficultyID() == DIFFICULTY_NORMAL)
                    events.RescheduleEvent(EVENT_STANCE_MOUNTAIN_2, 2000);
                else
                    DoCast(SPELL_STANCE_MOUNTAIN_SUM_3);
                break;
            case SPELL_STANCE_MOUNTAIN_SUM_3:
                DoCast(SPELL_STANCE_MOUNTAIN_SUM_4);
                break;
            case SPELL_STANCE_MOUNTAIN_SUM_4:
                events.RescheduleEvent(EVENT_STANCE_MOUNTAIN_2, 2000);
                break;
        }
    }

    void SpellHitTarget(Unit* target, const SpellInfo* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_STANCE_MOUNTAIN_FILTER:
            {
                Position pos;
                me->GetRandomNearPosition(pos, 30.0f);
                target->CastSpell(pos, SPELL_STANCE_MOUNTAIN_MOVE, true);
                break;
            }
            case SPELL_STRIKE_MOUNTAIN_2:
            {
                Position pos;
                float angle = M_PI;
                for (uint8 i = 0; i < 4; ++i)
                {
                    target->GetNearPosition(pos, 5.0f, angle);
                    target->CastSpell(pos, SPELL_STRIKE_MOUNTAIN_AT, true);
                    angle += M_PI / 2.0f;
                }
                break;
            }
        }
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        if (summon->GetGUID() == stanceGUID)
        {
            me->CastSpell(summon->GetPosition(), SPELL_STANCE_MOUNTAIN_TELEPORT, true);
            me->RemoveAurasDueToSpell(SPELL_STANCE_MOUNTAIN_MORPH);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
            me->SetReactState(REACT_AGGRESSIVE);
            DoCast(me, SPELL_STANCE_MOUNTAIN_END, true);
            DefaultEvent(false);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (CheckHomeDistToEvade(diff, 40.0f))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SUNDER:
                    if (me->getVictim())
                    {
                        DoCast(me, SPELL_SUNDER_CALL, true);
                        DoCastVictim(SPELL_SUNDER);
                    }
                    events.RescheduleEvent(EVENT_SUNDER, 10000);
                    break;
                case EVENT_STRIKE_MOUNTAIN:
                    listGuid.clear();
                    DoCast(SPELL_STRIKE_MOUNTAIN);        
                    events.RescheduleEvent(EVENT_STRIKE_MOUNTAIN, 16000);
                    break;
                case EVENT_BELLOW_DEEPS_1:
                    DoCast(SPELL_BELLOW_DEEPS_1);
                    events.RescheduleEvent(EVENT_BELLOW_DEEPS_1, 32000);
                    events.RescheduleEvent(EVENT_BELLOW_DEEPS_2, 3000);
                    break;
                case EVENT_BELLOW_DEEPS_2:
                    DoCast(me, SPELL_BELLOW_DEEPS_CALL, true);
                    DoCast(SPELL_BELLOW_DEEPS_2);
                    break;
                case EVENT_STANCE_MOUNTAIN_1:
                    events.Reset();
                    me->StopAttack();
                    me->GetMotionMaster()->Clear();
                    me->CastSpell(me, SPELL_STANCE_MOUNTAIN_JUMP, TriggerCastFlags(TRIGGERED_IGNORE_POWER_AND_REAGENT_COST));
                    Talk(SAY_STANCE_EMOTE);
                    break;
                case EVENT_STANCE_MOUNTAIN_2:
                    me->RemoveAurasAllDots();
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
                    DoCast(SPELL_STANCE_MOUNTAIN_MORPH);
                    if (auto summon = me->SummonCreature(NPC_BELLOWING_IDOL_2, me->GetPosition()))
                        stanceGUID = summon->GetGUID();
                    events.RescheduleEvent(EVENT_STANCE_MOUNTAIN_3, 2000);
                    break;
                case EVENT_STANCE_MOUNTAIN_3:
                    me->AddAura(SPELL_STANCE_MOUNTAIN_TICK, me);
                    events.RescheduleEvent(EVENT_STANCE_MOUNTAIN_4, 10000);
                    break;
                case EVENT_STANCE_MOUNTAIN_4:
                {
                    EntryCheckPredicate pred(NPC_BELLOWING_IDOL_2);
                    summons.DoAction(ACTION_1, pred);
                    me->AddAura(SPELL_FALLING_DEBRIS_ULAROGG, me);
                    break;
                }
            }
        }
        DoMeleeAttackIfReady();
    }
};

//98081, 100818
struct npc_ularogg_bellowing_idols : public ScriptedAI
{
    npc_ularogg_bellowing_idols(Creature* creature) : ScriptedAI(creature) 
    {
        me->SetReactState(REACT_PASSIVE);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISTRACT, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
    }

    void Reset() override {}

    void IsSummonedBy(Unit* summoner) override
    {
        if (me->GetEntry() == NPC_BELLOWING_IDOL)
            DoCast(me, SPELL_FALLING_DEBRIS_2, true);
        else
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
            DoCast(me, 198569, true); //Visual Spawn
        }
    }

    void DoAction(int32 const action) override
    {
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
        DoCast(me, SPELL_FALLING_DEBRIS_2, true);
    }

    void UpdateAI(uint32 diff) override {}
};

//102228
struct npc_stonedark_slave : public ScriptedAI
{
    npc_stonedark_slave(Creature* creature) : ScriptedAI(creature) {}

    bool _endBarrel = false;
    bool _intro = false;

    void MoveInLineOfSight(Unit* who) override
    {  
        if (!who->IsPlayer())
            return;

        if (!_endBarrel && me->IsWithinDistInMap(who, 90.0f))
        {
            // who->CastSpell(who, 209531, true);  пока офф до 7.0.3, крашит
            _endBarrel = true;
        }

        if (!_intro && me->IsWithinDistInMap(who, 40.0f))
        {
            who->CastSpell(who, 209536, true);
            _intro = true;
        }
    }
};

//92473
struct npc_ularogg_empty_barrel : public ScriptedAI
{
    npc_ularogg_empty_barrel(Creature* creature) : ScriptedAI(creature) 
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void Reset() override {}

    void OnSpellClick(Unit* clicker) override
    {
        //Start WP player
        if (!clicker->HasAura(183213))
            clicker->CastSpell(clicker, 183213, true);
    }

    void UpdateAI(uint32 diff) override {}
};

//92610
struct npc_nl_understone_drummer : public ScriptedAI
{
    npc_nl_understone_drummer(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;
    bool drumsMove = true;
    bool drumsCast = true;
    Position pos;

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
    }

    void Reset() override
    {
        drumsMove = true;
        drumsCast = true;
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        if (Creature* drums = me->FindNearestCreature(92387, 40.0f, true))
        {
            if (drums->isAlive())
                events.RescheduleEvent(EVENT_1, 500);
            else
                events.Reset();
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;
        else
            me->SetReactState(REACT_AGGRESSIVE);

        if (drumsMove)
        {
            if (Creature* drums = me->FindNearestCreature(92387, 40.0f, true))
            {
                if (drums->isAlive())
                {
                    events.RescheduleEvent(EVENT_1, 1000);
                    drumsMove = false;
                }
            }
        }

        if (drumsCast)
        {
            if (Creature* drums = me->FindNearestCreature(92387, 3.0f, true))
            {
                events.RescheduleEvent(EVENT_2, 1000);
                drumsCast = false;
            }
        }

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_1:
                {
                    if (Creature* drums = me->FindNearestCreature(92387, 40.0f, true))
                    {
                        if (drums->isAlive())
                        {
                            pos = me->FindNearestCreature(92387, 40.0f, true)->GetPosition();
                            pos.m_positionX -= 3.0f;
                            me->GetMotionMaster()->MovePoint(1, pos, false);
                        }
                    }
                    break;
                }
                case EVENT_2:
                {
                    me->SetOrientation(0.5f);
                    me->SetReactState(REACT_AGGRESSIVE, 24000);
                    me->StopAttack(true);
                    DoCast(183526);
                    break;
                }
            }
        }
        DoMeleeAttackIfReady();
    }
};

//183213
class spell_barrel_ride_plr_move : public AuraScript
{
    PrepareAuraScript(spell_barrel_ride_plr_move);

    void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (Player* player = GetTarget()->ToPlayer())
        {
            player->GetMotionMaster()->MoveIdle();
            player->GetMotionMaster()->MovePath(9100400, false);
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_barrel_ride_plr_move::OnApply, EFFECT_2, SPELL_AURA_MOD_NO_ACTIONS, AURA_EFFECT_HANDLE_REAL);
    }
};

//198719
class spell_ularogg_falling_debris : public AuraScript
{
    PrepareAuraScript(spell_ularogg_falling_debris);

    uint8 tickTrigger = 6;
    uint8 tickSwitch = 0;

    void OnTick(AuraEffect const* aurEff)
    {
        if (tickTrigger > 0)
        {
            if (aurEff->GetTickNumber() % tickTrigger)
                PreventDefaultAction();
            else if (++tickSwitch == 6)
            {
                tickSwitch = 0;
                tickTrigger -= 2;
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_ularogg_falling_debris::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

//183088
class spell_nl_avalanche : public SpellScript
{
    PrepareSpellScript(spell_nl_avalanche);
    
    void HandleScript(SpellEffIndex effIndex)
    {
        if (!GetCaster())
            return;

        std::list<Player*> playerList;
        GetPlayerListInGrid(playerList, GetCaster(), 40);
        Trinity::Containers::RandomResizeList(playerList, 2);
        for (auto player : playerList)
            GetCaster()->CastSpell(player, 183095, true);                 
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_nl_avalanche::HandleScript, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

//198475
class spell_ularogg_strike_mountain : public SpellScript
{
    PrepareSpellScript(spell_ularogg_strike_mountain);

    void HandleScript(SpellEffIndex effectIndex)
    {
        if (!GetCaster() || !GetHitUnit())
            return;

        if (auto instance = GetCaster()->GetInstanceScript())
            if (auto ularogg = instance->instance->GetCreature(instance->GetGuidData(NPC_ULAROGG_CRAGSHAPER)))
                if (ularogg->AI()->GetObjectData(GetHitUnit()->GetGUID(), effectIndex))
                {
                    if (effectIndex == EFFECT_0)
                        SetHitDamage(0);
                    else
                        PreventHitDefaultEffect(effectIndex);
                }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_ularogg_strike_mountain::HandleScript, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        OnEffectHitTarget += SpellEffectFn(spell_ularogg_strike_mountain::HandleScript, EFFECT_1, SPELL_EFFECT_KNOCK_BACK);
    }
};

void AddSC_boss_ularogg_cragshaper()
{
    RegisterCreatureAI(boss_ularogg_cragshaper);
    RegisterCreatureAI(npc_ularogg_bellowing_idols);
    RegisterCreatureAI(npc_stonedark_slave);
    RegisterCreatureAI(npc_ularogg_empty_barrel);
    RegisterCreatureAI(npc_nl_understone_drummer);
    RegisterAuraScript(spell_barrel_ride_plr_move);
    RegisterAuraScript(spell_ularogg_falling_debris);
    RegisterSpellScript(spell_nl_avalanche);
    RegisterSpellScript(spell_ularogg_strike_mountain);
}