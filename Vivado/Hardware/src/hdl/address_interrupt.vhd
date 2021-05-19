library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity address_interrupt is
    Port ( 
            clk, rst : in std_logic ; 
           input_1 : in STD_LOGIC_VECTOR (31 downto 0);
           input_2 : in STD_LOGIC_VECTOR (31 downto 0);
           output_x : out STD_LOGIC);
end address_interrupt;

architecture Behavioral of address_interrupt is

signal cnt_reg,cnt_next : unsigned(12 downto 0):= (others=> '0');
type fsm is (off_d,on_d) ; 
signal state_reg, state_next : fsm ; 
begin

process(clk,rst)
begin 
    if(rst = '1') then 
        cnt_reg <= (others => '0');
        state_reg <= off_d ;  
    elsif(clk'event and clk = '1') then 
        cnt_reg <= cnt_next ;
        state_reg <= state_next ;  
    end if;  
end process ; 

process(state_reg, input_1, input_2, cnt_reg)
begin 
state_next <= state_reg ; 
cnt_next <= cnt_reg ; 

    case state_reg is 
        when off_d => 
            if(input_1 = input_2) then 
                state_next <= on_d ; 
            else 
                state_next <= off_d ; 
            end if ;
        when on_d => 
            if (cnt_reg = 8191) then 
                cnt_next <= (others => '0');
                state_next <= off_d ; 
            else 
                cnt_next <= cnt_reg + 1 ; 
                state_next <= on_d ; 
            end if ;
    end case ; 

end process ; 

output_x <= '1' when state_reg = on_d else 
          '0' ;
          
end Behavioral;
