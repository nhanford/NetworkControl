%Converts units microseconds (mus) -> centiseconds (cs) 
%microsecs * 1 s/10^6 microsec * 10^2 centiseconds/1 s = centiseconds

function x_cs = ConvertMicrosecToCentisec(x_mus)

x_cs = 10^(-4)*x_mus;