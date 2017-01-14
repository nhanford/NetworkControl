%Simulates packet drops on tau_kStart = [kStart, kStart+1], and updates sender congestion window size, c, accordingly.

function [C, c_kStartPlus1, E] = SimulatePacketDrops(C, E, alpha, beta, kStart, NSubintPerInt) 
    
%Use continuous time Markov chain to simulate 'burstiness' of packet drops.
%p_kStart_avg = # bytes retransmitted / # bytes sent in tau_k (but numerator is not known, so we assign a distribution to p_kStart_avg.)
p_kStart_avg = normrnd(0.005, 0.001);
pi_low = 0.995; %probability that system is in state 'low', 1-pi_low = probability that system is in state 'high'.
pdrop_low = 10^(-5); %probability of packet drop in state 'low'
%pdrop_high = min(normrnd(0.7,0.1),1);
pdrop_high = min((p_kStart_avg - pi_low*pdrop_low)/(1 - pi_low), 1); %probability of packet drop in state 'high'.

%UNCOMMENT LINE BELOW TO FIX PACKET DROPS
ForcedPacketDrops = [87, 242, 646, 894, 1628, 1930];

for i = 1:NSubintPerInt
    
    state = unifrnd(0,1);
    
    pdrop = (state < pi_low)*pdrop_low + (state >= pi_low)*pdrop_high; 
    %state \in [0, pi_low) -> 'low', pdrop = pdrop_low; state \in [pi_low, 1] -> 'high', pdrop = pdrop_high.
    
    iStart = (kStart-1)*NSubintPerInt + i; %Start index of tau_kStart_i.

    E(iStart) = binornd(1, pdrop); %binornd(#trials, p_success)
    %E(iStart) = 1 : one packet was dropped in tau_kStart_i
    %E(iStart) = 0 : no packets were dropped in tau_kStart_i
    
    %UNCOMMENT LINE BELOW TO FIX PACKET DROPS
    if ismember(iStart, ForcedPacketDrops), E(iStart) = 1; else E(iStart) = 0; end %Remove noise in packet drops for now.
   
    C(iStart + 1) = (E(iStart)==1)*beta*C(iStart) + (E(iStart)==0)*(C(iStart) + alpha);
    %C(start of next subinterval) = beta*C(start of current subinterval), if a packet was dropped in current subinterval ('multiplicative decrease')
    %C(start of next subinterval) = C(start of current subinterval) + alpha, if no packets were dropped in current subinterval ('additive increase')

end
    
c_kStartPlus1 = C(iStart + 1);
%Sender congestion window size at time kStart+1
%iStart = start index of last subinterval of tau_kStart (end of loop) ->
%iStart + 1 = start index of first subinterval of tau_kStart+1 = [kStart+1, kStart+2].

