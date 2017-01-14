%Converts bits (b) -> gigabits (Gb) 
% x bits * 1 gigabit / 10^9 bits = x * 10^(-9) gigabits

function x_Gb = Bits_To_Gigabits(x_b)

x_Gb = x_b * 10^(-9);