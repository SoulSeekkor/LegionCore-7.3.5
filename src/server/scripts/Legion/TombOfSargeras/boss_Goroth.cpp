#include "AreaTriggerAI.h"
#include "tomb_of_sargeras.h"

enum Spells
{
    SPELL_GOROTH_ENERGIZE                       = 237333,
    SPELL_BURNING_ARMOR                         = 231363,
    SPELL_BURNING_ERUPTION                      = 231395,
    SPELL_BURNING_ERUPTION_DUMMY                = 233025,
    SPELL_CRASHING_COMET_FILTER                 = 232249, //Caster Boss
    SPELL_CRASHING_COMET_LFR_FILTER             = 244548, //Caster Boss. Only LFR
    SPELL_CRASHING_COMET_FILTER2                = 232254, //Caster Ember Stalker - 115892
    SPELL_CRASHING_COMET_LFR_FILTER2            = 244580, //Caster Ember Stalker - 115892. Only LFR
    SPELL_CRASHING_COMET_MISSILE                = 230339,
    SPELL_CRASHING_COMET_LFR_MISSILE            = 244581,
    SPELL_INFERNAL_SPIKE_FILTER                 = 233050,
    SPELL_INFERNAL_SPIKE_SUMMON                 = 233055,
    SPELL_INFERNAL_SPIKE_AT                     = 233019,
    SPELL_INFERNAL_SPIKE_PROTECTION             = 234475,
    SPELL_SHATTERING_STAR_FILTER                = 233269,
    SPELL_SHATTERING_STAR_MARK                  = 233272,
    SPELL_SHATTERING_STAR_AURA                  = 233289, //unk
    SPELL_SHATTERING_STAR_SPEED                 = 233290,
    SPELL_SHATTERING_STAR_HIT                   = 233274,
    SPELL_SHATTERING_STAR_AT                    = 233279,
    SPELL_SHATTERING_NOVA                       = 233283,
    SPELL_STAR_BURN                             = 236329,
    SPELL_INFERNAL_BURNING                      = 233062,
    SPELL_INFERNAL_DETONATION                   = 233900, //Mythic
    SPELL_RAIN_OF_BRIMSTONE_DUMMY               = 233285, //Mythic
    SPELL_RAIN_OF_BRIMSTONE_MISSILE             = 238587, //Mythic
    SPELL_RAIN_OF_BRIMSTONE_SUMMON              = 233266, //Mythic
    SPELL_RAIN_OF_BRIMSTONE_SPIKE_DESTROY       = 238659, //Mythic

    //Lava Stalker
    SPELL_FEL_PERIODIC_TRIGGER                  = 234386,
    SPELL_FEL_ERUPTION_TELEGRAPH                = 234366,
    SPELL_FEL_ERUPTION_DUMMY                    = 234368,
    SPELL_FEL_ERUPTION_MISSILE                  = 234387,
    SPELL_FEL_ERUPTION_AT                       = 234330,

    //Brimstone Infernal
    SPELL_FEL_FIRE                              = 241455,
};

enum eEvents
{
    EVENT_BURNING_ARMOR                         = 1,
    EVENT_CRASHING_COMET                        = 2,
    EVENT_INFERNAL_SPIKE                        = 3,
    EVENT_SHATTERING_STAR                       = 4,
    EVENT_RAIN_OF_BRIMSTONE                     = 5,
};

enum Misc
{
    SPELLVISUAL_INFERNAL_SPIKE_DESTROY          = 66119,
    DATA_SPIKES_COUNTER,
};

Position const centrPos = {6101.30f, -795.72f, 2971.72f};

//115844
struct boss_goroth : BossAI
{
    explicit boss_goroth(Creature* creature) : BossAI(creature, DATA_GOROTH) {}

    uint8 spiketouched = 0;
    uint8 felRand = 0;
    uint8 shatteringCounter = 0;
    uint8 rainBrimstoneCounter = 0;
    uint32 felEruptionTimer = 0;
    uint32 shatteringTimer = 0;
    uint32 rainBrimstoneTimer = 0;
    ObjectGuid shatteringTargetGUID;

    void Reset() override
    {
        _Reset();

        spiketouched = 0;
        shatteringCounter = 0;
        rainBrimstoneCounter = 0;
        shatteringTimer = 60000;
        rainBrimstoneTimer = 60000;
        me->RemoveAurasDueToSpell(SPELL_GOROTH_ENERGIZE);
        me->SetMaxPower(POWER_ENERGY, 100);
        me->SetPower(POWER_ENERGY, 0);
        me->SetReactState(REACT_DEFENSIVE);
        me->SummonCreature(NPC_EMBER_STALKER, 6194.41f, -842.93f, 3042.72f, 0.0f);

        if (IsHeroicPlusRaid())
        {
            felRand = 0;
            felEruptionTimer = 30000;
        }
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();

        DoCast(me, SPELL_GOROTH_ENERGIZE, true);
        events.RescheduleEvent(EVENT_BURNING_ARMOR, 10000);
        events.RescheduleEvent(EVENT_CRASHING_COMET, 8000);
        events.RescheduleEvent(EVENT_INFERNAL_SPIKE, 4000);
        events.RescheduleEvent(EVENT_SHATTERING_STAR, IsMythicRaid() ? 34000 : 24000);

        if (IsMythicRaid())
            events.RescheduleEvent(EVENT_RAIN_OF_BRIMSTONE, 12000);

        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_INFERNAL_SPIKE_PROTECTION);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(12);

        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_INFERNAL_SPIKE_PROTECTION);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_STAR_BURN);
    }

    void KilledUnit(Unit* who) override
    {
        if (!who->IsPlayer())
            return;

        Talk({ 7, 9, 10 });
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
            case ACTION_1:
                if (auto player = Player::GetPlayer(*me, shatteringTargetGUID))
                    DoCast(player, SPELL_STAR_BURN, true);
                break;
            case ACTION_2:
                ++spiketouched;
                break;
        }
    }

    uint32 GetData(uint32 type) const override
    {
        if (type == DATA_SPIKES_COUNTER)
            return spiketouched;

        return 0;
    }

    void OnRemoveAuraTarget(Unit* target, uint32 spellId, AuraRemoveMode mode) override
    {
        if (!me->isInCombat() || !target || mode != AURA_REMOVE_BY_EXPIRE)
            return;

        switch (spellId)
        {
            case SPELL_BURNING_ARMOR:
                target->CastSpell(target, SPELL_BURNING_ERUPTION, true);
                me->CastSpell(target, SPELL_BURNING_ERUPTION_DUMMY, true);
                break;
        }
    }

    void SpellFinishCast(const SpellInfo* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_INFERNAL_BURNING:
            {
                me->SetPower(POWER_ENERGY, 0);
                EntryCheckPredicate pred(NPC_INFERNAL_SPIKE);
                summons.DoAction(ACTION_1, pred);
                break;
            }
            case SPELL_RAIN_OF_BRIMSTONE_DUMMY:
            {
                Position pos;
                std::list<Position> randPosList;
                bool badPos = false;
                uint16 traiCount = 0;
                while (randPosList.size() < 3)
                {
                    badPos = false;
                    centrPos.SimplePosXYRelocationByAngle(pos, frand(25.0f, 35.0f), frand(0.0f, 6.28f));
                    ++traiCount;

                    for (auto _pos : randPosList)
                    {
                        if (pos.GetExactDist(&_pos) <= 10.0f)
                        {
                            badPos = true;
                            break;
                        }
                    }
                    if (!badPos || traiCount > 15)
                        randPosList.push_back(pos);
                }
                for (auto _pos : randPosList)
                    me->CastSpell(_pos, SPELL_RAIN_OF_BRIMSTONE_MISSILE, true);
                break;
            }
        }
    }

    void SpellHit(Unit* target, SpellInfo const* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_SHATTERING_STAR_HIT:
                if (me->isInCombat())
                    DoCast(target, SPELL_SHATTERING_STAR_AT, true);
                break;
        }
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_INFERNAL_SPIKE_FILTER:
                DoCast(target, SPELL_INFERNAL_SPIKE_SUMMON, true);
                break;
            case SPELL_SHATTERING_STAR_FILTER:
                shatteringTargetGUID = target->GetGUID();
                DoCast(target, SPELL_SHATTERING_STAR_MARK, true);
                DoCast(target, SPELL_SHATTERING_STAR_SPEED, true);
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (CheckHomeDistToEvade(diff, 62.0f, 6100.85f, -795.70f, 2971.72f))
            return;

        if (felEruptionTimer)
        {
            if (felEruptionTimer <= diff)
            {
                felEruptionTimer = 30000;
                uint8 felRandOld = felRand;

                while (felRandOld == felRand)
                {
                    felRand = urand(2, 5);
                }
                me->SummonCreatureGroupDespawn(felRandOld);
                me->SummonCreatureGroup(felRand);
            }
            else
                felEruptionTimer -= diff;
        }

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_BURNING_ARMOR:
                    DoCastTopAggro(SPELL_BURNING_ARMOR);
                    events.RescheduleEvent(EVENT_BURNING_ARMOR, 24000);
                    break;
                case EVENT_CRASHING_COMET:
                {
                    if (IsLfrRaid())
                        DoCast(SPELL_CRASHING_COMET_LFR_FILTER);
                    else
                        DoCast(SPELL_CRASHING_COMET_FILTER);
                    EntryCheckPredicate pred(NPC_EMBER_STALKER);
                    summons.DoAction(ACTION_1, pred);
                    events.RescheduleEvent(EVENT_CRASHING_COMET, 18000);
                    break;
                }
                case EVENT_INFERNAL_SPIKE:
                    DoCast(SPELL_INFERNAL_SPIKE_FILTER);
                    events.RescheduleEvent(EVENT_INFERNAL_SPIKE, 16000);
                    break;
                case EVENT_SHATTERING_STAR:
                    if (++shatteringCounter == 4)
                        shatteringTimer = IsMythicRaid() ? 30000 : 50000;
                    shatteringTargetGUID.Clear();
                    DoCast(SPELL_SHATTERING_STAR_FILTER);
                    events.RescheduleEvent(EVENT_SHATTERING_STAR, shatteringTimer);
                    break;
                case EVENT_RAIN_OF_BRIMSTONE:
                    if (++rainBrimstoneCounter == 4)
                    {
                        rainBrimstoneCounter = 0;
                        rainBrimstoneTimer = 68000;
                    }
                    else
                        rainBrimstoneTimer = 60000;
                    DoCast(SPELL_RAIN_OF_BRIMSTONE_DUMMY);
                    events.RescheduleEvent(EVENT_RAIN_OF_BRIMSTONE, rainBrimstoneTimer);
                    break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//115892
struct npc_goroth_ember_stalker : public ScriptedAI
{
    npc_goroth_ember_stalker(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisplayId(11686);
    }

    void Reset() override {}

    void DoAction(int32 const action) override
    {
        if (IsLfrRaid())
            me->CastSpellDelay(me, SPELL_CRASHING_COMET_LFR_FILTER2, true, 500);
        else
            me->CastSpellDelay(me, SPELL_CRASHING_COMET_FILTER2, true, 500);
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spell) override
    {
        auto owner = me->GetAnyOwner();
        if (!owner)
            return;

        switch (spell->Id)
        {
            case SPELL_CRASHING_COMET_LFR_FILTER2:
                me->CastSpell(target, SPELL_CRASHING_COMET_LFR_MISSILE, true, nullptr, nullptr, owner->GetGUID());
                break;
            case SPELL_CRASHING_COMET_FILTER2:
                me->CastSpell(target, SPELL_CRASHING_COMET_MISSILE, true, nullptr, nullptr, owner->GetGUID());
                break;
        }
    }

    void UpdateAI(uint32 diff) override {}
};

//116976
struct npc_goroth_infernal_spike : public ScriptedAI
{
    npc_goroth_infernal_spike(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    bool despawn = false;

    void IsSummonedBy(Unit* summoner) override
    {
        me->CastSpellDelay(me, SPELL_INFERNAL_SPIKE_AT, false, 500);
    }

    void Reset() override {}

    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        if (despawn || caster->GetEntry() != NPC_GOROTH)
            return;

        if (spell->Id == SPELL_BURNING_ERUPTION_DUMMY || spell->Id == SPELL_RAIN_OF_BRIMSTONE_SPIKE_DESTROY)
            DoAction(true);
    }

    void OnAreaTriggerDespawn(uint32 spellId, Position pos, bool duration) override
    {
        if (despawn)
            return;

        if (spellId == SPELL_INFERNAL_SPIKE_AT)
            DoAction(true);
    }

    void DoAction(int32 const action) override
    {
        if (despawn)
            return;

        despawn = true;

        if (IsMythicRaid())
        {
            if (auto owner = me->GetAnyOwner())
                owner->CastSpell(me, SPELL_INFERNAL_DETONATION, true);
        }

        me->PlaySpellVisual(me->GetPosition(), SPELLVISUAL_INFERNAL_SPIKE_DESTROY, 0.0f, me->GetGUID(), false);
        me->DespawnOrUnsummon(1000);
    }

    void UpdateAI(uint32 diff) override {}
};

//117931
struct npc_goroth_lava_stalker : public ScriptedAI
{
    npc_goroth_lava_stalker(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* summoner) override
    {
        me->CastSpellDelay(me, SPELL_FEL_ERUPTION_TELEGRAPH, true, 500);
        me->CastSpellDelay(me, SPELL_FEL_PERIODIC_TRIGGER, true, 7000);
    }

    void Reset() override {}

    void UpdateAI(uint32 diff) override {}
};

//119950
struct npc_goroth_brimstone_infernal : public ScriptedAI
{
    npc_goroth_brimstone_infernal(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        instance = me->GetInstanceScript();
    }

    InstanceScript* instance;
    uint32 felTimer = 0;

    void IsSummonedBy(Unit* summoner) override
    {
        me->SetReactState(REACT_AGGRESSIVE, 2000);
        felTimer = 12000; //Need correct time!
    }

    void Reset() override {}

    void EnterCombat(Unit* /*who*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
    }

    void JustDied(Unit* /*killer*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (felTimer)
        {
            if (felTimer <= diff)
            {
                felTimer = 15000; //Need correct time!
                DoCast(SPELL_FEL_FIRE);
            }
            else
                felTimer -= diff;
        }
        DoMeleeAttackIfReady();
    }
};

//233024
class spell_tos_goroth_crashing_comet : public SpellScript
{
    PrepareSpellScript(spell_tos_goroth_crashing_comet);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        std::list<WorldObject*> infernalSpikes;
        for (auto target : targets)
        {
            if (target->GetEntry() == NPC_INFERNAL_SPIKE)
                infernalSpikes.push_back(target);
        }

        for (auto infernalSpike : infernalSpikes)
        {
            if (auto spike = Creature::GetCreature(*GetCaster(), infernalSpike->GetGUID()))
                spike->GetAI()->DoAction(true);
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_tos_goroth_crashing_comet::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
    }
};

//234368
class spell_tos_goroth_fel_eruption_dummy : public SpellScript
{
    PrepareSpellScript(spell_tos_goroth_fel_eruption_dummy);

    void HandleScript(SpellEffIndex /*effectIndex*/)
    {
        if (!GetCaster() || !GetHitDest())
            return;

        if (InstanceScript* instance = GetCaster()->GetInstanceScript())
            if (Creature* goroth = instance->instance->GetCreature(instance->GetGuidData(DATA_GOROTH)))
                goroth->CastSpell(GetHitDest(), SPELL_FEL_ERUPTION_AT, true);
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_tos_goroth_fel_eruption_dummy::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

//238588
class spell_tos_goroth_rain_of_brimstone : public SpellScript
{
    PrepareSpellScript(spell_tos_goroth_rain_of_brimstone);

    uint8 targetCount = 0;

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (!GetCaster() || !GetExplTargetDest())
            return;

        GetCaster()->CastSpell(GetExplTargetDest(), SPELL_RAIN_OF_BRIMSTONE_SPIKE_DESTROY, true);

        if (targets.empty())
            GetCaster()->CastSpell(GetExplTargetDest(), SPELL_RAIN_OF_BRIMSTONE_SUMMON, true);
        else
            targetCount = targets.size();
    }

    void HandleDamage(SpellEffIndex /*effectIndex*/)
    {
        if (targetCount)
            SetHitDamage(GetHitDamage() / targetCount);
    }

    void Register()
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_tos_goroth_rain_of_brimstone::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_tos_goroth_rain_of_brimstone::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

//237333
class spell_tos_goroth_energy_tracker : public AuraScript
{
    PrepareAuraScript(spell_tos_goroth_energy_tracker);

    uint8 powerTick = 0;

    void OnTick(AuraEffect const* aurEff)
    {
        auto caster = GetCaster()->ToCreature();
        if (!caster || !caster->isInCombat())
            return;

        auto powerCount = caster->GetPower(POWER_ENERGY);
        if (powerCount < 100)
        {
            if (powerTick < 8)
            {
                ++powerTick;

                if (aurEff->GetTickNumber() == 1)
                    caster->SetPower(POWER_ENERGY, powerCount + 1);
                else
                    caster->SetPower(POWER_ENERGY, powerCount + 2);
            }
            else
            {
                powerTick = 0;
                caster->SetPower(POWER_ENERGY, powerCount + 1);
            }
        }
        else if (!caster->HasUnitState(UNIT_STATE_CASTING))
        {
            caster->CastSpell(caster, SPELL_INFERNAL_BURNING, false);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_tos_goroth_energy_tracker::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

//234386
class spell_tos_goroth_fel_periodic_trigger : public AuraScript
{
    PrepareAuraScript(spell_tos_goroth_fel_periodic_trigger);

    void OnTick(AuraEffect const* aurEff)
    {
        PreventDefaultAction();

        if (auto caster = GetCaster())
        {
            Position pos;
            float angle = caster->GetRelativeAngle(centrPos.GetPositionX(), centrPos.GetPositionY()) + frand(-1.0f, 1.0f);
            caster->GetNearPosition(pos, frand(35.0f, 45.0f), angle);
            caster->CastSpell(pos, SPELL_FEL_ERUPTION_MISSILE, true);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_tos_goroth_fel_periodic_trigger::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

//955600
struct at_goroth_infernal_spike_los_blocker : AreaTriggerAI
{
    at_goroth_infernal_spike_los_blocker(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) {}

    bool IsValidTarget(Unit* caster, Unit* target, AreaTriggerActionMoment actionM) override
    {
        if (caster && target && target->IsPlayer())
        {
            if (auto instance = target->GetInstanceScript())
            {
                if (auto goroth = Creature::GetCreature(*target, instance->GetGuidData(DATA_GOROTH)))
                {
                    if (at->IsInBetween(target, goroth, 2.5f))
                    {
                        if (!target->HasAura(SPELL_INFERNAL_SPIKE_PROTECTION))
                            target->CastSpell(target, SPELL_INFERNAL_SPIKE_PROTECTION, true, nullptr, nullptr, caster->GetGUID());
                    }
                    else if (auto aura = target->GetAura(SPELL_INFERNAL_SPIKE_PROTECTION, caster->GetGUID()))
                        aura->Remove();
                }
            }
        }
        return true;
    }
};

//13412
struct at_goroth_shattering_star : AreaTriggerAI
{
    explicit at_goroth_shattering_star(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) {}

    uint8 infernalSpikeTouched = 1;

    void OnUnitEnter(Unit* unit) override
    {
        auto caster = at->GetCaster();
        if (unit->GetEntry() == NPC_INFERNAL_SPIKE)
        {
            ++infernalSpikeTouched;

            if (unit->IsAIEnabled)
                unit->GetAI()->DoAction(true);

            //Achievement
            if (caster->IsAIEnabled)
                caster->GetAI()->DoAction(ACTION_2);
        }
    }

    void OnDestinationReached() override
    {
        auto caster = at->GetCaster();
        if (!caster)
        {
            at->Remove();
            return;
        }

        if (auto spellInfo = sSpellMgr->GetSpellInfo(SPELL_SHATTERING_NOVA))
        {
            if (auto effInfo = spellInfo->GetEffect(EFFECT_0, caster->GetMap()->GetDifficultyID()))
            {
                float damage = effInfo->BasePoints / std::max(infernalSpikeTouched, uint8(1));
                SpellCastTargets targets;
                targets.SetCaster(caster);
                targets.SetDst(at->GetPosition());
                CustomSpellValues values;
                values.AddSpellMod(SPELLVALUE_BASE_POINT0, damage);

                caster->CastSpell(targets, spellInfo, &values, TriggerCastFlags(TRIGGERED_FULL_MASK));
            }
        }
        if (infernalSpikeTouched < 2 && caster->IsAIEnabled)
            caster->GetAI()->DoAction(ACTION_1);
    }
};

//13526
struct at_goroth_fel_pool : AreaTriggerAI
{
    explicit at_goroth_fel_pool(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) {}

    void OnUnitEnter(Unit* unit) override
    {
        if (unit->GetEntry() == NPC_INFERNAL_SPIKE)
            unit->GetAI()->DoAction(true);
    }
};

//36337
class achievement_fel_turkey : public AchievementCriteriaScript
{
public:
    achievement_fel_turkey() : AchievementCriteriaScript("achievement_fel_turkey") {}

    bool OnCheck(Player* /*player*/, Unit* target) override
    {
        if (!target)
            return false;

        if (auto boss = target->ToCreature())
            if (boss->IsAIEnabled && boss->AI()->GetData(DATA_SPIKES_COUNTER) >= 30)
                return true;

        return false;
    }
};

void AddSC_boss_goroth()
{
    RegisterCreatureAI(boss_goroth);
    RegisterCreatureAI(npc_goroth_ember_stalker);
    RegisterCreatureAI(npc_goroth_infernal_spike);
    RegisterCreatureAI(npc_goroth_lava_stalker);
    RegisterCreatureAI(npc_goroth_brimstone_infernal);
    RegisterSpellScript(spell_tos_goroth_crashing_comet);
    RegisterSpellScript(spell_tos_goroth_fel_eruption_dummy);
    RegisterSpellScript(spell_tos_goroth_rain_of_brimstone);
    RegisterAuraScript(spell_tos_goroth_energy_tracker);
    RegisterAuraScript(spell_tos_goroth_fel_periodic_trigger);
    RegisterAreaTriggerAI(at_goroth_infernal_spike_los_blocker);
    RegisterAreaTriggerAI(at_goroth_shattering_star);
    RegisterAreaTriggerAI(at_goroth_fel_pool);
    new achievement_fel_turkey();
}
