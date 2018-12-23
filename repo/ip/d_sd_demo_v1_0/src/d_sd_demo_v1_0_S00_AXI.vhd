library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity d_sd_demo_v1_0_S00_AXI is
	generic (
		-- Users to add parameters here

		-- User parameters ends
		-- Do not modify the parameters beyond this line

		-- Width of S_AXI data bus
		C_S_AXI_DATA_WIDTH	: integer	:= 32;
		-- Width of S_AXI address bus
		C_S_AXI_ADDR_WIDTH	: integer	:= 6
	);
	port (
		-- Users to add ports here
      -- sd native interface
      sd_sck_o : out std_logic;
      sd_cmd_i : in std_logic;
      sd_cmd_t : out std_logic;
      sd_cmd_o : out std_logic;
      sd_dat0_i : in std_logic;
      sd_dat1_i : in std_logic;
      sd_dat2_i : in std_logic;
      sd_dat3_i : in std_logic;
		-- User ports ends
		-- Do not modify the ports beyond this line

		-- Global Clock Signal
		S_AXI_ACLK	: in std_logic;
		-- Global Reset Signal. This Signal is Active LOW
		S_AXI_ARESETN	: in std_logic;
		-- Write address (issued by master, acceped by Slave)
		S_AXI_AWADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		-- Write channel Protection type. This signal indicates the
    		-- privilege and security level of the transaction, and whether
    		-- the transaction is a data access or an instruction access.
		S_AXI_AWPROT	: in std_logic_vector(2 downto 0);
		-- Write address valid. This signal indicates that the master signaling
    		-- valid write address and control information.
		S_AXI_AWVALID	: in std_logic;
		-- Write address ready. This signal indicates that the slave is ready
    		-- to accept an address and associated control signals.
		S_AXI_AWREADY	: out std_logic;
		-- Write data (issued by master, acceped by Slave) 
		S_AXI_WDATA	: in std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		-- Write strobes. This signal indicates which byte lanes hold
    		-- valid data. There is one write strobe bit for each eight
    		-- bits of the write data bus.    
		S_AXI_WSTRB	: in std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
		-- Write valid. This signal indicates that valid write
    		-- data and strobes are available.
		S_AXI_WVALID	: in std_logic;
		-- Write ready. This signal indicates that the slave
    		-- can accept the write data.
		S_AXI_WREADY	: out std_logic;
		-- Write response. This signal indicates the status
    		-- of the write transaction.
		S_AXI_BRESP	: out std_logic_vector(1 downto 0);
		-- Write response valid. This signal indicates that the channel
    		-- is signaling a valid write response.
		S_AXI_BVALID	: out std_logic;
		-- Response ready. This signal indicates that the master
    		-- can accept a write response.
		S_AXI_BREADY	: in std_logic;
		-- Read address (issued by master, acceped by Slave)
		S_AXI_ARADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		-- Protection type. This signal indicates the privilege
    		-- and security level of the transaction, and whether the
    		-- transaction is a data access or an instruction access.
		S_AXI_ARPROT	: in std_logic_vector(2 downto 0);
		-- Read address valid. This signal indicates that the channel
    		-- is signaling valid read address and control information.
		S_AXI_ARVALID	: in std_logic;
		-- Read address ready. This signal indicates that the slave is
    		-- ready to accept an address and associated control signals.
		S_AXI_ARREADY	: out std_logic;
		-- Read data (issued by slave)
		S_AXI_RDATA	: out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		-- Read response. This signal indicates the status of the
    		-- read transfer.
		S_AXI_RRESP	: out std_logic_vector(1 downto 0);
		-- Read valid. This signal indicates that the channel is
    		-- signaling the required read data.
		S_AXI_RVALID	: out std_logic;
		-- Read ready. This signal indicates that the master can
    		-- accept the read data and response information.
		S_AXI_RREADY	: in std_logic
	);
end d_sd_demo_v1_0_S00_AXI;

architecture arch_imp of d_sd_demo_v1_0_S00_AXI is

component cmd_rxtx is
   port(
      -- global signals
      clk_i             : in  std_logic;
      rst_i             : in  std_logic;
      -- clock divider value
      sck_div_i         : in  std_logic_vector(2 downto 0);
      sck_en_i          : in  std_logic;
      -- command control/flag signals
      cmd_index_i       : in  std_logic_vector(7 downto 0);
      cmd_arg_i         : in  std_logic_vector(31 downto 0);
      cmd_send_i        : in  std_logic;
      cmd_done_o        : out std_logic;
      -- debug
      dbg_crc0_decoded  : out std_logic_vector(15 downto 0);
      dbg_crc1_decoded  : out std_logic_vector(15 downto 0);
      dbg_crc2_decoded  : out std_logic_vector(15 downto 0);
      dbg_crc3_decoded  : out std_logic_vector(15 downto 0);
      dbg_crc0_actual   : out std_logic_vector(15 downto 0);
      dbg_crc1_actual   : out std_logic_vector(15 downto 0);
      dbg_crc2_actual   : out std_logic_vector(15 downto 0);
      dbg_crc3_actual   : out std_logic_vector(15 downto 0);
      -- response control/flag signals
      resp_type_i       : in  std_logic_vector(1 downto 0);
      resp_data_o       : out std_logic_vector(125 downto 0);
      resp_done_o       : out std_logic;
      resp_timeout_o    : out std_logic;
      -- data control/flag signals
      data_transfer_i   : in  std_logic;
      data_crc0_err_o   : out std_logic;
      data_crc1_err_o   : out std_logic;
      data_crc2_err_o   : out std_logic;
      data_crc3_err_o   : out std_logic;
      data_zero0_o      : out std_logic;
      data_zero1_o      : out std_logic;
      data_zero2_o      : out std_logic;
      data_zero3_o      : out std_logic;
      data_done_o       : out std_logic;
      -- sd interface
      sd_sck_o          : out std_logic;
      sd_cmd_i          : in  std_logic;
      sd_cmd_t          : out std_logic;
      sd_cmd_o          : out std_logic;
      sd_dat0_i         : in  std_logic;
      sd_dat1_i         : in  std_logic;
      sd_dat2_i         : in  std_logic;
      sd_dat3_i         : in  std_logic);
end component;

	-- AXI4LITE signals
	signal axi_awaddr	: std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
	signal axi_awready	: std_logic;
	signal axi_wready	: std_logic;
	signal axi_bresp	: std_logic_vector(1 downto 0);
	signal axi_bvalid	: std_logic;
	signal axi_araddr	: std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
	signal axi_arready	: std_logic;
	signal axi_rdata	: std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal axi_rresp	: std_logic_vector(1 downto 0);
	signal axi_rvalid	: std_logic;

	-- Example-specific design signals
	-- local parameter for addressing 32 bit / 64 bit C_S_AXI_DATA_WIDTH
	-- ADDR_LSB is used for addressing 32/64 bit registers/memories
	-- ADDR_LSB = 2 for 32 bits (n downto 2)
	-- ADDR_LSB = 3 for 64 bits (n downto 3)
	constant ADDR_LSB  : integer := (C_S_AXI_DATA_WIDTH/32)+ 1;
	constant OPT_MEM_ADDR_BITS : integer := 3;
	------------------------------------------------
	---- Signals for user logic register space example
	--------------------------------------------------
	---- Number of Slave Registers 12
	signal COMMAND_REG : std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal ARGUMENT_REG : std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal STATUS_REG : std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal RESPONSE_REG0 : std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal RESPONSE_REG1 : std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal RESPONSE_REG2 : std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal RESPONSE_REG3 : std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal DBG_REG0 : std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal DBG_REG1 : std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal DBG_REG2 : std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal DBG_REG3 : std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg11 : std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg_rden : std_logic;
	signal slv_reg_wren : std_logic;
	signal reg_data_out : std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal byte_index	: integer;
	
   signal resp_timeout, resp_done, resp_en : std_logic;
   signal resp_data : std_logic_vector(125 downto 0);
   signal resp_type : std_logic_vector(1 downto 0);
   signal cmd_done, cmd_send : std_logic;
   signal cmd_arg : std_logic_vector(31 downto 0);
   signal cmd_index : std_logic_vector(7 downto 0);
   signal sck_en : std_logic;
   signal sck_div : std_logic_vector(2 downto 0);
   signal rst : std_logic;
   signal data_transfer : std_logic;
   signal data_crc0_err, data_crc1_err, data_crc2_err, data_crc3_err : std_logic;
   signal data_zero0, data_zero1, data_zero2, data_zero3 : std_logic;
   signal data_done : std_logic;
   signal dbg_crc3_decoded, dbg_crc2_decoded, dbg_crc1_decoded, dbg_crc0_decoded : std_logic_vector(15 downto 0);
   signal dbg_crc3_actual, dbg_crc2_actual, dbg_crc1_actual, dbg_crc0_actual : std_logic_vector(15 downto 0);

begin
	
   rst            <= not S_AXI_ARESETN;
   sck_en         <= COMMAND_REG(0);
   sck_div        <= COMMAND_REG(3 downto 1);
   resp_type      <= COMMAND_REG(5 downto 4);
   cmd_send       <= '1' when slv_reg_wren = '1' and axi_awaddr(ADDR_LSB + OPT_MEM_ADDR_BITS downto ADDR_LSB) = "0001" else '0';
   data_transfer  <= COMMAND_REG(7);
   cmd_index      <= COMMAND_REG(15 downto 8);
   cmd_arg        <= ARGUMENT_REG;

    STATUS_REG(31 downto 18) <= (others => '0');
    STATUS_REG(17) <= data_crc3_err;
    STATUS_REG(16) <= data_crc2_err;
    STATUS_REG(15) <= data_crc1_err;
    STATUS_REG(14) <= data_crc0_err;
    STATUS_REG(13) <= data_zero3;
    STATUS_REG(12) <= data_zero2;
    STATUS_REG(11) <= data_zero1;
    STATUS_REG(10) <= data_zero0;
    STATUS_REG(9)  <= '0';
    STATUS_REG(8)  <= data_done;
    STATUS_REG(7)  <= '0';
    STATUS_REG(6)  <= resp_timeout;
    STATUS_REG(5)  <= '0';
    STATUS_REG(4)  <= resp_done;
    STATUS_REG(3 downto 1) <= (others => '0');
    STATUS_REG(0)  <= cmd_done;
    RESPONSE_REG0 <= resp_data(31 downto 0);
    RESPONSE_REG1 <= resp_data(63 downto 32);
    RESPONSE_REG2 <= resp_data(95 downto 64);
    RESPONSE_REG3(31 downto 30) <= (others => '0');
    RESPONSE_REG3(29 downto 0) <= resp_data(125 downto 96);
    DBG_REG0(31 downto 16) <= dbg_crc0_actual;
    DBG_REG0(15 downto 0) <= dbg_crc0_decoded;
    DBG_REG1(31 downto 16) <= dbg_crc1_actual;
    DBG_REG1(15 downto 0) <= dbg_crc1_decoded;
    DBG_REG2(31 downto 16) <= dbg_crc2_actual;
    DBG_REG2(15 downto 0) <= dbg_crc2_decoded;
    DBG_REG3(31 downto 16) <= dbg_crc3_actual;
    DBG_REG3(15 downto 0) <= dbg_crc3_decoded;
   
   COMMAND: cmd_rxtx
   port map(
      clk_i             => S_AXI_ACLK,
      rst_i             => rst,
      sck_div_i         => sck_div, -- 3 bit
      sck_en_i          => sck_en,
      cmd_index_i       => cmd_index, -- 8 bit
      cmd_arg_i         => cmd_arg, -- 32 bit
      cmd_send_i        => cmd_send,
      cmd_done_o        => cmd_done,
      dbg_crc0_decoded  => dbg_crc0_decoded,
      dbg_crc1_decoded  => dbg_crc1_decoded,
      dbg_crc2_decoded  => dbg_crc2_decoded,
      dbg_crc3_decoded  => dbg_crc3_decoded,
      dbg_crc0_actual   => dbg_crc0_actual,
      dbg_crc1_actual   => dbg_crc1_actual,
      dbg_crc2_actual   => dbg_crc2_actual,
      dbg_crc3_actual   => dbg_crc3_actual,
      resp_type_i       => resp_type, -- 2 bit
      resp_data_o       => resp_data, -- 126 bit
      resp_done_o       => resp_done,
      resp_timeout_o    => resp_timeout,
      data_transfer_i   => data_transfer,
      data_crc0_err_o   => data_crc0_err,
      data_crc1_err_o   => data_crc1_err,
      data_crc2_err_o   => data_crc2_err,
      data_crc3_err_o   => data_crc3_err,
      data_zero0_o      => data_zero0,
      data_zero1_o      => data_zero1,
      data_zero2_o      => data_zero2,
      data_zero3_o      => data_zero3,
      data_done_o       => data_done, 
      sd_sck_o          => sd_sck_o,
      sd_cmd_i          => sd_cmd_i,
      sd_cmd_t          => sd_cmd_t,
      sd_cmd_o          => sd_cmd_o,
      sd_dat0_i         => sd_dat0_i,
      sd_dat1_i         => sd_dat1_i,
      sd_dat2_i         => sd_dat2_i,
      sd_dat3_i         => sd_dat3_i);

	-- I/O Connections assignments
	S_AXI_AWREADY <= axi_awready;
	S_AXI_WREADY <= axi_wready;
	S_AXI_BRESP <= axi_bresp;
	S_AXI_BVALID <= axi_bvalid;
	S_AXI_ARREADY <= axi_arready;
	S_AXI_RDATA <= axi_rdata;
	S_AXI_RRESP <= axi_rresp;
	S_AXI_RVALID <= axi_rvalid;
	
	-- Implement axi_awready generation
	-- axi_awready is asserted for one S_AXI_ACLK clock cycle when both
	-- S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_awready is
	-- de-asserted when reset is low.
	process(S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_awready <= '0';
	    else
	      if (axi_awready = '0' and S_AXI_AWVALID = '1' and S_AXI_WVALID = '1') then
	        axi_awready <= '1';
	      else
	        axi_awready <= '0';
	      end if;
	    end if;
	  end if;
	end process;

	-- Implement axi_awaddr latching
	-- This process is used to latch the address when both 
	-- S_AXI_AWVALID and S_AXI_WVALID are valid. 
	process(S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_awaddr <= (others => '0');
	    else
	      if (axi_awready = '0' and S_AXI_AWVALID = '1' and S_AXI_WVALID = '1') then
	        -- Write Address latching
	        axi_awaddr <= S_AXI_AWADDR;
	      end if;
	    end if;
	  end if;                   
	end process; 

	-- Implement axi_wready generation
	-- axi_wready is asserted for one S_AXI_ACLK clock cycle when both
	-- S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_wready is 
	-- de-asserted when reset is low. 
	process(S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_wready <= '0';
	    else
	      if (axi_wready = '0' and S_AXI_WVALID = '1' and S_AXI_AWVALID = '1') then           
	        axi_wready <= '1';
	      else
	        axi_wready <= '0';
	      end if;
	    end if;
	  end if;
	end process; 

	-- Implement memory mapped register select and write logic generation
	-- The write data is accepted and written to memory mapped registers when
	-- axi_awready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted. Write strobes are used to
	-- select byte enables of slave registers while writing.
	-- These registers are cleared when reset (active low) is applied.
	-- Slave register write enable is asserted when valid address and data are available
	-- and the slave is ready to accept the write address and write data.
	slv_reg_wren <= axi_wready and S_AXI_WVALID and axi_awready and S_AXI_AWVALID;

	process(S_AXI_ACLK)
	variable loc_addr : std_logic_vector(OPT_MEM_ADDR_BITS downto 0); 
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      COMMAND_REG <= (others => '0');
	      ARGUMENT_REG <= (others => '0');
	      --STATUS_REG <= (others => '0');
	      --RESPONSE_REG0 <= (others => '0');
	      --RESPONSE_REG1 <= (others => '0');
	      --RESPONSE_REG2 <= (others => '0');
	      --RESPONSE_REG3 <= (others => '0');
	      --DBG_REG0 <= (others => '0');
	      --DBG_REG1 <= (others => '0');
	      --DBG_REG2 <= (others => '0');
	      --DBG_REG3 <= (others => '0');
	      slv_reg11 <= (others => '0');
	    else
	      loc_addr := axi_awaddr(ADDR_LSB + OPT_MEM_ADDR_BITS downto ADDR_LSB);
	      if (slv_reg_wren = '1') then
	        case loc_addr is
	          when b"0000" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                COMMAND_REG(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"0001" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                ARGUMENT_REG(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"1011" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 11
	                slv_reg11(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when others => null;
	        end case;
	      end if;
	    end if;
	  end if;                   
	end process; 

	-- Implement write response logic generation
	-- The write response and response valid signals are asserted by the slave 
	-- when axi_wready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted.  
	-- This marks the acceptance of address and indicates the status of 
	-- write transaction.
	process(S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_bvalid <= '0';
	      axi_bresp <= "00"; --need to work more on the responses
	    else
	      if (axi_awready = '1' and S_AXI_AWVALID = '1' and axi_wready = '1' and S_AXI_WVALID = '1' and axi_bvalid = '0'  ) then
	        axi_bvalid <= '1';
	        axi_bresp  <= "00"; 
	      elsif (S_AXI_BREADY = '1' and axi_bvalid = '1') then   --check if bready is asserted while bvalid is high)
	        axi_bvalid <= '0';                                 -- (there is a possibility that bready is always asserted high)
	      end if;
	    end if;
	  end if;                   
	end process; 

	-- Implement axi_arready generation
	-- axi_arready is asserted for one S_AXI_ACLK clock cycle when
	-- S_AXI_ARVALID is asserted. axi_awready is 
	-- de-asserted when reset (active low) is asserted. 
	-- The read address is also latched when S_AXI_ARVALID is 
	-- asserted. axi_araddr is reset to zero on reset assertion.
	process(S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_arready <= '0';
	      axi_araddr <= (others => '1');
	    else
	      if (axi_arready = '0' and S_AXI_ARVALID = '1') then
	        -- indicates that the slave has acceped the valid read address
	        axi_arready <= '1';
	        -- Read Address latching 
	        axi_araddr  <= S_AXI_ARADDR;           
	      else
	        axi_arready <= '0';
	      end if;
	    end if;
	  end if;                   
	end process; 

	-- Implement axi_arvalid generation
	-- axi_rvalid is asserted for one S_AXI_ACLK clock cycle when both 
	-- S_AXI_ARVALID and axi_arready are asserted. The slave registers 
	-- data are available on the axi_rdata bus at this instance. The 
	-- assertion of axi_rvalid marks the validity of read data on the 
	-- bus and axi_rresp indicates the status of read transaction.axi_rvalid 
	-- is deasserted on reset (active low). axi_rresp and axi_rdata are 
	-- cleared to zero on reset (active low).  
	process(S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then
	    if S_AXI_ARESETN = '0' then
	      axi_rvalid <= '0';
	      axi_rresp <= "00";
	    else
	      if (axi_arready = '1' and S_AXI_ARVALID = '1' and axi_rvalid = '0') then
	        -- Valid read data is available at the read data bus
	        axi_rvalid <= '1';
	        axi_rresp <= "00"; -- 'OKAY' response
	      elsif (axi_rvalid = '1' and S_AXI_RREADY = '1') then
	        -- Read data is accepted by the master
	        axi_rvalid <= '0';
	      end if;            
	    end if;
	  end if;
	end process;

	-- Implement memory mapped register select and read logic generation
	-- Slave register read enable is asserted when valid address is available
	-- and the slave is ready to accept the read address.
	slv_reg_rden <= axi_arready and S_AXI_ARVALID and (not axi_rvalid) ;

	process (COMMAND_REG, ARGUMENT_REG, STATUS_REG, RESPONSE_REG0, RESPONSE_REG1, 
	RESPONSE_REG2, RESPONSE_REG3, DBG_REG0, DBG_REG1, DBG_REG2, DBG_REG3, slv_reg11, 
	axi_araddr, S_AXI_ARESETN, slv_reg_rden)
	variable loc_addr : std_logic_vector(OPT_MEM_ADDR_BITS downto 0);
	begin
	  if S_AXI_ARESETN = '0' then
	    reg_data_out  <= (others => '1');
	  else
	    -- Address decoding for reading registers
	    loc_addr := axi_araddr(ADDR_LSB + OPT_MEM_ADDR_BITS downto ADDR_LSB);
	    case loc_addr is
	      when b"0000" => reg_data_out <= COMMAND_REG;
	      when b"0001" => reg_data_out <= ARGUMENT_REG;
	      when b"0010" => reg_data_out <= STATUS_REG;
	      when b"0011" => reg_data_out <= RESPONSE_REG0;
	      when b"0100" => reg_data_out <= RESPONSE_REG1;
	      when b"0101" => reg_data_out <= RESPONSE_REG2;
	      when b"0110" => reg_data_out <= RESPONSE_REG3;
	      when b"0111" => reg_data_out <= DBG_REG0;
	      when b"1000" => reg_data_out <= DBG_REG1;
	      when b"1001" => reg_data_out <= DBG_REG2;
	      when b"1010" => reg_data_out <= DBG_REG3;
	      when b"1011" => reg_data_out <= slv_reg11;
	      when others => reg_data_out  <= (others => '0');
	    end case;
	  end if;
	end process; 

	-- Output register or memory read data
	process( S_AXI_ACLK ) is
	begin
	  if (rising_edge (S_AXI_ACLK)) then
	    if ( S_AXI_ARESETN = '0' ) then
	      axi_rdata  <= (others => '0');
	    else
	      if (slv_reg_rden = '1') then
	          axi_rdata <= reg_data_out;     -- register read data
	      end if;   
	    end if;
	  end if;
	end process;


	-- Add user logic here

	-- User logic ends

end arch_imp;
