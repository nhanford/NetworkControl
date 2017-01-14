%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Converts microseconds (mus) to milliseconds (ms).

% x microseconds * 1 second / 10^6 microseconds * 10^3 milliseconds / second = x * 10^(-3) milliseconds
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function x_ms = Microsec_To_Millisec(x_mus)

x_ms = x_mus * 10^(-3);

