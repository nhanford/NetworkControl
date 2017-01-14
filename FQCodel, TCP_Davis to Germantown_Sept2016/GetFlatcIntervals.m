%Extracts time intervals of constant sender congestion window size, c, from a vector of observations over time.
%Row i, col 1 = start time of ith 'c flat' interval
%Row i, col 2 = stop time of ith 'c flat' interval

function FlatcIntervalMatrix = GetFlatcIntervals(c)

K = length(c);

FlatcIntervalMatrix = [];

StartTime = 1; EndTime = StartTime;

for k = 1 : K-1
    
    if c(StartTime) == c(k+1)
        
        EndTime = k + 1;
        
        if EndTime == K, FlatcIntervalMatrix = [FlatcIntervalMatrix; [StartTime, EndTime]]; end
                
    else
        
        if StartTime ~= EndTime, FlatcIntervalMatrix = [FlatcIntervalMatrix; [StartTime, EndTime]]; end
        
        StartTime = k + 1; EndTime = StartTime;
        
    end
    
end

%8/9
%Manual output matched function output for Expt 1.1 data (Fixed max FQ pacing rate \in [1,...,10] Gb/s, Test 1 and Test 2).


    
    