-------------------------------------------------------------------------------
--
-- File: vid_io_mux.vhd
-- Author: Elod Gyorgy
-- Original Project: Atlys2 User Demo 
-- Date: 3 December 2014
--
-------------------------------------------------------------------------------
-- (c) 2014 Copyright Digilent Incorporated
-- All Rights Reserved
-- 
-- This program is free software; distributed under the terms of BSD 3-clause 
-- license ("Revised BSD License", "New BSD License", or "Modified BSD License")
--
-- Redistribution and use in source and binary forms, with or without modification,
-- are permitted provided that the following conditions are met:
--
-- 1. Redistributions of source code must retain the above copyright notice, this
--    list of conditions and the following disclaimer.
-- 2. Redistributions in binary form must reproduce the above copyright notice,
--    this list of conditions and the following disclaimer in the documentation
--    and/or other materials provided with the distribution.
-- 3. Neither the name(s) of the above-listed copyright holder(s) nor the names
--    of its contributors may be used to endorse or promote products derived
--    from this software without specific prior written permission.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
-- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
-- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
-- ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE 
-- FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
-- DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
-- SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
-- CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
-- OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
-- OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--
-------------------------------------------------------------------------------
--
-- Purpose:
--    This module selects between two vid_io interfaces and their respective
--    clocks. It outputs video input 0, if aSel is low. It outputs video input 1,
--    if aSel is high. The outputs are registered on the output clock.
--  
-------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
library UNISIM;
use UNISIM.VComponents.all;

entity vid_io_mux is
	Generic (
      kDataWidth           : integer   := 24 -- Data width
   );
   Port (
      -- RGB interface input 0
      PixelClk_0     : in  std_logic;
      p0Data         : in  std_logic_vector((kDataWidth-1) downto 0);
      p0HSync        : in  std_logic;
      p0VSync        : in  std_logic;
      p0Vde          : in  std_logic;
      
      -- RGB interface input 1
      PixelClk_1     : in  std_logic;
      p1Data         : in  std_logic_vector((kDataWidth-1) downto 0);
      p1HSync        : in  std_logic;
      p1VSync        : in  std_logic;
      p1Vde          : in  std_logic;
      
      -- RGB interface output
      PixelClk       : out std_logic;
      pData          : out std_logic_vector((kDataWidth-1) downto 0);
      pHSync         : out std_logic;
      pVSync         : out std_logic;
      pVde           : out std_logic;
      
      aSel           : in std_logic
   );
end vid_io_mux;

architecture Behavioral of vid_io_mux is
COMPONENT fifo_generator_0
  PORT (
    rst : IN STD_LOGIC;
    wr_clk : IN STD_LOGIC;
    rd_clk : IN STD_LOGIC;
    din : IN STD_LOGIC_VECTOR(26 DOWNTO 0);
    wr_en : IN STD_LOGIC;
    rd_en : IN STD_LOGIC;
    dout : OUT STD_LOGIC_VECTOR(26 DOWNTO 0);
    full : OUT STD_LOGIC;
    empty : OUT STD_LOGIC;
    prog_empty : OUT STD_LOGIC
  );
END COMPONENT;
signal PixelClk_int, pSel, pProgEmpty0, pProgEmpty1, pRdEn0, pRdEn1, pSel_q, pProgEmpty0_q, pProgEmpty1_q : std_logic;
signal p0FifoDataIn, pFifoDataOut0, p1FifoDataIn, pFifoDataOut1 : std_logic_vector(kDataWidth+3-1 downto 0);
begin

-- Pixel clock Mux
-- IGNORE1 set to '1', because PixelClk_1 is the recovered clock from the DVI Input
-- and might disappear
PixelClkMux: BUFGCTRL
   generic map (
      INIT_OUT => 0, -- Initial value of BUFGCTRL output ($VALUES;)
      PRESELECT_I0 => TRUE, -- BUFGCTRL output uses I0 input ($VALUES;)
      PRESELECT_I1 => FALSE -- BUFGCTRL output uses I1 input ($VALUES;)
   )
   port map (
      O => PixelClk_int, -- 1-bit output: Clock output
      CE0 => '1', -- 1-bit input: Clock enable input for I0
      CE1 => '1', -- 1-bit input: Clock enable input for I1
      I0 => PixelClk_0, -- 1-bit input: Primary clock
      I1 => PixelClk_1, -- 1-bit input: Secondary clock
      IGNORE0 => '1', -- 1-bit input: Clock ignore input for I0
      IGNORE1 => '1', -- 1-bit input: Clock ignore input for I1 
      S0 => not aSel, -- 1-bit input: Clock select for I0
      S1 => aSel -- 1-bit input: Clock select for I1
   );
   
PixelClk <= PixelClk_int;

-- Sync aSel signal into the PixelClk_int (BUFGMUX output) domain
SyncAsyncx: entity work.SyncAsync
   generic map (
      kResetTo => '0',
      kStages => 2) --use double FF synchronizer
   port map (
      aReset => '0',
      aIn => aSel,
      OutClk => PixelClk_int,
      oOut => pSel);

-- Use asynchronous FIFO to cross data through input clock - output clock boundary
InputFIFO0 : fifo_generator_0
  PORT MAP (
    rst => aSel, -- FIFO0 is in reset when input 1 is selected (aSel = 1)
    wr_clk => PixelClk_0,
    rd_clk => PixelClk_int,
    din => p0FifoDataIn,
    wr_en => '1',
    rd_en => pRdEn0,
    dout => pFifoDataOut0,
    full => open,
    empty => open,
    prog_empty => pProgEmpty0
  );
InputFIFO1 : fifo_generator_0
    PORT MAP (
      rst => not aSel, -- FIFO1 is in reset when input 0 is selected (aSel = 0)
      wr_clk => PixelClk_1,
      rd_clk => PixelClk_int,
      din => p1FifoDataIn,
      wr_en => '1',
      rd_en => pRdEn1,
      dout => pFifoDataOut1,
      full => open,
      empty => open,
      prog_empty => pProgEmpty1
    );  
  p0FifoDataIn <= p0Data & p0HSync & p0VSync & p0Vde;
  p1FifoDataIn <= p1Data & p1HSync & p1VSync & p1Vde;
  
-- Synchro and component signals Mux
RegOutputs: process (PixelClk_int)
begin
   if Rising_Edge(PixelClk_int) then
      if (pSel = '0') then
         pData    <= pFifoDataOut0(kDataWidth+3-1 downto 3);
         pHSync   <= pFifoDataOut0(2);
         pVSync   <= pFifoDataOut0(1);
         pVde     <= pFifoDataOut0(0);
      else 
         pData    <= pFifoDataOut1(kDataWidth+3-1 downto 3);
         pHSync   <= pFifoDataOut1(2);
         pVSync   <= pFifoDataOut1(1);
         pVde     <= pFifoDataOut1(0);
      end if;
      
      if (pSel_q /= pSel) then
         pRdEn0 <= '0';
      elsif (pProgEmpty0_q = '1' and pProgEmpty0 = '0') then
         pRdEn0 <= '1';
      end if;
      
      if (pSel_q /= pSel) then
         pRdEn1 <= '0';
      elsif (pProgEmpty1_q = '1' and pProgEmpty1 = '0') then
         pRdEn1 <= '1';
      end if;
      
      pSel_q <= pSel;
      pProgEmpty0_q <= pProgEmpty0;
      pProgEmpty1_q <= pProgEmpty1;
   end if;
end process RegOutputs;

end Behavioral;
