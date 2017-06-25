-- phpMyAdmin SQL Dump
-- version 2.9.1.1-Debian-2ubuntu1
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Oct 03, 2007 at 12:10 AM
-- Server version: 5.0.38
-- PHP Version: 5.2.1
-- 
-- Database: `tm`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `accounts`
-- 

CREATE TABLE `accounts` (
  `username` varchar(40) NOT NULL COMMENT 'email address',
  `password` varchar(40) NOT NULL COMMENT 'password',
  `id` int(11) NOT NULL auto_increment,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 COMMENT='Ticketmaster account info' AUTO_INCREMENT=14 ;

-- 
-- Dumping data for table `accounts`
-- 

INSERT INTO `accounts` (`username`, `password`, `id`) VALUES 
('daniellesusan@aol.com', 'maiseybaby1', 1),
('houevents@aol.com', 'qqqwwe', 2),
('thomaswheeler1@aol.com', 'qqqwwe', 12),
('gricelda10@aol.com', 'qqqwwe', 11),
('christinaspec@aol.com', 'www2234', 13);
