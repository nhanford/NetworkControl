Evals = [1 2 3 4 6 8 9]; NEvals = length(Evals); 

cStar = 8 * 10^6; %bytes

ControlType = cell(2); 
ControlType{1} = 'On.csv'; %Our policy
ControlType{2} = 'Off.csv'; %TCP-CoDel algorithm

for i = 1 : 1
    
    DataFileAddress = strcat('Empirical tests, Oct 10\',num2str(Evals(i)),'c',ControlType{2});

    [M, X] = GetHybridState_RealEval(DataFileAddress, cStar);
    
    PlotHybridResponse_RealEval(M, X);
    
end



