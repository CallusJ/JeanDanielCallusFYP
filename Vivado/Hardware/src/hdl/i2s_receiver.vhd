library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;


entity i2s_receiver is
    Port (
        lrsel,goWait            : in std_logic ; 
        lr_ris,lr_fall,bck_ris  : in std_Logic ;
        clk, rst                : in std_logic ;   
        data                    : in std_logic ;
        data_out                : out std_logic_vector(23 downto 0) ;
        valid                   : out std_Logic   
     );
end i2s_receiver;

architecture Behavioral of i2s_receiver is

type fsm is (idle, rxLwait,rxRwait,rx_R,rx_L,rdy);
signal state_reg , state_next : fsm ; 
signal cnt_reg,cnt_next : unsigned(4 downto 0):= (others => '0'); 
signal aud_data_long_reg,aud_data_long_next : std_logic_vector(24 downto 0):=(others => '0');
signal valid_reg, valid_next : std_logic ; 

begin
data_out <= aud_data_long_reg(23 downto 0); 
valid <=valid_reg; 

 process(clk,rst)
 begin 
    if(rst = '1')then 
        state_reg <= idle ; 
        cnt_reg <= "11000"  ;
        aud_data_long_reg <= (others => '0') ; 
        valid_reg <= '0' ; 
    elsif(clk'event and clk = '1') then 
        state_reg <= state_next ; 
        cnt_reg <= cnt_next ; 
        aud_data_long_reg <= aud_data_long_next ;
        valid_reg <=valid_next ; 
    end if ; 
 end process ;
 
 process(state_reg,goWait,lrsel,lr_ris,lr_fall,bck_ris,cnt_reg,aud_data_long_reg)
 begin 
 cnt_next <= cnt_reg ; 
 state_next <= state_reg ; 
 aud_data_long_next <= aud_data_long_reg ; 
 valid_next <= '0' ; 
    case state_reg is 
        when idle => 
            if(goWait = '1') then 
            aud_data_long_next <= (others => '0');
            cnt_next <= "11000"  ;
                if(lrsel = '1') then 
                    state_next <= rxLwait ;
                else
                    state_next <= rxrwait ;
                end if ;
            else
                state_next <= idle ;
            end if; 
        when rxLwait =>
            if (lr_ris = '1') then 
                state_next <= rx_L ;
            else 
                state_next <= rxLwait ; 
            end if ; 
        when rxRwait =>
        aud_data_long_next <= (others => '0');
            if (lr_fall = '1') then 
                state_next <= rx_R ;
            else 
                state_next <= rxRwait ; 
            end if ;
    
        when rx_R =>
            if (lr_ris = '1') then 
                state_next <= rdy ;
            else 
                state_next <= rx_R ; 
                if(bck_ris = '1' and cnt_reg > 0) then
                    cnt_next <= cnt_reg - 1 ; 
                    aud_data_long_next(to_integer(cnt_reg)) <= data ;
                elsif(bck_ris = '1' and cnt_reg = 0) then 
                    aud_data_long_next(to_integer(cnt_reg)) <= data ;
                    state_next <= rdy ;
                else 
                end if ; 
            end if ; 
            
        when rx_L =>
            if (lr_fall = '1') then 
                state_next <= rdy ;
            else 
                state_next <= rx_L ; 
                if(bck_ris = '1') then 
                    cnt_next <= cnt_reg - 1 ; 
                    aud_data_long_next(to_integer(cnt_reg)) <= data ;
                else 
                end if ; 
            end if ; 
        when rdy => 
            valid_next <= '1' ;
            state_next <= idle ; 
        when others => 
            state_next <= idle;
     end case ;  
 end process;
 

end Behavioral;
