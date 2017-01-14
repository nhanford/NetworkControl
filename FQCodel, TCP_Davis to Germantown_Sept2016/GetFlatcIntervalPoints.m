%Returns data points associated with 'c flat' intervals, [StartTime_1 : EndTime_1, StartTime_2 : EndTime_2, ...].
%'c flat' intervals are from a vector of observations of sender congestion window size over time.

function FlatcIntervalPoints = GetFlatcIntervalPoints(FlatcIntervalMatrix)

[NInterval, ~] = size(FlatcIntervalMatrix);

FlatcIntervalPoints = [];

for i = 1 : NInterval
    
    StartTime = FlatcIntervalMatrix(i,1); EndTime = FlatcIntervalMatrix(i,2);
    
    FlatcIntervalPoints = [FlatcIntervalPoints, StartTime : EndTime];
    
end

%Tests passed 8/9