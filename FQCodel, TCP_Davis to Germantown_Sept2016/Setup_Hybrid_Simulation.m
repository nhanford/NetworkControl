K = 30; %Time horizon length

cStar_Gb = Bytes_To_Gigabits(2*10^7); % c* = 0.16 Gb
%Chosen to be slightly greater than the maximum c observed in q0 data (~1.7*10^7 bytes)

NSubintPerInt = round(1000/lStar_ms); %1000 ms / l* ms = number of subintervals in tau_k (length = 1s)
%In each subinterval, one packet drop may occur -> c dynamics.

[alpha_Gb, beta] = ComputeTCPParameters(Expt2Data, NSubintPerInt); %We approximate increase in c as linear (it looks exponential)
%Averages computed using trials 1 and 3.

c_k = 0.45;

x_k = [20, -10]; %Poor initial condition


















