library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity i2s_transmitter is
    Port (
        lrsel,goSend            : in std_logic ; 
        lr_ris,lr_fall,bck_ris  : in std_Logic ;
        clk, rst                : in std_logic ;   
        data                    : out std_logic ;
        data_in                 : in std_logic_vector(23 downto 0) ;
        valid                   : out std_Logic 
         );
end i2s_transmitter;



architecture Behavioral of i2s_transmitter is
type fsm is (idle, txLwait, txRwait, tx_L, tx_R);
signal state_reg , state_next : fsm ; 
signal cnt_reg,cnt_next : unsigned(6 downto 0) ;


signal aud : std_logic_vector(127 downto 0);
signal aud_data_reg,aud_data_next : std_Logic_vector(23 downto 0 );
signal zeros : std_logic_vector(102 downto 0):= (others => '0'); 

begin 
aud <= '0' & aud_data_reg & zeros ; 

process(rst,clk)
begin 
    if(rst = '1') then 
        state_reg <= idle ;        
        aud_data_reg <= (others => '0');
        cnt_reg <= "1111110" ; 
    elsif(clk'event and clk = '1') then
        state_reg <= state_next ;
        aud_data_reg <= aud_data_next ; 
        cnt_reg <= cnt_next ;
    end if ; 
end process ;

process(state_reg,goSend,lrsel,data_in,aud_data_reg,aud,lr_fall,lr_ris,bck_ris,cnt_reg)
begin 
aud_data_next <= aud_data_reg ; 
cnt_next <= cnt_reg ; 

    case state_reg is 
        when idle => 
        cnt_next <= "1111110" ; 
            --if( goSend = '1') then
                aud_data_next <= data_in ; 
                if(lrsel = '1') then 
                    state_next <= txLwait ;
                elsif (lrsel = '0')then 
                    state_next <= txrwait ;
                end if ;  
            --else 
            --    state_next <= idle ;
            --end if ; 
            
        when txLwait => 
            if (lr_ris = '1') then 
                state_next <= tx_L ;
            else 
                state_next <= txLwait ; 
            end if ; 
        
        when txRwait => 
            if (lr_fall = '1') then 
                state_next <= tx_R ;
            else 
                state_next <= txRwait ; 
            end if ;

        when tx_L => 
            if (lr_fall = '1') then 
                state_next <= idle ;
            else 
                state_next <= tx_L ; 
                if(bck_ris = '1') then 
                    cnt_next <= cnt_reg - 1 ; 
                    data <= aud(to_integer(cnt_reg)) ;
                else 
                end if ; 
            end if ;
        when tx_R =>
            if (lr_ris = '1') then 
                state_next <= idle ;
            else 
                state_next <= tx_R ; 
                if(bck_ris = '1') then
                cnt_next <= cnt_reg - 1 ; 
                data <= aud(to_integer(cnt_reg)) ;  
                else 
                end if ; 
            end if ; 
        when others => 
            state_next <= idle ; 
    end case ; 
end process;





end Behavioral;
