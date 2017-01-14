%Converts units bytes (B) -> megabits (Mb) 
% bytes * 8 bits/byte * 1 Megabit/10^6 bits = megabits

function x_Mb = Bytes_To_Megabits(x_B)

x_Mb = 8*10^(-6)*x_B;


