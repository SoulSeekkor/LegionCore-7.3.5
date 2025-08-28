-- Fix script name
UPDATE `spell_script_names` SET `ScriptName` = 'spell_fallen_protectors_shadow_weakness_proc' WHERE `spell_id` = 144079;

-- Fix commmand help
UPDATE `command` SET `help` = 'Syntax: .cast #spellid [triggered]\n   Cast #spellid to selected target. If no target selected cast on self. If ''triggered'' provided then spell cast with triggered flag.' WHERE `name` = 'cast';
UPDATE `command` SET `help` = 'Syntax: .cast back #spellid [triggered]\n   Selected target will cast #spellid on your character. If ''triggered'' provided then spell cast with triggered flag.' WHERE `name` = 'cast back';
UPDATE `command` SET `help` = 'Syntax: .cast dest #spellid #x #y #z [triggered]\n   Selected target will cast #spellid at provided destination. If ''triggered'' provided then spell cast with triggered flag.' WHERE `name` = 'cast dest';
UPDATE `command` SET `help` = 'Syntax: .cast dist #spellid [#dist [triggered]]\n   You will cast spell to point at distance #dist. If ''triggered'' provided then spell cast with triggered flag. Not all spells can be cast as area spells.' WHERE `name` = 'cast dist';
UPDATE `command` SET `help` = 'Syntax: .cast self #spellid [triggered]\n Cast #spellid by target at target itself. If ''triggered'' provided then spell cast with triggered flag.' WHERE `name` = 'cast self';
UPDATE `command` SET `help` = 'Syntax: .cast target #spellid [triggered]\n   Selected target will cast #spellid to his victim. If ''triggered'' provided then spell cast with triggered flag.' WHERE `name` = 'cast target';

-- Fix condition comments
UPDATE `conditions` SET `Comment` = 'DARK_PORTAL_INTRO SPELL 162685 if not triggered scene' WHERE `SourceTypeOrReferenceId` = 17 AND `SourceGroup` = 0 AND `SourceEntry` = 162685 AND `ConditionValue1` = 724;
UPDATE `conditions` SET `Comment` = 'DARK_PORTAL_INTRO SPELL 163023 if not triggered scene' WHERE `SourceTypeOrReferenceId` = 17 AND `SourceGroup` = 0 AND `SourceEntry` = 163023 AND `ConditionValue1` = 727;
UPDATE `conditions` SET `Comment` = 'DARK_PORTAL_INTRO spell 163770 not trigger teleport for 801' WHERE `SourceTypeOrReferenceId` = 17 AND `SourceGroup` = 0 AND `SourceEntry` = 163770 AND `ConditionValue1` = 753;
UPDATE `conditions` SET `Comment` = 'DARK_PORTAL_INTRO SPELL 164609 if not triggered scene' WHERE `SourceTypeOrReferenceId` = 17 AND `SourceGroup` = 0 AND `SourceEntry` = 164609 AND `ConditionValue1` = 770;
UPDATE `conditions` SET `Comment` = 'DARK_PORTAL_INTRO SPELL 165549 Q34429 not trigger scene 796' WHERE `SourceTypeOrReferenceId` = 17 AND `SourceGroup` = 0 AND `SourceEntry` = 165549 AND `ConditionValue1` = 796;
UPDATE `conditions` SET `Comment` = 'Draenor. FrostFireRidge.  trigger scene 604' WHERE `SourceTypeOrReferenceId` = 23 AND `SourceGroup` = 6720 AND `SourceEntry` = 6 AND `ConditionValue1` = 604;
UPDATE `conditions` SET `Comment` = 'DARK_PORTAL_INTRO tmp phase after proc scene trigger Bridge for 727' WHERE `SourceTypeOrReferenceId` = 23 AND `SourceGroup` = 7025 AND `SourceEntry` = 18 AND `ConditionValue1` = 727;
UPDATE `conditions` SET `Comment` = 'DARK_PORTAL_INTRO Q34429 after proc scene trigger 796' WHERE `SourceTypeOrReferenceId` = 23 AND `SourceGroup` = 7025 AND `SourceEntry` = 22 AND `ConditionValue1` = 796;
UPDATE `conditions` SET `Comment` = 'DARK_PORTAL_INTRO tmp phase after proc scene trigger teleport for 801' WHERE `SourceTypeOrReferenceId` = 23 AND `SourceGroup` = 7025 AND `SourceEntry` = 30 AND `ConditionValue1` = 753;
UPDATE `conditions` SET `Comment` = 'DARK_PORTAL_INTRO tmp phase after proc scene trigger teleport for 801' WHERE `SourceTypeOrReferenceId` = 23 AND `SourceGroup` = 7025 AND `SourceEntry` = 33 AND `ConditionValue1` = 753;

-- Fix column name
ALTER TABLE `spell_scene_event` RENAME COLUMN `trigerSpell` TO `TriggerSpell`;

-- Update creature scripts
UPDATE `creature_template` SET `ScriptName` = 'mob_mandori_trigger' WHERE `entry` = 59962;