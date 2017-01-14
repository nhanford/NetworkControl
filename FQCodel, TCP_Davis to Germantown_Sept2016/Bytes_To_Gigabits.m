%Converts units Bytes (B) -> Gigabits (Gb) 
% Bytes * 8 bits/Byte * 1 Gigabit/10^9 bits = Gigabits

function x_Gb = Bytes_To_Gigabits(x_B)

x_Gb = 8*10^(-9)*x_B;


