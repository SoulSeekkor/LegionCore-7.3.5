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
SDName: Instance_Gruuls_Lair
SD%Complete: 100
SDComment:
SDCategory: Gruul's Lair
EndScriptData */

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "gruuls_lair.h"

#define MAX_ENCOUNTER 2

/* Gruuls Lair encounters:
1 - High King Maulgar event
2 - Gruul event
*/

class instance_gruuls_lair : public InstanceMapScript
{
public:
    instance_gruuls_lair() : InstanceMapScript("instance_gruuls_lair", 565) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_gruuls_lair_InstanceMapScript(map);
    }

    struct instance_gruuls_lair_InstanceMapScript : public InstanceScript
    {
        instance_gruuls_lair_InstanceMapScript(Map* map) : InstanceScript(map) {}

        uint32 m_auiEncounter[MAX_ENCOUNTER];

        ObjectGuid MaulgarEvent_Tank;
        ObjectGuid KigglerTheCrazed;
        ObjectGuid BlindeyeTheSeer;
        ObjectGuid OlmTheSummoner;
        ObjectGuid KroshFirehand;
        ObjectGuid Maulgar;

        ObjectGuid MaulgarDoor;
        ObjectGuid GruulDoor;

        void Initialize() override
        {
            memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

            MaulgarEvent_Tank.Clear();
            KigglerTheCrazed.Clear();
            BlindeyeTheSeer.Clear();
            OlmTheSummoner.Clear();
            KroshFirehand.Clear();
            Maulgar.Clear();

            MaulgarDoor.Clear();
            GruulDoor.Clear();
        }

        bool IsEncounterInProgress() const override
        {
            for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                if (m_auiEncounter[i] == IN_PROGRESS)
                    return true;

            return false;
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case 18835: KigglerTheCrazed = creature->GetGUID(); break;
                case 18836: BlindeyeTheSeer = creature->GetGUID();  break;
                case 18834: OlmTheSummoner = creature->GetGUID();   break;
                case 18832: KroshFirehand = creature->GetGUID();    break;
                case 18831: Maulgar = creature->GetGUID();          break;
            }
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
                case 184468:
                    MaulgarDoor = go->GetGUID();
                    if (m_auiEncounter[0] == DONE)
                        HandleGameObject(ObjectGuid::Empty, true, go);
                    break;
                case 184662:
                    GruulDoor = go->GetGUID();
                    break;
            }
        }

        void SetGuidData(uint32 type, ObjectGuid data) override
        {
            if (type == DATA_MAULGAREVENT_TANK)
                MaulgarEvent_Tank = data;
        }

        ObjectGuid GetGuidData(uint32 identifier) const override
        {
            switch (identifier)
            {
                case DATA_MAULGAREVENT_TANK:    return MaulgarEvent_Tank;
                case DATA_KIGGLERTHECRAZED:     return KigglerTheCrazed;
                case DATA_BLINDEYETHESEER:      return BlindeyeTheSeer;
                case DATA_OLMTHESUMMONER:       return OlmTheSummoner;
                case DATA_KROSHFIREHAND:        return KroshFirehand;
                case DATA_MAULGARDOOR:          return MaulgarDoor;
                case DATA_GRUULDOOR:            return GruulDoor;
                case DATA_MAULGAR:              return Maulgar;
            }
            return ObjectGuid::Empty;
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
                case DATA_MAULGAREVENT:
                    if (data == DONE)
                        HandleGameObject(MaulgarDoor, true);
                    m_auiEncounter[0] = data;
                    break;

                case DATA_GRUULEVENT:
                    if (data == IN_PROGRESS)
                        HandleGameObject(GruulDoor, false);
                    else
                        HandleGameObject(GruulDoor, true);
                    m_auiEncounter[1] = data;
                    break;
            }

            if (data == DONE)
                SaveToDB();
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
                case DATA_MAULGAREVENT: return m_auiEncounter[0];
                case DATA_GRUULEVENT:   return m_auiEncounter[1];
            }
            return 0;
        }

        std::string GetSaveData() override
        {
            OUT_SAVE_INST_DATA;
            std::ostringstream stream;
            stream << m_auiEncounter[0] << ' ' << m_auiEncounter[1];

            OUT_SAVE_INST_DATA_COMPLETE;
            return stream.str();
        }

        void Load(const char* in) override
        {
            if (!in)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(in);
            std::istringstream stream(in);
            stream >> m_auiEncounter[0] >> m_auiEncounter[1];
            for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                if (m_auiEncounter[i] == IN_PROGRESS)                // Do not load an encounter as "In Progress" - reset it instead.
                    m_auiEncounter[i] = NOT_STARTED;
            OUT_LOAD_INST_DATA_COMPLETE;
        }
    };

};

void AddSC_instance_gruuls_lair()
{
    new instance_gruuls_lair();
}
