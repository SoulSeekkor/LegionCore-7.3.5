-- Fix loot chance for the following quest items

-- These should all be 100%
-- 981 = Bernice's Necklace
-- 1006 = Brass Collar
-- 1946 = Mary's Looking Glass
-- 1968 = Ogre's Monocle
-- 2713 = Ol' Sooty's Head
-- 59522 = Key of Ilgalar

-- 60792 = Pristine Flight Feather (bump from 35% to 50%)

DELETE FROM `creature_loot_template` WHERE `item` IN (981,1006,1946,1968,2713,59522,60792);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`, `shared`) VALUES
(327, 981, -100, 0, 0, 1, 1, 0),
(330, 1006, -100, 0, 0, 1, 1, 0),
(511, 1946, -100, 0, 0, 1, 1, 0),
(300, 1968, -100, 0, 0, 1, 1, 0),
(1225, 2713, -100, 0, 0, 1, 1, 0),
(703, 59522, -100, 0, 0, 1, 1, 0),
(44628, 60792, -50, 0, 0, 1, 1, 0);

-- Fix broken "Rescue the Survivors!" quest (can't heal the Draenei Survivor, wrong faction and NPC flag)
DELETE FROM `creature_template` WHERE `entry` = 16483;
INSERT INTO `creature_template` (`entry`, `gossip_menu_id`, `minlevel`, `maxlevel`, `SandboxScalingID`, `ScaleLevelMin`, `ScaleLevelMax`, `ScaleLevelDelta`, `ScaleLevelDuration`, `exp`, `faction`, `npcflag`, `npcflag2`, `speed_walk`, `speed_run`, `speed_fly`, `scale`, `mindmg`, `maxdmg`, `dmgschool`, `attackpower`, `dmg_multiplier`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `unit_flags2`, `unit_flags3`, `dynamicflags`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `minrangedmg`, `maxrangedmg`, `rangedattackpower`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `InhabitType`, `HoverHeight`, `Mana_mod_extra`, `Armor_mod`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ControllerID`, `WorldEffects`, `PassiveSpells`, `StateWorldEffectID`, `SpellStateVisualID`, `SpellStateAnimID`, `SpellStateAnimKitID`, `IgnoreLos`, `AffixState`, `MaxVisible`, `ScriptName`) VALUES
('16483', '0', '1', '5', '165', '0', '0', '0', '0', '0', '1638', '0', '0', '1', '1.14286', '1.14286', '1', '5', '6', '0', '2', '1', '2000', '0', '1', '4608', '2048', '0', '8', '0', '0', '0', '0', '3', '4', '100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', 'SmartAI', '1', '3', '1', '1', '1', '0', '0', '2', '0', '', '', '0', '0', '0', '0', '0', '0', '0', 'npc_draenei_survivor');

-- Fix broken "Thistle While You Work" quest (can't loot the seeds)
DELETE FROM `conditions` WHERE `SourceEntry` = 60737;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(4, 205089, 60737, 0, 0, 9, 0, 27025, 0, 0, 0, 0, '', '');

DELETE FROM `gameobject_template` WHERE `entry` = 205089;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `faction`, `flags`, `size`, `questItem1`, `questItem2`, `questItem3`, `questItem4`, `questItem5`, `questItem6`, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `Data24`, `Data25`, `Data26`, `Data27`, `Data28`, `Data29`, `Data30`, `Data31`, `Data32`, `unkInt32`, `AIName`, `ScriptName`, `WorldEffectID`, `StateWorldEffectID`, `SpellVisualID`, `SpellStateVisualID`, `SpellStateAnimID`, `SpellStateAnimKitID`, `MaxVisible`, `IgnoreDynLos`, `MinGold`, `MaxGold`, `VerifiedBuild`) VALUES
(205089, 3, 7918, 'Stabthistle Seed', '', 'Collecting', '', 0, 4, 0.5, 60737, 0, 0, 0, 0, 0, 43, 205089, 0, 1, 0, 0, 0, 0, 27025, 0, 0, 0, 0, 0, 19676, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);