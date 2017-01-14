%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Computes [mode q1 = 'c flat' + congestion avoidance] state vectors.

%x(1) = l - l* [ms], x(2) = t - t* [Gbps].

%StateVectors_q1{r}{t}{i}(:,j) = q1-state vector j of 'c flat' interval i, (fixed) rate r, trial t. 
%(j *not* = time point, data may correspond to slow start or q0) 
%StateVectors_q1{r}{t} = [] if there are no 'c flat' intervals, rate r, trial t.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function StateVectors_q1 = ComputeStateVectors_q1(data_c, data_l, data_tput, FQRatesData, lStar_ms, K_ca, tStar_Gbps)

[NFQRate, NTrial] = size(FQRatesData);

StateVectors_q1 = cell(NFQRate, NTrial);

for r = 1 : NFQRate %Index across fixed FQ pacing rates.
    
    for t = 1 : NTrial
        
        c_B = FQRatesData{r}{t}{data_c};
        
        FlatcInterval_Matrix = GetFlatcIntervals(c_B);
        
        if isempty(FlatcInterval_Matrix), StateVectors_q1{r}{t} = []; %No 'c flat' intervals -> no mode q1 data.
            
        elseif size(FlatcInterval_Matrix,1) == 1 && FlatcInterval_Matrix(2) <= K_ca(1), StateVectors_q1{r}{t} = [];
        %There is only one 'c flat' interval. The end point <= (conservative) start time of congestion avoidance (=5).
        %E.g. Interval [5 6] is completely in mode q1, but interval [4 5] is not. 
        
        else
        
            if size(FlatcInterval_Matrix,1) > 1 && FlatcInterval_Matrix(1,2) <= K_ca(1), FlatcInterval_Matrix(1,:) = []; end
            %There are at least two 'c flat' intervals. The earliest interval has end point too small for mode q1. This interval is removed.
        
            [NInterval, ~] = size(FlatcInterval_Matrix); %row i : 'flat c' interval i, at least 1-sec duration in congestion avoidance
        
            for i = 1 : NInterval
            
                K_q1_i = intersect(K_ca, FlatcInterval_Matrix(i,1) : FlatcInterval_Matrix(i,2)); %mode q1 indices, interval i
                
                l_mus_i = FQRatesData{r}{t}{data_l}(K_q1_i);
            
                t_bps_i = FQRatesData{r}{t}{data_tput}(K_q1_i);
            
                StateVectors_q1{r}{t}{i} = CalculateStates_1Sample(l_mus_i, lStar_ms, t_bps_i, tStar_Gbps);
                
            end
                  
        end
        
    end
    
end