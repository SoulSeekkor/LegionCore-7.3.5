/*
    Dungeon : Neltharions Lair 100-110
    Encounter: Rokmora
    Normal: 100%, Heroic: 100%, Mythic: 100%
*/

#include "neltharions_lair.h"

enum Says
{
    SAY_AGGRO           = 0,
    SAY_RAZOR           = 1,
    SAY_DEATH           = 2,
}; 

enum Spells
{
    SPELL_GAIN_ENERGY               = 193245,
    SPELL_BRITTLE                   = 187714,
    SPELL_INTRO_ULAROGG             = 209390, //Boss 01 Intro Ularogg Cast
    SPELL_INTRO_EMERGE              = 209394, //Boss 01 Intro Emerge

    SPELL_SHATTER_START_CALL_1      = 198122, //Conversation Shatter
    SPELL_SHATTER_END_CALL_1        = 198135, //Conversation Shatter
    SPELL_SHATTER_END_CALL_2        = 198136, //Conversation Shatter
    SPELL_SHATTER                   = 188114,
    SPELL_SHATTER_KILL              = 200247,
    SPELL_RAZOR_SHARDS_CALL         = 198125, //Conversation Razor Shards
    SPELL_RAZOR_SHARDS              = 188169,
    SPELL_RAZOR_SHARDS_VISUAL_1     = 188207,
    SPELL_RAZOR_SHARDS_VISUAL_2     = 197920,
    SPELL_RAZOR_SHARDS_FILTER       = 209718,

    //Heroic
    SPELL_CRYSTALLINE_GROUND        = 198024,
    SPELL_CRYSTALLINE_GROUND_DMG    = 198028,
    SPELL_RUPTURING_SKITTER         = 215929,

    SPELL_CHOKING_DUST_AT           = 192799,
};

enum eEvents
{
    EVENT_RAZOR_SHARDS              = 1,
    EVENT_CRYSTALLINE_GROUND        = 2,
    EVENT_DEAD_CONVERSATION         = 3,
};

//91003
struct boss_rokmora : public BossAI
{
    boss_rokmora(Creature* creature) : BossAI(creature, DATA_ROKMORA) 
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
    }

    bool introDone = false;

    void Reset() override
    {
        _Reset();

        DoCast(me, SPELL_BRITTLE, true);
        me->RemoveAurasDueToSpell(SPELL_GAIN_ENERGY);
        me->SetMaxPower(POWER_MANA, 100);
        me->SetPower(POWER_MANA, 0);        
    }

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(SAY_AGGRO); //Rok SMASH!
        _EnterCombat();
        DoCast(me, SPELL_GAIN_ENERGY, true);

        events.RescheduleEvent(EVENT_RAZOR_SHARDS, 30000);

        if (GetDifficultyID() != DIFFICULTY_LFR && GetDifficultyID() != DIFFICULTY_NORMAL)
            events.RescheduleEvent(EVENT_CRYSTALLINE_GROUND, 4000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_DEATH);
        _JustDied();
        events.RescheduleEvent(EVENT_DEAD_CONVERSATION, 3000);
    }

    void MoveInLineOfSight(Unit* who) override
    {  
        if (!who->IsPlayer())
            return;

        if (!introDone && me->IsWithinDistInMap(who, 40.0f))
        {
            introDone = true;
            me->SummonCreature(NPC_NAVARROGG_INTRO, 2917.32f, 1402.29f, -2.28f, 2.744620f, TEMPSUMMON_TIMED_DESPAWN, 22000);
            if (auto ularogg = me->SummonCreature(NPC_ULAROGG_INTRO, 2900.33f, 1410.06f, -2.32f, 4.05f, TEMPSUMMON_TIMED_DESPAWN, 22000))
                ularogg->CastSpell(me, SPELL_INTRO_ULAROGG, true);

            DoCast(me, 209374, true); //Convers
            DoCast(SPELL_INTRO_EMERGE);
        }
    }

    void SpellFinishCast(const SpellInfo* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_INTRO_EMERGE:
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
                break;
            case SPELL_RAZOR_SHARDS:
            {
                Position pos;
                for (uint8 i = 0; i < 20; ++i)
                {
                    me->GetNearPosition(pos, frand(15.0f, 30.0f), frand(-1.5f, 1.5f));
                    me->CastSpell(pos, SPELL_RAZOR_SHARDS_VISUAL_1, true);
                }
                for (uint8 i = 0; i < 7; ++i)
                    me->CastSpellDelay(me, SPELL_RAZOR_SHARDS_VISUAL_2, true, 50 * i);
                break;
            }
            case SPELL_SHATTER:
                me->CastSpell(me, SPELL_SHATTER_KILL, true);
                break;
        }
    }

    void DoAction(int32 const action) override
    {
        if (GetDifficultyID() != DIFFICULTY_LFR && GetDifficultyID() != DIFFICULTY_NORMAL)
            events.RescheduleEvent(EVENT_CRYSTALLINE_GROUND, 4000);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (CheckHomeDistToEvade(diff, 45.0f))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_RAZOR_SHARDS:
                    DoCast(me, SPELL_RAZOR_SHARDS_CALL, true);
                    if (auto target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    {
                        Position pos;
                        me->GetNearPosition(pos, 60.0f, me->GetRelativeAngle(target));
                        if (auto summon = me->SummonCreature(NPC_RAZOR_SHARDS_STALKER, pos, TEMPSUMMON_TIMED_DESPAWN, 10000))
                        {
                            summon->StopAttack();
                            me->CastSpell(summon, SPELL_RAZOR_SHARDS);
                        }
                    }
                    Talk(SAY_RAZOR);
                    events.RescheduleEvent(EVENT_RAZOR_SHARDS, 26000);
                    break;
                case EVENT_CRYSTALLINE_GROUND:
                    //Talk();
                    DoCast(SPELL_CRYSTALLINE_GROUND);
                    break;
                case EVENT_DEAD_CONVERSATION:
                    //DoCast(199392);
                    break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//97720
struct npc_rokmora_blightshard_skitter : public ScriptedAI
{
    npc_rokmora_blightshard_skitter(Creature* creature) : ScriptedAI(creature) {}

    void Reset() override {}

    void JustDied(Unit* killer) override
    {
        if (auto owner = me->GetAnyOwner())
            me->CastSpell(me, SPELL_CHOKING_DUST_AT, true, nullptr, nullptr, owner->GetGUID());
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

//91001
struct npc_nl_tarspitter_lurker : public ScriptedAI
{
    npc_nl_tarspitter_lurker(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void Reset() override
    {
        events.Reset();
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->CastSpell(me, 226385, true);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, urandms(6, 7));
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING | UNIT_STATE_STUNNED))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_1:
                {
                    me->StopAttack(true);
                    me->SetReactState(REACT_AGGRESSIVE, 10000);
                    DoCast(183433);
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(target, 183430, true);
                    events.RescheduleEvent(EVENT_2, 2000);
                    events.RescheduleEvent(EVENT_1, 25000);
                    break;
                }
                case EVENT_2:
                {
                    DoCast(183438);
                    events.RescheduleEvent(EVENT_3, 1500);
                    break;
                }
                case EVENT_3:
                {
                    me->StopAttack(true);
                    me->SetReactState(REACT_AGGRESSIVE, 5500);
                    me->CastSpell(me, 183465);
                    break;
                }
            }
        }
        DoMeleeAttackIfReady();
    }
};

//91000
struct npc_nl_vileshard_hulk : public ScriptedAI
{
    npc_nl_vileshard_hulk(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
    }

    void Reset() override
    {
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 7000);
        events.RescheduleEvent(EVENT_2, 9000);
        events.RescheduleEvent(EVENT_3, 15000);
    }

    void SpellFinishCast(const SpellInfo* spell) override
    {
        if ((spell->Id == 226296) || (spell->Id == 226304))
        {
            Position pos;
            for (uint8 i = 0; i < 20; ++i)
            {
                me->GetNearPosition(pos, frand(5.0f, 30.0f), frand(-0.5f, 0.5f));
                me->CastSpell(pos, 226305, true);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_1:
                    DoCastVictim(193505);
                    events.RescheduleEvent(EVENT_1, 16000);
                    break;
                case EVENT_2:
                    DoCast(226296);
                    events.RescheduleEvent(EVENT_2, 16000);
                    break;
                case EVENT_3:
                    DoCast(226304);
                    events.RescheduleEvent(EVENT_3, 16000);
                    break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//200247
class spell_rokmora_shatter_kill : public SpellScript
{
    PrepareSpellScript(spell_rokmora_shatter_kill);

    void HandleDamage(SpellEffIndex effIndex)
    {
        if (!GetCaster() || !GetHitUnit())
            return;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_RUPTURING_SKITTER))
        {
            float dmg = CalculatePct(spellInfo->GetEffect(EFFECT_0, GetCaster()->GetMap()->GetDifficultyID())->CalcValue(), GetHitUnit()->GetHealthPct());
            GetHitUnit()->CastCustomSpell(GetHitUnit(), spellInfo->Id, &dmg, nullptr, nullptr, true);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_rokmora_shatter_kill::HandleDamage, EFFECT_0, SPELL_EFFECT_INSTAKILL);
    }
};

//193245
class spell_rokmora_gain_energy : public AuraScript
{
    PrepareAuraScript(spell_rokmora_gain_energy);

    bool checkTalk = false;

    void OnTick(AuraEffect const* aurEff)
    {
        auto caster = GetCaster()->ToCreature();
        if (!caster)
            return;

        if (caster->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (caster->GetPower(POWER_MANA) == 0 && checkTalk)
        {
            checkTalk = false;
            uint8 rand = urand(0, 1);
            caster->CastSpell(caster, rand ? SPELL_SHATTER_END_CALL_1 : SPELL_SHATTER_END_CALL_2, true);
        }

        if (caster->GetPower(POWER_MANA) >= 100)
        {
            caster->CastSpell(caster, SPELL_SHATTER_START_CALL_1, true);
            caster->CastSpell(caster, SPELL_SHATTER);
            caster->AI()->DoAction(true);
            checkTalk = true;
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_rokmora_gain_energy::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_ENERGIZE);
    }
};

//215898
class spell_rokmora_crystalline_ground : public AuraScript
{
    PrepareAuraScript(spell_rokmora_crystalline_ground);

    void OnPeriodic(AuraEffect const* aurEff)
    {
        if (!GetTarget())
            return;

        if (GetTarget()->isMoving())
            GetTarget()->CastSpell(GetTarget(), SPELL_CRYSTALLINE_GROUND_DMG, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_rokmora_crystalline_ground::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

//183433
class spell_nl_submerge : public AuraScript
{
    PrepareAuraScript(spell_nl_submerge);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTarget())
            GetTarget()->SetVisible(false);      
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (!GetTarget())
            return;

        std::list<Player*> playerList;
        GetPlayerListInGrid(playerList, GetTarget(), 40);
        Trinity::Containers::RandomResizeList(playerList, 1);
        if (!playerList.empty())
        {
            GetTarget()->CastSpell(GetTarget(), 183438, true);
            GetTarget()->CastSpell(playerList.front(), 183430, true);
        }
        GetTarget()->SetVisible(true);          
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_nl_submerge::OnApply, EFFECT_0, SPELL_AURA_TRANSFORM, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_nl_submerge::OnRemove, EFFECT_0, SPELL_AURA_TRANSFORM, AURA_EFFECT_HANDLE_REAL);
    }
};

//209888
class spell_entrance_run_plr_move : public AuraScript
{
    PrepareAuraScript(spell_entrance_run_plr_move);

    void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (auto player = GetTarget()->ToPlayer())
        {
            player->GetMotionMaster()->MoveIdle();
            player->GetMotionMaster()->MovePath(20988800, false);
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_entrance_run_plr_move::OnApply, EFFECT_2, SPELL_AURA_MOD_NO_ACTIONS, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_boss_rokmora()
{
    RegisterCreatureAI(boss_rokmora);
    RegisterCreatureAI(npc_rokmora_blightshard_skitter);
    RegisterCreatureAI(npc_nl_tarspitter_lurker);
    RegisterCreatureAI(npc_nl_vileshard_hulk);
    RegisterSpellScript(spell_rokmora_shatter_kill);
    RegisterAuraScript(spell_rokmora_gain_energy);
    RegisterAuraScript(spell_rokmora_crystalline_ground);
    RegisterAuraScript(spell_nl_submerge);
    RegisterAuraScript(spell_entrance_run_plr_move);
}