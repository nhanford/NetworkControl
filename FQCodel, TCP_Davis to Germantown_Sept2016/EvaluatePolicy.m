%Outputs observations of the hybrid dynamical response subject to a specified policy.
%TCPCoDel == 1 <-> TCP-CoDel algorithm
%TCPCoDel == 0 <-> Our policy

function EvaluatePolicy(TCPCoDel)

if TCPCoDel == 1
    Policy = 'Off.csv'; %TCP-CoDel algorithm
elseif TCPCoDel == 0
    Policy = 'On.csv'; %Our policy
end

Evals = [1 2 3 4 6 8 9]; NEvals = length(Evals); 

cStar = 8 * 10^6; %bytes

for i = 1:NEvals
    
    DataFileAddress = strcat('Empirical tests, Oct 10\',num2str(Evals(i)),'c',Policy);

    [M, X] = GetHybridState_RealEval(DataFileAddress, cStar);
    
    PlotHybridResponse_RealEval(M, X);
    
end



