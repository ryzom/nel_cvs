# MySQL-Front Dump 2.4
#
# Host: localhost   Database: nel
#--------------------------------------------------------
# Server version 3.23.34


#
# Table structure for table 'shard'
#

CREATE TABLE `shard` (
  `ShardId` int(10) NOT NULL auto_increment,
  `WsAddr` varchar(64) default NULL,
  `NbPlayers` int(10) unsigned default '0',
  `ShardName` varchar(64) default NULL,
  `Online` tinyint(1) unsigned default '0',
  `ProgramName` varchar(64) NOT NULL default '',
  PRIMARY KEY  (`ShardId`)
) TYPE=MyISAM COMMENT='contains all shards informations for login system';



#
# Table structure for table 'user'
#

CREATE TABLE `user` (
  `UId` int(10) NOT NULL auto_increment,
  `Login` varchar(64) NOT NULL default '',
  `Password` tinytext NOT NULL,
  `ShardId` int(10) NOT NULL default '-1',
  `State` enum('Offline','Online') NOT NULL default 'Offline',
  PRIMARY KEY  (`UId`)
) TYPE=MyISAM COMMENT='contains all users informations for login system';

