%Converts bits (b) -> megabits (Mb) 
% x bits * 1 megabit / 10^6 bits = x * 10^(-6) megabits

function x_Mb = Bits_To_Megabits(x_b)

x_Mb = x_b * 10^(-6);