# MySQL-Front Dump 2.4
#
# Host: localhost   Database: nel
#--------------------------------------------------------
# Server version 3.23.34


#
# Table structure for table 'shards'
#

CREATE TABLE `shards` (
  `ShardId` int(10) NOT NULL auto_increment,
  `WsAddr` tinytext,
  `NbPlayers` int(10) unsigned default '0',
  `ShardName` tinytext,
  `Online` tinyint(1) unsigned default '0',
  `ProgramName` tinytext NOT NULL,
  PRIMARY KEY  (`ShardId`)
) TYPE=MyISAM COMMENT='contains all shards informations for login system';



#
# Table structure for table 'users'
#

CREATE TABLE `users` (
  `UId` int(10) NOT NULL auto_increment,
  `Login` tinytext NOT NULL,
  `Password` tinytext NOT NULL,
  `ShardId` int(10) NOT NULL default '-1',
  `State` enum('Offline','Online') NOT NULL default 'Offline',
  PRIMARY KEY  (`UId`)
) TYPE=MyISAM COMMENT='contains all users informations for login system';

