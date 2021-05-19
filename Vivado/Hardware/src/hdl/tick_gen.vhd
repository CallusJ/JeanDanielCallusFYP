library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity tick_gen is
    Port (
        clk , rst                           : in std_logic ; 
        sck_in, bck_in, lrck_in             : in std_Logic ; 
        lr_ris,lr_fall,bck_ris,bck_fall     :  out std_Logic ; 
        lrck, sck , bck                     : out STD_LOGIC
     );
end tick_gen;

architecture Behavioral of tick_gen is
    signal bck_reg, lrck_reg  : std_Logic_vector (1 downto 0);
    signal b0,b1,lr0,lr1      : std_Logic ;
begin
lrck <= lrck_in ; 
sck <= sck_in ; 
bck <= not bck_in ;

bck_reg     <= b1 & b0 ;
bck_ris <=  '1' when bck_reg = "01" else 
            '0' ;  
bck_fall <= '1' when bck_reg = "10" else 
            '0' ;  
lrck_reg    <= lr1 & lr0 ;
lr_ris <=   '1' when lrck_reg = "01" else 
            '0' ;  
lr_fall <=  '1' when lrck_reg = "10" else 
            '0' ;
            
process(rst,clk)
begin 
    if(rst = '1') then 
    elsif(clk'event and clk = '1') then

        b1 <= b0 ; 
        b0 <= bck_in ;
        lr1 <= lr0 ; 
        lr0 <= lrck_in ; 
        
    end if ; 
end process ;
end Behavioral;
