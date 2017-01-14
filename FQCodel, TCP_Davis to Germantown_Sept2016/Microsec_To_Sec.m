%Converts units microseconds (mus) -> seconds (s) 
% mus * 1 s / 10^6 mus = s

function x_s = Microsec_To_Sec(x_mus)

x_s = x_mus / 10^6;
