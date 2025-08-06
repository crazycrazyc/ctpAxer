CREATE TABLE `instrument_basic_info` (  `id` bigint(11) NOT NULL AUTO_INCREMENT,  `datenum` int(11) DEFAULT NULL,  `timenum` int(11) DEFAULT NULL,  `exname` varchar(100) DEFAULT NULL,  `pz` varchar(50) DEFAULT NULL,  `instrumentid` varchar(50) DEFAULT NULL,  `end_month` int(11) DEFAULT NULL,  `maxposition` int(11) DEFAULT '9999999' COMMENT '???ֲ?',  `maxopen` int(11) DEFAULT '9999999' COMMENT '??󿪲?',  `vol` int(11) DEFAULT NULL COMMENT '?ɽ???',  `opi` int(11) DEFAULT NULL COMMENT '?ֲ???',  `far_ratio` int(11) DEFAULT NULL,  `cur_far_ratio` int(10) DEFAULT '0',  `near_ratio` int(11) DEFAULT NULL,  `cur_near_ratio` int(10) DEFAULT '0',  `tick_size` double(7,4) DEFAULT '1.0',  `factor` int(11) DEFAULT '10',`update_time` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',  PRIMARY KEY (`id`),  UNIQUE KEY `instrumentid` (`instrumentid`,`datenum`) USING BTREE) ENGINE=InnoDB AUTO_INCREMENT=681485 DEFAULT CHARSET=utf8mb4;

host:172.16.30.97
port:13307
user:elwriter
password:elwriter123
database: cta_trader
