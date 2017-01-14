%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Computes matrices, X and XPlus, for dynamics matrix identification (A*).

%StateVectors{t} = q0-state vectors from trial t.

%Mode q0 = 'c not flat' + congestion avoidance.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


function [X, XPlus] = GetMatrices_For_AStar_ID(StateVectors_q0)

NTrials = length(StateVectors_q0);

X = []; XPlus = [];

for t = 1 : NTrials
    
    dummy_t = StateVectors_q0{t}; %State vectors from trial t
    
    X = [X, dummy_t(:, 1:end-1)]; %All rows, col 1 -> col 29
    
    XPlus = [XPlus, dummy_t(:, 2:end)]; %All rows, col 2 -> col 30
    
end

