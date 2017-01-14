%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Identifies FQ pacing rate factor, gamma, using data from mode q1 = 'c flat' + congestion avoidance.

%100 * r = gamma * c / l.
    %100 * r = FQ pacing rate (megabits per second, 'Mbps')
    %c = sender congestion window size (megabits, 'Mb')
    %l = latency (seconds, 's')
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function gamma = ComputeFQPacingRateFactor(data_c, data_l, FQRatesData, K_ca)

[NFQRate, NTrial] = size(FQRatesData);

AllGammaEstimates = [];

for r = 1 : NFQRate %Index across fixed FQ pacing rates.
    
    for t = 1 : NTrial
        
        c_B = FQRatesData{r}{t}{data_c}; %c (bytes), both modes
        
        FlatcInterval_Matrix = GetFlatcIntervals(c_B);
        
        FlatcInterval_Points = GetFlatcIntervalPoints(FlatcInterval_Matrix);
    
        K_q1 = intersect(K_ca, FlatcInterval_Points);       %Time points, mode q1 = 'c flat' + congestion avoidance.
                                                            %'K_q1 = K_ca \intersect K_bar' in methods doc.
                                                            
        c_B = c_B(K_q1);                                    %c (bytes), mode q1
        
        l_mus = FQRatesData{r}{t}{data_l}(K_q1);            %l (microseconds), mode q1
    
        GammaEstimates_rt = 100 * r * Microsec_To_Sec(l_mus) ./ Bytes_To_Megabits(c_B); %gamma [no units], 100 * r [Mbps]
    
        AllGammaEstimates = [AllGammaEstimates; GammaEstimates_rt]; %Store gamma estimates for each r, each t.
        
    end
    
end

FigureSettings(figure)

hist(AllGammaEstimates); title('Estimates of FQ pacing rate factor'); ylabel('Bin count'); xlabel('\gamma')

gamma = mean(AllGammaEstimates);




    
    


