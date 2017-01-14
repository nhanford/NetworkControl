%Computes the TCP parameters using data from mode q0 = 'c not flat' + congestion avoidance.
%beta = multiplicative decrease per subinterval (no units), alpha = additive increase (Gb per subinterval)
%Expt2Data{t}{d} = col vector of data type d from trial t. (c : data type 1 in bytes)

function [alpha_Gb, beta] = ComputeTCPParameters(Mode_q0_data, NSubintPerInt)

Trials = [1 3]; %Use data from trial 1 and trial 3.

IncreaseInt = cell(length(Trials)); DecreaseInt = IncreaseInt;

IncreaseInt{1} = [5 12; 13 24; 25 30];
%Intervals associated with increasing sender congestion window size for trial 1: [5, 12], [13, 24], [25, 30].

DecreaseInt{1} = [12 13; 24 25];
%Intervals associated with decreasing sender congestion window size for trial 1: [12, 13], [24, 25].

IncreaseInt{2} = [5 15; 16 29];
%Intervals associated with increasing sender congestion window size for trial 3: [5, 15], [16, 29].

DecreaseInt{2} = [15 16; 29 30];
%Intervals associated with decreasing sender congestion window size for trial 3: [15, 16], [29, 30].

alphas = []; betas = [];

for t = 1 : length(Trials)
    
    [NInc, ~] = size(IncreaseInt{t}); %Number of increasing intervals for trial #Trial(t) (trial #1 or trial #3)
    
    for i = 1 : NInc
        
        tStart = IncreaseInt{t}(i,1);
        
        tEnd = IncreaseInt{t}(i,2);
        
        alphas = [alphas Bytes_To_Gigabits((Mode_q0_data{Trials(t)}{1}(tEnd) - Mode_q0_data{Trials(t)}{1}(tStart))/((tEnd - tStart)*NSubintPerInt))];
            
    end
    
    [NDec, ~] = size(DecreaseInt{t}); %Number of decreasing intervals for trial #Trial(t) (trial #1 or trial #3)
    
    for i = 1 : NDec
    
        tStart = DecreaseInt{t}(i,1);
        
        tEnd = DecreaseInt{t}(i,2);
        
        betas = [betas Mode_q0_data{Trials(t)}{1}(tEnd)/Mode_q0_data{Trials(t)}{1}(tStart)];
        
    end
        
end
        
    
alpha_Gb = mean(alphas);

beta = mean(betas);