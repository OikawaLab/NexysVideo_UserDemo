----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    16:16:24 10/26/2011 
-- Design Name: 
-- Module Name:    dpti_basic_sync_performance_test - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
library UNISIM;
use UNISIM.VComponents.all;

entity dpti_basic_sync_performance_test is
    Port ( DPTI_FTCLK    : in  STD_LOGIC;
           DPTI_RXF      : in  STD_LOGIC;
           DPTI_TXE      : in  STD_LOGIC;
           DPTI_JTAGEN   : in  STD_LOGIC;
           DPTI_RD       : out  STD_LOGIC;
           DPTI_WR       : out  STD_LOGIC;
           DPTI_OE       : out  STD_LOGIC;
           DPTI_SIWU     : out STD_LOGIC;
           DPTI_PDB      : inout  STD_LOGIC_VECTOR (7 downto 0));
end dpti_basic_sync_performance_test;

architecture Behavioral of dpti_basic_sync_performance_test is

attribute keep : string;  

-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------

component fifo_buffer is
    Port ( DIN : in  STD_LOGIC_VECTOR (7 downto 0);
           FWR : in  STD_LOGIC;
           FRD : in  STD_LOGIC;
           CLK : in  STD_LOGIC;
           RST : in  STD_LOGIC;
           DOUT : out  STD_LOGIC_VECTOR (7 downto 0);
           FULL : out  STD_LOGIC;
           PFULL : out  STD_LOGIC;
           EMPTY : out  STD_LOGIC);
end component;

-------------------------------------------------------------------------------
-- Local Type Declarations
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Constant Declarations
-------------------------------------------------------------------------------

-- The following constants define state codes for the Synchronous PTI port
-- state machine. The high order bits of the state number provide a unique
-- state identifier for each state. The low order bits are the state machine
-- outputs for that state. This type of state machine implementation uses no
-- combinational logic to generate the outputs, which should result in glitch
-- free outputs. Please note that you MUST set the "FSM Encoding Algorithm" to
-- none for this to work.

--constant    stPtiRdy    : std_logic_vector(6 downto 0):= "00" & "01011";
--constant    stPtiOut0   : std_logic_vector(6 downto 0):= "01" & "01011";
--constant    stPtiOut1   : std_logic_vector(6 downto 0):= "01" & "00110";
--constant    stPtiIn0    : std_logic_vector(6 downto 0):= "10" & "11011";
--constant    stPtiIn1    : std_logic_vector(6 downto 0):= "10" & "11001";

--constant    stPtiRdy    : std_logic_vector(6 downto 0):= "00" & "01011";
--constant    stPtiOut0   : std_logic_vector(6 downto 0):= "01" & "01011";
--constant    stPtiOut1   : std_logic_vector(6 downto 0):= "01" & "00011";
--constant    stPtiOut2   : std_logic_vector(6 downto 0):= "01" & "00110";
--constant    stPtiIn0    : std_logic_vector(6 downto 0):= "10" & "01011";
--constant    stPtiIn1    : std_logic_vector(6 downto 0):= "10" & "11011";
--constant    stPtiIn2    : std_logic_vector(6 downto 0):= "10" & "11001";

--constant    stPtiRdy    : std_logic_vector(6 downto 0):= "00" & "01011";
--constant    stPtiOut0   : std_logic_vector(6 downto 0):= "01" & "01011";
--constant    stPtiOut1   : std_logic_vector(6 downto 0):= "01" & "00011";
--constant    stPtiOut2   : std_logic_vector(6 downto 0):= "01" & "00110";
--constant    stPtiIn0    : std_logic_vector(6 downto 0):= "10" & "01011";
--constant    stPtiIn1    : std_logic_vector(6 downto 0):= "10" & "11011";
--constant    stPtiIn2    : std_logic_vector(6 downto 0):= "10" & "11001";

constant    stPtiRdy    : std_logic_vector(8 downto 0):= "00" & "00" & "01011";
constant    stPtiOut0   : std_logic_vector(8 downto 0):= "01" & "00" & "01011";
constant    stPtiOut1   : std_logic_vector(8 downto 0):= "01" & "00" & "00011";
constant    stPtiOut2   : std_logic_vector(8 downto 0):= "01" & "00" & "00110";
constant    stPtiIn0    : std_logic_vector(8 downto 0):= "10" & "00" & "01011";
constant    stPtiIn1    : std_logic_vector(8 downto 0):= "10" & "00" & "11011";
constant    stPtiIn2    : std_logic_vector(8 downto 0):= "10" & "00" & "11001";

constant    stPtiInOut0 : std_logic_vector(8 downto 0):= "11" & "00" & "01011";
constant    stPtiInOut1 : std_logic_vector(8 downto 0):= "11" & "00" & "00011";
constant    stPtiInOut2 : std_logic_vector(8 downto 0):= "11" & "01" & "00010";
constant    stPtiInOut3 : std_logic_vector(8 downto 0):= "11" & "10" & "11011";
constant    stPtiInOut4 : std_logic_vector(8 downto 0):= "11" & "10" & "11001";

-------------------------------------------------------------------------------
-- Signal Declarations
-------------------------------------------------------------------------------

--signal  stPtiCur    : std_logic_vector(6 downto 0) := stPtiRdy;
--signal  stPtiNext   : std_logic_vector(6 downto 0);

signal  stPtiCur    : std_logic_vector(8 downto 0) := stPtiRdy;
signal  stPtiNext   : std_logic_vector(8 downto 0);

signal  clkPti  : std_logic;

-- Internal control signals
signal ctlRd    : std_logic;
signal ctlWr    : std_logic;
signal ctlDwr   : std_logic;
signal ctlOe    : std_logic;
signal ctlDir   : std_logic;

signal ctlRxf   : std_logic;
signal ctlTxe   : std_logic;
signal ctlRst   : std_logic;
signal ctlMd0   : std_logic;
signal ctlMd1   : std_logic;

signal ctlFull  : std_logic;
signal ctlPfull : std_logic;
signal ctlEmpty : std_logic;
signal ctlFwr   : std_logic;
signal ctlFrd   : std_logic;

signal busPtiTris   : std_logic;
signal busPtiOut    : std_logic_vector(7 downto 0);
signal busPtiOut_reg    : std_logic_vector(7 downto 0) := (others =>'0');

signal enOddrReg : std_logic;
signal oddrRegPreLd : std_logic := '0';

signal busPtiIn     : std_logic_vector(7 downto 0);
signal busFifoOut   : std_logic_vector(7 downto 0);

-- Registers
signal  regData : std_logic_vector(7 downto 0);

--attribute keep of busPtiIn: signal is "true";  
--attribute keep of busPtiOut: signal is "true"; 
attribute keep of ctlRd: signal is "true"; 
attribute keep of ctlWr: signal is "true";  
attribute keep of ctlOe: signal is "true"; 
attribute keep of ctlDir: signal is "true"; 
attribute keep of ctlRst: signal is "true"; 
attribute keep of ctlRxf: signal is "true"; 
attribute keep of ctlTxe: signal is "true"; 
attribute keep of ctlMd0: signal is "true"; 
attribute keep of ctlMd1: signal is "true"; 

-------------------------------------------------------------------------------
-- Module Implementation
-------------------------------------------------------------------------------

begin

ODDR_gen : for index in 0 to 7 generate
  ODDR_inst : ODDR
  generic map(
  DDR_CLK_EDGE => "SAME_EDGE", -- "OPPOSITE_EDGE" or "SAME_EDGE"
  INIT => '0', -- Initial value for Q port ('1' or '0')
  SRTYPE => "SYNC") -- Reset Type ("ASYNC" or "SYNC")
  port map (
  Q => busPtiOut_reg(index), -- 1-bit DDR output
  C => clkPti, -- 1-bit clock input
  CE => enOddrReg, -- 1-bit clock enable input
  D1 => busPtiOut(index), -- 1-bit data input (positive edge)
  D2 => busPtiOut(index), -- 1-bit data input (negative edge)
  R => '0', -- 1-bit reset input
  S => '0' -- 1-bit set input
  );
end generate;

enOddrReg <= ctlFrd;

process (clkPti, ctlRst)
begin
    if ctlRst = '1' then
      oddrRegPreLd <= '0';
    elsif clkPti = '1' and clkPti'Event then
      if (stPtiCur = stPtiInOut3) then
        oddrRegPreLd <= '1';
      elsif ((ctlEmpty = '1' and stPtiCur = stPtiInOut4) and ctlTxe = '0') then
        oddrRegPreLd <= '0';
      end if;
    end if;
end process;


-------------------------------------------------------------------------------
-- Map basic status and control signals
-------------------------------------------------------------------------------

DPTI_SIWU <= '1';

clkPti <= DPTI_FTCLK;
ctlRxf <= DPTI_RXF;
ctlTxe <= DPTI_TXE;
ctlRst <= DPTI_JTAGEN;
ctlMd0 <= '1';
ctlMd1 <= '1';

DPTI_WR <= ctlWr;
DPTI_RD <= ctlRd;
DPTI_OE <= ctlOe;

-- Data bus direction and control.

IOBUF_gen : for index in 0 to 7 generate
  IOBUF_inst : IOBUF
  generic map (
  DRIVE => 16,
  IOSTANDARD => "DEFAULT",
  SLEW => "FAST")
  port map (
  O => busPtiIn(index), -- Buffer output
  IO => DPTI_PDB(index), -- Buffer inout port (connect directly to top-level port)
  I => busPtiOut_reg(index), -- Buffer input
  T => busPtiTris -- 3-state enable input, high=input, low=output
  );
end generate;

busPtiTris <= not(ctlDir and not(ctlRst));

--busPtiIn <= pdb;
--pdb <= busPtiOut_reg when ctlDir = '1' and ctlRst = '0' else "ZZZZZZZZ";

-------------------------------------------------------------------------------
-- PTI State Machine
-------------------------------------------------------------------------------

-- Map control signals from the current state
ctlRd   <= stPtiCur(0) or ctlFull; --keep in mind: active low
ctlWr   <= stPtiCur(1) or not(oddrRegPreLd); --keep in mind: active low
ctlDwr  <= stPtiCur(2);
ctlOe   <= stPtiCur(3);
ctlDir  <= stPtiCur(4);
ctlFwr  <= stPtiCur(5) and not ctlRxf;
ctlFrd  <= stPtiCur(6) and not ctlTxe;

fifo_buffer_1 : fifo_buffer
    port map ( DIN => busPtiIn,
               FWR => ctlFwr,
               FRD => ctlFrd,
               CLK => clkPti,
               RST => ctlRst,
               DOUT => busFifoOut,
               FULL => ctlFull,
               PFULL => ctlPfull,
               EMPTY => ctlEmpty);

-- This process moves the state machine to the next state on each clock.
process (clkPti, ctlRst)
begin
    if ctlRst = '1' then
        stPtiCur <= stPtiRdy;
    elsif clkPti = '1' and clkPti'Event then
        stPtiCur <= stPtiNext;
    end if;
end process;

-- This process determines the next state based on the current state and the
-- state machine inputs.
process (stPtiCur, stPtiNext, ctlRxf, ctlTxe, ctlMd0, ctlMd1, ctlFull, ctlPfull, ctlEmpty, oddrRegPreLd)
begin
    case stPtiCur is
        when stPtiRdy =>
--            if ctlMd0 = '1' then
--                stPtiNext <= stPtiOut0;
--            elsif ctlMd1 = '1' then
--                stPtiNext <= stPtiIn0;
--            else
--                stPtiNext <= stPtiRdy;
--            end if;
            if    ctlMd1 = '0' and ctlMd0 = '0' then
                stPtiNext <= stPtiRdy;
            elsif ctlMd1 = '0' and ctlMd0 = '1' then
                stPtiNext <= stPtiOut0;
            elsif ctlMd1 = '1' and ctlMd0 = '0' then
                stPtiNext <= stPtiIn0;
            else
                stPtiNext <= stPtiInOut0;
            end if;
            
        when stPtiOut0 =>
            if ctlRxf = '0' then
                stPtiNext <= stPtiOut1;
            else
                stPtiNext <= stPtiOut0;
            end if;
            
--        when stPtiOut1 =>
--            if ctlMd1 = '1' then
--                stPtiNext <= stPtiIn0;
--            else
--                stPtiNext <= stPtiOut0;
--            end if;
        
        when stPtiOut1 =>
            stPtiNext <= stPtiOut2;
        
        when stPtiOut2 =>
            if ctlRxf = '1' then
                stPtiNext <= stPtiOut0;
            else
                stPtiNext <= stPtiOut2;
            end if;
        
--        when stPtiOut2 =>
--            if ctlMd1 = '1' then
--                stPtiNext <= stPtiIn0;
--            else
--                stPtiNext <= stPtiOut0;
--            end if;
            
        when stPtiIn0 =>
            if ctlTxe = '0' then
                stPtiNext <= stPtiIn1;
            else
                stPtiNext <= stPtiIn0;
            end if;
            
--        when stPtiIn1 =>
--            if ctlMd0 = '1' then
--                stPtiNext <= stPtiOut0;
--            else
--                stPtiNext <= stPtiIn0;
--            end if;
            
        when stPtiIn1 =>
            stPtiNext <= stPtiIn2;
            
--        when stPtiIn2 =>
--            if ctlMd0 = '1' then
--                stPtiNext <= stPtiOut0;
--            else
--                stPtiNext <= stPtiIn0;
--            end if;
        
        when stPtiIn2 =>
            if ctlTxe = '1' then
                stPtiNext <= stPtiIn0;
            else
                stPtiNext <= stPtiIn2;
            end if;
        
        when stPtiInOut0 =>
            if ctlRxf = '0' and ctlFull = '0' then
                stPtiNext <= stPtiInOut1;
            elsif ctlTxe = '0' and (ctlEmpty = '0' or oddrRegPreLd = '1') then
                if (oddrRegPreLd = '0') then
                  stPtiNext <= stPtiInout3;
                else
                  stPtiNext <= stPtiInout4;
                end if;
            else
                stPtiNext <= stPtiInOut0;
            end if;
            
        when stPtiInOut1 =>
            stPtiNext <= stPtiInOut2;
            
        when stPtiInOut2 =>
            if ctlRxf = '0' and ctlPfull = '0' then
                stPtiNext <= stPtiInOut2;
            else
                stPtiNext <= stPtiInOut0;
            end if;
            
        when stPtiInOut3 =>
            stPtiNext <= stPtiInOut4;
            
        when stPtiInOut4 =>
            --if ctlTxe = '0' and ctlEmpty = '0' then
            if ctlTxe = '0' and oddrRegPreLd = '1' then --wasted cycle in this state?
                stPtiNext <= stPtiInOut4;
            else
                stPtiNext <= stPtiInOut0;
            end if;
        
        when others =>
            stPtiNext <= stPtiRdy;
    end case;
end process;

-------------------------------------------------------------------------------
-- Data Registers
-------------------------------------------------------------------------------

process  (clkPti, ctlRst)
begin
    if ctlRst = '1' then
        regData <= "0100" & "1010";
    elsif clkPti = '1' and clkPti'Event then
        if ctlDwr = '1' then
            regData <= busPtiIn;
        end if;
    end if;
end process;

busPtiOut <= not busFifoOut when ctlMd0 = '1' and ctlMd1 = '1' else not regData;

--process (clkPti, ctlRst, DPTI_RXF)
--begin
--    if ctlRst = '1' then
--        ctlRxf <= '1';
--    elsif clkPti = '1' and clkPti'Event then
--        ctlRxf <= DPTI_RXF;
--    end if;
--end process;
--
--process (clkPti, ctlRst, DPTI_TXE)
--begin
--    if ctlRst = '1' then
--        ctlTxe <= '1';
--    elsif clkPti = '1' and clkPti'Event then
--        ctlTxe <= DPTI_TXE;
--    end if;
--end process;

end Behavioral;

