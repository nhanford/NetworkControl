%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Extracts hybrid system state from .csv file. 

%Used to evaluate efficacy of our control policy vs. TCP-CoDel algorithm.

%Sender congestion window size = col 4
%xl = l_k - l* = col 7
%xt = t_k - t* = col 8
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


function [M, X] = GetHybridState_RealEval(DataFileAddress, cStar)

RowOffset = 1; %Start reading at row #2 due to header in row #1.
ColOffset = 0; %Start reading at col #1.

Data = csvread(DataFileAddress,RowOffset,ColOffset);

xl_ms = Data(:,7); %milliseconds

xt_Gbps = Data(:,8); %gigabits per second

X = [transpose(xl_ms); transpose(xt_Gbps)];

c_B = Data(:,4); %bytes

M = (c_B - cStar >= 0); 
%c_B(k) >= cStar <-> mode q1 <-> M(k) = 1
%c_B(k) < cStar <-> mode q0 <-> M(k) = 0