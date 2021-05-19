library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;


entity i2s_controller is
    Port (
        clk, rst ,locked        : in std_logic  ; 
        ce                      : out std_Logic ; 
        
        goWait,goSend           : out std_logic ; 
        valid                   : in std_logic ; 
        
        en_adc,rst_adc,en_dac,rst_dac  : out std_logic ; 
        we_adc, we_dac                 : out std_logic_vector(3 downto 0) 
        );
end i2s_controller;

architecture Behavioral of i2s_controller is

type fsm is (por,init,idle,read_bram,i2s,write_bram,inc_addr);
signal state_reg , state_next : fsm ;
signal cnt_reg, cnt_next : unsigned(3 downto 0):= (others => '0');

signal ce_next , ce_reg :std_logic ; 

signal goWait_next , goWait_reg : std_logic ;  
signal goSend_next , goSend_reg : std_logic ;  

signal weadc_reg, weadc_next : std_logic_vector(3 downto 0) ; 
signal enadc_reg , enadc_next : std_Logic ; 
signal rstadc_reg , rstadc_next : std_Logic ;

signal wedac_reg, wedac_next : std_logic_vector(3 downto 0) ; 
signal endac_reg , endac_next : std_Logic ; 
signal rstdac_reg , rstdac_next : std_Logic ;

begin

ce <= ce_reg ; 

goWait <= goWait_reg ;
goSend <= goSend_reg ;

en_adc <= enadc_reg ; 
we_adc <= weadc_reg ;  
rst_adc <= rstadc_reg ; 

en_dac <= endac_reg ; 
we_dac <= wedac_reg ;  
rst_dac <= rstdac_reg ;

process(clk,rst)
begin 
    if(rst = '1')then 
        state_reg <= por ; 
        cnt_reg <= (others => '0'); 
        
        ce_reg <= '0' ; 
        
        goWait_reg <= '0' ;
        goSend_reg <= '0' ;
        
        weadc_reg <= (others => '0');
        enadc_reg <= '0' ; 
        rstadc_reg <= '1' ; 
        
        wedac_reg <= (others => '0');
        endac_reg <= '0' ; 
        rstdac_reg <= '1' ; 
    elsif(clk'event and clk = '1') then 
        state_reg <= state_next ;
        cnt_reg <= cnt_next ; 
        
        ce_reg <= ce_next ; 
        
        goWait_reg <= goWait_next ; 
        goSend_reg <= goSend_next ; 
        
        weadc_reg <= weadc_next ;
        enadc_reg <= enadc_next ;  
        rstadc_reg <= rstadc_next ; 
        
        wedac_reg <= wedac_next ;
        endac_reg <= endac_next ;  
        rstdac_reg <= rstdac_next ; 
    end if ;
end process ; 

process(state_reg,locked,cnt_reg,valid)
begin 
cnt_next <= cnt_reg ; 
ce_next <= '0' ;
 
goWait_next <= '0' ; 
goSend_next <= '0' ; 

weadc_next <= (others=> '0');
enadc_next  <= '0' ; 
rstadc_next <= '0' ; 

wedac_next <= (others=> '0');
endac_next  <= '0' ; 
rstdac_next <= '0' ; 

    case state_reg is 
    when por => 
        if(locked = '1') then 
            state_next <= init ; 
        else 
            state_next <= por ; 
        end if ;
    
    when init =>
    if(cnt_reg = "0100") then 
        state_next <= idle ;
        cnt_next <= (others => '0');
    else
        ce_next <= '1' ;
        cnt_next <= cnt_reg + 1; 
        state_next <= init ;
    end if ;
         
    when idle => 
        state_next <= read_bram ; 
    
    when read_bram =>
     if(cnt_reg = "010") then 
        state_next <= i2s ; 
        cnt_next <= (others => '0'); 
        goWait_next <= '1' ; 
        goSend_next <= '1' ;  
    else
        endac_next <= '1' ; 
        cnt_next <= cnt_reg + 1; 
        state_next <= read_bram ;
        endac_next <= '1' ; 
        wedac_next <= (others=> '0') ; 
     end if ;   
        
    when i2s => 
        if(valid = '1') then 
            state_next <= write_bram;
        else 
            state_next <= i2s ; 
        end if ; 
    
    when write_bram =>
    if(cnt_reg = "010") then 
        state_next <= inc_addr ; 
        cnt_next <= (others => '0');  
    else
        enadc_next <= '1' ; 
        cnt_next <= cnt_reg + 1; 
        state_next <= write_bram ;
        enadc_next <= '1' ; 
        weadc_next <= "1111" ; 
    end if ;
        
    when inc_addr =>
    if(cnt_reg = "100") then     
        state_next <= idle ;
        cnt_next <= (others => '0');
    else
        ce_next <= '1' ;
        cnt_next <= cnt_reg + 1; 
        state_next <= inc_addr ;
    end if ;

    when others => 
        state_next <= por ;
 end case ;  
end process ;

end Behavioral;
