%Converts units bits (b) -> Gigabits (Gb) 
% bits * 1 Gigabit/10^9 bits = Gigabits

function x_Gb = ConvertbitsToGigabits(x_b)

x_Gb = 10^(-9)*x_b;