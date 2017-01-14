%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Computes sample standard deviation of state i up to time k (easy measure of variability of state i at time k).
%Two-continuous-state model: x1 = l - l* (ms), x2 = t - t* (Gbps).
%X = matrix of observed continuous states.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [SampleSD_l, SampleSD_t] = GetSampleStandardDeviation(X)

[n, K] = size(X); state_latency = 1; state_throughput = n;

SampleSD_l = zeros(K,1); SampleSD_t = SampleSD_l;

for k = 1:K, 
    
    SampleSD_l(k) = std( X( state_latency, 1:k ), 1 ); %SD of latency observations time 1 -> time k.

    SampleSD_t(k) = std( X( state_throughput, 1:k ), 1 ); %SD of throughput observations time 1 -> time k
        
end

%Y = std(X,1) normalizes by N = sample size and produces the square root of the second moment of the sample about its mean.
%For vectors, Y = std(X) returns the standard deviation.