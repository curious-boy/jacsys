/*
Navicat MySQL Data Transfer

Source Server         : 192.168.0.106
Source Server Version : 50173
Source Host           : 192.168.0.106:3306
Source Database       : jacdb

Target Server Type    : MYSQL
Target Server Version : 50173
File Encoding         : 65001

Date: 2016-09-08 21:34:24
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `data_dict`
-- ----------------------------
DROP TABLE IF EXISTS `data_dict`;
CREATE TABLE `data_dict` (
  `id` int(11) NOT NULL,
  `table_name` varchar(30) DEFAULT NULL COMMENT '字典类型',
  `col_name` varchar(30) DEFAULT NULL COMMENT '字典项',
  `value` varchar(30) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of data_dict
-- ----------------------------

-- ----------------------------
-- Table structure for `employee_info`
-- ----------------------------
DROP TABLE IF EXISTS `employee_info`;
CREATE TABLE `employee_info` (
  `job_id` varchar(30) NOT NULL COMMENT '员工编号\n工号\n',
  `name` varchar(20) DEFAULT NULL COMMENT '姓名',
  `work_type` int(11) DEFAULT NULL COMMENT '工种',
  `hire_date` datetime DEFAULT NULL COMMENT '入职日期',
  `phone_number` varchar(15) DEFAULT NULL COMMENT '联系电话',
  `work_group` int(11) DEFAULT NULL COMMENT '班组\n工作组\n用于优化工作任务分配',
  `password` varchar(10) DEFAULT NULL COMMENT '密码',
  PRIMARY KEY (`job_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of employee_info
-- ----------------------------
INSERT INTO `employee_info` VALUES ('0001', '张三', '1', '2016-08-16 20:17:03', '13082222211', '222', '333333');

-- ----------------------------
-- Table structure for `fault_record`
-- ----------------------------
DROP TABLE IF EXISTS `fault_record`;
CREATE TABLE `fault_record` (
  `fault_id` int(11) NOT NULL COMMENT '故障记录编号\n',
  `register_time` timestamp NULL DEFAULT NULL COMMENT '登记时间',
  `fault_type` varchar(45) DEFAULT NULL COMMENT '故障类型',
  `machine_id` varchar(30) DEFAULT NULL COMMENT '节点编号',
  `machine_type` int(11) DEFAULT NULL COMMENT '机器类型',
  `status` varchar(30) DEFAULT NULL COMMENT '故障状态\n',
  `operator_time` timestamp NULL DEFAULT NULL COMMENT '处理时间',
  `description` varchar(80) DEFAULT NULL COMMENT '描述',
  `remarks` varchar(80) DEFAULT NULL COMMENT '备注',
  PRIMARY KEY (`fault_id`),
  KEY `fk_fault_record_machine_management1_idx` (`operator_time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of fault_record
-- ----------------------------

-- ----------------------------
-- Table structure for `figure_info`
-- ----------------------------
DROP TABLE IF EXISTS `figure_info`;
CREATE TABLE `figure_info` (
  `id` int(11) NOT NULL COMMENT '记录编号',
  `register_time` timestamp NULL DEFAULT NULL COMMENT '登记时间',
  `figure_name` varchar(30) DEFAULT NULL COMMENT '花机名称',
  `customer_id` varchar(30) DEFAULT NULL COMMENT '客房编号',
  `latitude` int(11) DEFAULT NULL COMMENT '纬度',
  `opening` int(11) DEFAULT NULL COMMENT '开度',
  `tasks_number` int(11) DEFAULT NULL COMMENT '任务数',
  `number_produced` int(11) DEFAULT NULL COMMENT '已经生产的产量',
  `how_long_to_finish` int(11) DEFAULT NULL COMMENT '距任务完成时间',
  `concurrent_produce_number` int(11) DEFAULT NULL COMMENT '同时产出条数',
  `machine_id` varchar(30) DEFAULT NULL COMMENT '节点编号\n',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of figure_info
-- ----------------------------

-- ----------------------------
-- Table structure for `login_info`
-- ----------------------------
DROP TABLE IF EXISTS `login_info`;
CREATE TABLE `login_info` (
  `id` int(11) NOT NULL COMMENT '记录编号\n用户登录记录',
  `job_id` varchar(30) DEFAULT NULL COMMENT '工号\n员工编号',
  `login_type` int(11) DEFAULT NULL COMMENT '登录类型\n登录\n注销\n',
  `time` timestamp NULL DEFAULT NULL COMMENT '发生时间',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of login_info
-- ----------------------------

-- ----------------------------
-- Table structure for `machine_info`
-- ----------------------------
DROP TABLE IF EXISTS `machine_info`;
CREATE TABLE `machine_info` (
  `machine_id` varchar(30) NOT NULL COMMENT '节点编号',
  `register_time` timestamp NULL DEFAULT NULL COMMENT '登记时间',
  `total_run_time` mediumtext COMMENT '机台累计开机时长',
  PRIMARY KEY (`machine_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of machine_info
-- ----------------------------

-- ----------------------------
-- Table structure for `machine_management`
-- ----------------------------
DROP TABLE IF EXISTS `machine_management`;
CREATE TABLE `machine_management` (
  `machine_id` varchar(30) NOT NULL COMMENT '机器编号\n节点编号\n',
  `register_time` timestamp NULL DEFAULT NULL COMMENT '登记时间',
  `addr` int(11) DEFAULT NULL COMMENT '节点地址',
  `gateway` varchar(30) DEFAULT NULL COMMENT '网关名称',
  `machine_type` int(11) DEFAULT NULL COMMENT '机器类型\n',
  `row` int(11) DEFAULT NULL COMMENT '行',
  `col` int(11) DEFAULT NULL COMMENT '列',
  `thread_number` int(11) DEFAULT NULL COMMENT '针数',
  PRIMARY KEY (`machine_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of machine_management
-- ----------------------------

-- ----------------------------
-- Table structure for `machine_statis`
-- ----------------------------
DROP TABLE IF EXISTS `machine_statis`;
CREATE TABLE `machine_statis` (
  `id` int(11) NOT NULL COMMENT '记录编号',
  `register_time` timestamp NULL DEFAULT NULL COMMENT '登记时间',
  `operator` varchar(30) DEFAULT NULL COMMENT '值机工号',
  `product_total_time` int(11) DEFAULT NULL COMMENT '计件时长',
  `product_total_output` int(11) DEFAULT NULL COMMENT '计件产量',
  `machine_id` varchar(30) DEFAULT NULL COMMENT '节点编号',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of machine_statis
-- ----------------------------

-- ----------------------------
-- Table structure for `machine_status`
-- ----------------------------
DROP TABLE IF EXISTS `machine_status`;
CREATE TABLE `machine_status` (
  `machine_id` varchar(30) NOT NULL COMMENT '节点编号',
  `register_time` timestamp NULL DEFAULT NULL COMMENT '登记时间',
  `machine_state` int(11) DEFAULT NULL COMMENT '机台当前运行状态',
  `broken_total_time` mediumtext COMMENT '停机累计时长',
  `halting_reason` int(11) DEFAULT NULL COMMENT '停机原因\n',
  `broken_halt` int(11) DEFAULT NULL COMMENT '断纱停机',
  PRIMARY KEY (`machine_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of machine_status
-- ----------------------------

-- ----------------------------
-- Table structure for `node_register_info`
-- ----------------------------
DROP TABLE IF EXISTS `node_register_info`;
CREATE TABLE `node_register_info` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `gateway` varchar(30) NOT NULL,
  `ip` varchar(30) DEFAULT NULL,
  `node` smallint(6) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=5 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of node_register_info
-- ----------------------------
INSERT INTO `node_register_info` VALUES ('4', 'g001', '192.168.0.1', '100');

-- ----------------------------
-- Table structure for `production_info`
-- ----------------------------
DROP TABLE IF EXISTS `production_info`;
CREATE TABLE `production_info` (
  `id` int(11) NOT NULL COMMENT '记录编号',
  `register_time` timestamp NULL DEFAULT NULL COMMENT '登记时间',
  `operator` varchar(30) DEFAULT NULL COMMENT '值机工号',
  `figure_name` varchar(30) DEFAULT NULL COMMENT '花样名称\n',
  `product_total_time` int(11) DEFAULT NULL COMMENT '记件时长',
  `product_total_output` int(11) DEFAULT NULL COMMENT '计件产量',
  `machine_id` varchar(30) DEFAULT NULL COMMENT '节点编号',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of production_info
-- ----------------------------

-- ----------------------------
-- Table structure for `repair_record`
-- ----------------------------
DROP TABLE IF EXISTS `repair_record`;
CREATE TABLE `repair_record` (
  `repair_id` int(11) NOT NULL COMMENT '维修记录编号',
  `job_id` varchar(45) DEFAULT NULL COMMENT '工号\n员工编号',
  `machine_id` varchar(30) DEFAULT NULL COMMENT '机器编号\n节点编号',
  `start_time` timestamp NULL DEFAULT NULL COMMENT '维修开始时间',
  `end_time` timestamp NULL DEFAULT NULL COMMENT '维修结束时间',
  `description` varchar(80) DEFAULT NULL COMMENT '描述',
  `picture_1` blob COMMENT '图片1',
  `picture_2` blob COMMENT '图片2',
  `picture_3` blob COMMENT '图片3',
  PRIMARY KEY (`repair_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of repair_record
-- ----------------------------

-- ----------------------------
-- Table structure for `work_record`
-- ----------------------------
DROP TABLE IF EXISTS `work_record`;
CREATE TABLE `work_record` (
  `work_id` varchar(30) NOT NULL COMMENT '工单编号\n工单记录表',
  `job_id` varchar(30) DEFAULT NULL COMMENT '员工编号\n工号\n',
  `fault_id` varchar(30) DEFAULT NULL COMMENT '故障编号',
  `machine_id` varchar(30) DEFAULT NULL COMMENT '机台编号',
  `time` timestamp NULL DEFAULT NULL COMMENT '开始时间',
  `endtime` timestamp NULL DEFAULT NULL COMMENT '结束时间',
  `work_state` varchar(10) DEFAULT NULL COMMENT '故障的状态\n如完成，派发，转发等',
  `previous_item` varchar(20) DEFAULT NULL COMMENT '相关联的前一个工单号',
  `next_item` varchar(20) DEFAULT NULL COMMENT '后一个工单号',
  PRIMARY KEY (`work_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of work_record
-- ----------------------------
