%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Outputs data used to identify FQ pacing rate factor.

%FQRatesData{r}{t}{d} = col vector of data type d from trial t at pacing rate r.
    %FQRatesData{r}{t}{1} = sender congestion window size (bytes)
    %FQRatesData{r}{t}{2} = latency (microseconds)
    %FQRatesData{r}{t}{3} = throughput (bits per second)
    %FQRatesData{r}{t}{4} = time (seconds)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function FQRatesData = GetData_Expt1(data_c, data_l, data_tput, data_time, FQRatesAdd)

NFQRate = length(FQRatesAdd); NTrial = length(FQRatesAdd{1});

FQRatesData = cell(NFQRate,NTrial);

for r = 1:NFQRate
    
    for t = 1:NTrial
        
        DataFileAddress = FQRatesAdd{r}{t};
    
        [time_s, l_mus, t_bps, d_B, r_packets, c_B] = ExtractRawData(DataFileAddress);
        
        FQRatesData{r}{t}{data_c} = c_B;
        
        FQRatesData{r}{t}{data_l} = l_mus;
        
        FQRatesData{r}{t}{data_tput} = t_bps;
        
        FQRatesData{r}{t}{data_time} = time_s;
        
    end
       
end

%Revised output (8/9) matched original output (8/4) for data points that were checked.
