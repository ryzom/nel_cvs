#
# Database structure
#


CREATE DATABASE nel;

USE nel;

#
# Table structure for table 'shard'
#

CREATE TABLE `shard` (
  `ShardId` int(10) NOT NULL auto_increment,
  `WsAddr` tinytext,
  `NbPlayers` int(10) unsigned default '0',
  `ShardName` tinytext,
  `Online` tinyint(1) unsigned default '0',
  `ProgramName` tinytext NOT NULL,
  PRIMARY KEY  (`ShardId`)
) TYPE=MyISAM COMMENT='contains all shard informations for login system';



#
# Table structure for table 'user'
#

CREATE TABLE `user` (
  `UId` int(10) NOT NULL auto_increment,
  `Login` tinytext NOT NULL,
  `Password` tinytext NOT NULL,
  `ShardId` int(10) NOT NULL default '-1',
  `State` enum('Offline','Online') NOT NULL default 'Offline',
  PRIMARY KEY  (`UId`)
) TYPE=MyISAM COMMENT='contains all user informations for login system';

