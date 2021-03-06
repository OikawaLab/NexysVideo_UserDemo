-------------------------------------------------------------------------------
--                                                                 
--  COPYRIGHT (C) 2013, Digilent RO. All rights reserved
--
-------------------------------------------------------------------------------
--
--  Copyright (C) 2009 OutputLogic.com 
--  This source file may be used and distributed without restriction 
--  provided that this copyright statement is not removed from the file 
--  and that any derivative work contains the original copyright notice 
--  and the associated disclaimer. 
-- 
--  THIS SOURCE FILE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS 
--  OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED	
--  WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
--                                                                  
-------------------------------------------------------------------------------
-- FILE NAME : crc7.vhd
-- MODULE NAME : 7-bit CRC Generator
-- AUTHOR : Mihaita Nagy
-- AUTHOR'S EMAIL : mihaita.nagy@digilent.ro
-------------------------------------------------------------------------------
-- REVISION HISTORY
-- VERSION  DATE         AUTHOR         DESCRIPTION
-- 1.0 	   2013-01-28   Mihaita Nagy   Created
-------------------------------------------------------------------------------
-- DESCRIPTION : Generates parallel 7-bit Cyclic Redundancy Check code for 
-- either 40-bit or 120-bit of input data with the following generator poly: 
-- G(x) = x^7 + x^3 + 1.
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity crc7 is
   port(
      clk_i : in std_logic;
      crc_en_i : in std_logic;
      data_in : in std_logic_vector(39 downto 0);
      crc_o : out std_logic_vector(6 downto 0)
   );
end crc7;

architecture Behavioral of crc7 is

signal lfsr_q: std_logic_vector(6 downto 0) := (others => '0');
signal lfsr_c: std_logic_vector(6 downto 0);

signal crc_en_rise, crc_en_fall, crc_en_tmp : std_logic;

begin
   
   crc_o <= lfsr_q;

   lfsr_c(0) <= lfsr_q(1) xor lfsr_q(2) xor lfsr_q(4) xor lfsr_q(6) xor data_in(0) xor data_in(4) xor data_in(7) xor data_in(8) xor data_in(12) xor data_in(14) xor data_in(15) xor data_in(16) xor data_in(18) xor data_in(20) xor data_in(21) xor data_in(23) xor data_in(24) xor data_in(30) xor data_in(31) xor data_in(34) xor data_in(35) xor data_in(37) xor data_in(39);
   lfsr_c(1) <= lfsr_q(2) xor lfsr_q(3) xor lfsr_q(5) xor data_in(1) xor data_in(5) xor data_in(8) xor data_in(9) xor data_in(13) xor data_in(15) xor data_in(16) xor data_in(17) xor data_in(19) xor data_in(21) xor data_in(22) xor data_in(24) xor data_in(25) xor data_in(31) xor data_in(32) xor data_in(35) xor data_in(36) xor data_in(38);
   lfsr_c(2) <= lfsr_q(0) xor lfsr_q(3) xor lfsr_q(4) xor lfsr_q(6) xor data_in(2) xor data_in(6) xor data_in(9) xor data_in(10) xor data_in(14) xor data_in(16) xor data_in(17) xor data_in(18) xor data_in(20) xor data_in(22) xor data_in(23) xor data_in(25) xor data_in(26) xor data_in(32) xor data_in(33) xor data_in(36) xor data_in(37) xor data_in(39);
   lfsr_c(3) <= lfsr_q(0) xor lfsr_q(2) xor lfsr_q(5) xor lfsr_q(6) xor data_in(0) xor data_in(3) xor data_in(4) xor data_in(8) xor data_in(10) xor data_in(11) xor data_in(12) xor data_in(14) xor data_in(16) xor data_in(17) xor data_in(19) xor data_in(20) xor data_in(26) xor data_in(27) xor data_in(30) xor data_in(31) xor data_in(33) xor data_in(35) xor data_in(38) xor data_in(39);
   lfsr_c(4) <= lfsr_q(1) xor lfsr_q(3) xor lfsr_q(6) xor data_in(1) xor data_in(4) xor data_in(5) xor data_in(9) xor data_in(11) xor data_in(12) xor data_in(13) xor data_in(15) xor data_in(17) xor data_in(18) xor data_in(20) xor data_in(21) xor data_in(27) xor data_in(28) xor data_in(31) xor data_in(32) xor data_in(34) xor data_in(36) xor data_in(39);
   lfsr_c(5) <= lfsr_q(0) xor lfsr_q(2) xor lfsr_q(4) xor data_in(2) xor data_in(5) xor data_in(6) xor data_in(10) xor data_in(12) xor data_in(13) xor data_in(14) xor data_in(16) xor data_in(18) xor data_in(19) xor data_in(21) xor data_in(22) xor data_in(28) xor data_in(29) xor data_in(32) xor data_in(33) xor data_in(35) xor data_in(37);
   lfsr_c(6) <= lfsr_q(0) xor lfsr_q(1) xor lfsr_q(3) xor lfsr_q(5) xor data_in(3) xor data_in(6) xor data_in(7) xor data_in(11) xor data_in(13) xor data_in(14) xor data_in(15) xor data_in(17) xor data_in(19) xor data_in(20) xor data_in(22) xor data_in(23) xor data_in(29) xor data_in(30) xor data_in(33) xor data_in(34) xor data_in(36) xor data_in(38);
   
   process(clk_i) begin
      if clk_i'event and clk_i = '1' then
         crc_en_tmp <= crc_en_i;
      end if;
   end process;
   
   crc_en_rise <= '1' when crc_en_tmp = '0' and crc_en_i = '1' else '0';
   crc_en_fall <= '1' when crc_en_tmp = '1' and crc_en_i = '0' else '0';
   
   process(clk_i) begin
      if clk_i'event and clk_i = '1' then
         if crc_en_rise = '1' then 
            lfsr_q <= (others => '0');
         else
            if crc_en_fall = '1' then
               lfsr_q <= lfsr_c;
            end if;
         end if;
      end if;
   end process;
    
end Behavioral;
