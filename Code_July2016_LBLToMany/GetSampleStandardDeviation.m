%Computes sample standard deviation of state i up to time k (easy measure of variability of state i at time k).
%Two-continuous-state model: x1 = t - t* (Gb/s), x2 = l - l* (cs).

function [SampleSD_t, SampleSD_l] = GetSampleStandardDeviation(ScriptO)

[n, K] = size(ScriptO); state_throughput = 1; state_latency = n;

SampleSD_t = zeros(K,1); SampleSD_l = zeros(K,1);

for k = 1:K, 
    
    SampleSD_t(k) = std( ScriptO( state_throughput, 1:k ), 1 ); %SD of throughput observations time 1 -> time k
    
    SampleSD_l(k) = std( ScriptO( state_latency, 1:k ), 1 ); %SD of latency observations time 1 -> time k.
    
end