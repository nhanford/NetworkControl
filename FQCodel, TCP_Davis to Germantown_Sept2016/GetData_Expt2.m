%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Outputs data for (A, B) identification.

%Expt2Data{t}{d} = col vector of data type d from trial t.
    %Expt2Data{t}{1} = sender congestion window size (bytes)
    %Expt2Data{t}{2} = latency (microseconds)    
    %Expt2Data{t}{3} = throughput (bits per second)
    %Expt2Data{t}{4} = time (seconds)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function Expt2Data = GetData_Expt2(data_c, data_l, data_tput, data_time, Expt2Add)

NTrial = length(Expt2Add);

Expt2Data = cell(NTrial, 1);

for t = 1 : NTrial
    
    DataFileAddress = Expt2Add{t};
    
    [time_s, l_mus, t_bps, d_B, r_packets, c_B] = ExtractRawData(DataFileAddress);
        
    Expt2Data{t}{data_c} = c_B;
        
    Expt2Data{t}{data_l} = l_mus;
        
    Expt2Data{t}{data_tput} = t_bps;
        
    Expt2Data{t}{data_time} = time_s;
    
end



