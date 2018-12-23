----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    09:57:25 10/31/2011 
-- Design Name: 
-- Module Name:    fifo_buffer - Behavioral 
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
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity fifo_buffer is
    Port ( DIN : in  STD_LOGIC_VECTOR (7 downto 0);
           FWR : in  STD_LOGIC;
           FRD : in  STD_LOGIC;
           CLK : in  STD_LOGIC;
           RST : in  STD_LOGIC;
           DOUT : out  STD_LOGIC_VECTOR (7 downto 0);
           FULL : out  STD_LOGIC;
           PFULL : out  STD_LOGIC;
           EMPTY : out  STD_LOGIC);
end fifo_buffer;

architecture Behavioral of fifo_buffer is

attribute keep : string;  
-------------------------------------------------------------------------------
-- Component Declarations
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Local Type Declarations
-------------------------------------------------------------------------------

type    RamType is array (0 to 4095) of bit_vector(7 downto 0);

-------------------------------------------------------------------------------
-- Constant Declarations
-------------------------------------------------------------------------------

constant  cbFifoMax   : std_logic_vector(12 downto 0) := "1" & "0000" & "0000" & "0000";
constant  cbFifoMin   : std_logic_vector(12 downto 0) := "0" & "0000" & "0000" & "0000";

-------------------------------------------------------------------------------
-- Signal Declarations
-------------------------------------------------------------------------------

-- Internal Control Signals
signal  ctlWr       : std_logic;
signal  ctlRd       : std_logic;
signal  ctlFull     : std_logic;
signal  ctlEmpty    : std_logic;
signal  ctlRst      : std_logic;
signal  clkFifo     : std_logic;

--
signal  busDin  : std_logic_vector(7 downto 0);
signal  busDout : std_logic_vector(7 downto 0);

-- 
signal  ptrWrite : std_logic_vector(11 downto 0):="0000" & "0000" & "0000";
signal  PtrRead : std_logic_vector(11 downto 0) := "0000" & "0000" & "0000";
signal  cbFifo : std_logic_vector(12 downto 0) := cbFifoMin;


-- Registers
signal  ramData : RamType;

attribute WRITE_MODE : string;
attribute WRITE_MODE of ramData : signal is "NO_CHANGE";


attribute keep of PtrRead: signal is "true"; 
attribute keep of ctlRd: signal is "true"; 
--attribute keep of busDin: signal is "true"; 

-------------------------------------------------------------------------------
-- Module Implementation
-------------------------------------------------------------------------------


begin

-------------------------------------------------------------------------------
-- Map basic status and control signals
-------------------------------------------------------------------------------

clkFifo <= CLK;
ctlWr <= FWR;
ctlRd <= FRD;
ctlRst <= RST;
busDin <= DIN;

-- Implement logic required to output the Empty flag
process (cbFifo)
begin
    if cbFifo = cbFifoMin then
        ctlEmpty <= '1';
    else
        ctlEmpty <= '0';
    end if;
end process;

EMPTY <= ctlEmpty;


-- Implement logic required to output the FULL flag
process (cbFifo)
begin
    if cbFifo = cbFifoMax then
        ctlFull <= '1';
    else
        ctlFull <= '0';
    end if;
end process;

FULL <= ctlFull;


-- Implement logic required to output the PFULL flag
process (cbFifo)
begin
    if cbFifo + 1 = cbFifoMax then
        PFULL <= '1';
    else
        PFULL <= '0';
    end if;
end process;


-- Implement the byte counter
process (clkFifo, ctlRst)
begin
    if ctlRst = '1' then
        cbFifo <= "0" & "0000" & "0000" & "0000";
    elsif clkFifo = '1' and clkFifo'Event then
        if ctlWr = '1' and ctlRd = '0' then
            if ctlFull = '0' then
                cbFifo <= cbFifo + 1;
            end if;
        elsif ctlWr = '0' and ctlRd = '1' then
            if ctlEmpty = '0' then
                cbFifo <= cbFifo - 1;
            end if;
        end if;
    end if;
end process;


-- Implement write pointer/counter.
process (clkFifo, ctlRst)
begin
    if ctlRst = '1' then
        ptrWrite <= "0000" & "0000" & "0000";
    elsif clkFifo = '1' and clkFifo'Event then
        if ctlWr = '1' and ctlFull = '0' then
            ptrWrite <= ptrWrite + 1;
        end if;
    end if;
end process;


-- Implement read pointer/counter.
process (clkFifo, ctlRst)
begin
    if ctlRst = '1' then
        ptrRead <= "0000" & "0000" & "0000";
    elsif clkFifo = '1' and clkFifo'Event then
        if ctlRd = '1' and ctlEmpty = '0' then
            ptrRead <= ptrRead + 1;
        end if;
    end if;
end process;


-- Implement FIFO buffer.
process (clkFifo, ctlRst)
begin 
    if clkFifo = '1' and clkFifo'Event then
        if ctlWr = '1' then
            ramData(conv_integer(ptrWrite)) <= to_bitvector(busDin);
        end if;
        --busDout <= to_stdlogicvector(ramData(conv_integer(ptrRead)));
--        if ctlRd = '1' then
--            busDout <= to_stdlogicvector(ramData(conv_integer(ptrRead)));
--        end if;
    end if;
end process;

--DOUT <= busDout;
DOUT <= to_stdlogicvector(ramData(conv_integer(ptrRead)));

end Behavioral;

