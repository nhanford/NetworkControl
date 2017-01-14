%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Computes state vectors for a specified destination, identified by file index of trial 1.
%Two-continuous-state model:
    %x1 = throughput - nominal throughput (Gb/s), time k
    %x2 = latency - nominal latency (cs), time k
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Abbreviations

%OBSERVATIONS
%c = sender congestion window size ('snd_cwnd')
%d = data transmitted
%l = latency (measured via rtt)
%l* = nominal latency
%r = retransmits
%t = throughput
%t* = nominal throughput

%UNITS
%B = bytes
%b = bits
%cs = centiseconds (100th of a second)
%Gb = gigabits
%mus = microseconds
%P = packets
%s = seconds

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [S, SenderCongWindSize_Gb] = ComputeStateVectors(DataFileAddresses, file_trial1, NDestinations, NTrials, tStar_bps)

S = cell(NTrials,1); 

SenderCongWindSize_Gb = cell(NTrials,1);

n = 2;

for trial = 1:NTrials
    
    [time_s, l_mus, t_bps, d_B, r_packets, c_B] = ExtractRawData(DataFileAddresses{file_trial1 + (trial-1)*NDestinations});
    
    %Indices associated with same destination are: file_trial1 + (trial-1)*NDestinations, for all trials.

    lStar_mus = min(l_mus); %Nominal latency (rtt)
        
    K = length(time_s); %Time horizon length, 60 s
    
    Stilde = zeros(n,K); %Stack each state vector side-by-side: time 1 LEFT -> time K RIGHT.
    
    Stilde(1,:) = ConvertbitsToGigabits(transpose(t_bps) - tStar_bps*ones(1,K)); %x1 = t - t*, b/s -> Gb/s
    
    Stilde(n,:) = ConvertMicrosecToCentisec(transpose(l_mus) - lStar_mus*ones(1,K)); %x2 = l - l*, mus -> cs
    
    S{trial} = Stilde;
    
    SenderCongWindSize_Gb{trial} = ConvertBytesToGigabits(transpose(c_B)); %Sender congestion window size, B -> Gb

end



