-- Create table to hold zone level information for scaling globally
CREATE TABLE `reference_zone_level` (
  `zoneId` INT UNSIGNED NOT NULL,
  `minLevel` TINYINT UNSIGNED DEFAULT NULL,
  `maxLevel` TINYINT UNSIGNED DEFAULT NULL,
  `description` VARCHAR(255) DEFAULT NULL,
  PRIMARY KEY (`zoneId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb3;

DELETE FROM `reference_zone_level`;
INSERT INTO `reference_zone_level` (`zoneId`, `minLevel`, `maxLevel`, `description`) VALUES
(1, 1, 20, 'Dun Morogh'),
(3, 40, 60, 'Badlands'),
(4, 40, 60, 'Blasted Lands'),
(8, 40, 60, 'Swamp of Sorrows'),
(10, 20, 60, 'Duskwood'),
(11, 20, 60, 'Wetlands'),
(12, 1, 20, 'Elwynn Forest'),
(14, 1, 20, 'Durotar'),
(15, 35, 60, 'Dustwallow Marsh'),
(16, 10, 60, 'Azshara'),
(65, 61, 80, 'Dragonblight'),
(66, 64, 80, 'Zul''Drak'),
(67, 67, 80, 'The Storm Peaks'),
(210, 67, 80, 'Icecrown'),
(394, 63, 80, 'Grizzly Hills'),
(495, 58, 80, 'Howling Fjord'),
(2817, 67, 80, 'Crystalsong Forest'),
(3537, 58, 80, 'Borean Tundra'),
(3711, 66, 80, 'Scholazar Basin'),
(4197, 67, 80, 'Wintergrasp'),
(4742, 67, 80, 'Hrothgar''s Landing'),
(6170, 1, 20, 'Northshire Valley'),
(6455, 1, 20, 'Sunstrider Isle');