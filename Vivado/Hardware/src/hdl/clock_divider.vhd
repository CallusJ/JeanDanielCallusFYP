library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;


entity clock_divider is
    Port (sck_in_clk :in std_logic ; 
          sck_en     :in std_logic ; 
          i2s_clk_en :in std_logic ;
          sck        : out std_logic ;
          lrck       : out std_logic ; 
          bck        : out std_logic );
end clock_divider;

architecture Behavioral of clock_divider is

COMPONENT c_counter_binary_3
  PORT (
    CLK : IN STD_LOGIC;
    CE : IN STD_LOGIC;
    Q : OUT STD_LOGIC_VECTOR(3 DOWNTO 0)
  );
END COMPONENT;

COMPONENT c_counter_binary_1
  PORT (
    CLK : IN STD_LOGIC;
    CE : IN STD_LOGIC;
    Q : OUT STD_LOGIC_VECTOR(1 DOWNTO 0)
  );
END COMPONENT;

COMPONENT c_counter_binary_2
  PORT (
    CLK : IN STD_LOGIC;
    Q : OUT STD_LOGIC_VECTOR(6 DOWNTO 0)
  );
END COMPONENT;


signal sck_in_q     : std_Logic_vector(3 downto 0);
signal bck_in_q     : std_logic_vector(1 downto 0);
signal lrck_in_q    : std_logic_vector(6 downto 0);

signal sck_sig : std_logic ;
signal bck_sig : std_logic ; 
signal lrck_sig : std_logic ; 

begin
sck_sig <= sck_in_q(3);
bck_sig <= bck_in_q(0);
lrck_sig <= lrck_in_q(6);

sck <= sck_sig ; 
bck <= bck_sig;
lrck <= lrck_sig;


ctr_0 : c_counter_binary_3
  PORT MAP (
    CLK => sck_in_clk,
    CE => sck_en,
    Q => sck_in_q
);

ctr_1 : c_counter_binary_1
  PORT MAP (
    CLK => sck_sig,
    CE => i2s_clk_en,
    Q => bck_in_q
);

ctr_2 : c_counter_binary_2
  PORT MAP (
    CLK => bck_sig,
    Q => lrck_in_q
);

end Behavioral;
