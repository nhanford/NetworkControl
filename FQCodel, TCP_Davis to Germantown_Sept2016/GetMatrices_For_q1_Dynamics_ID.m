%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Computes matrices for identification of B* using q1-state vectors.

%Mode q1 = 'c flat' + congestion avoidance.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [U, X, XPlus] = GetMatrices_For_q1_Dynamics_ID(StateVectors_q1)

[NFQRate, NTrial] = size(StateVectors_q1);

X = []; XPlus = []; U = []; %u = (100 * r) Mbps = (10^(-1) * r) Gbps

for r = 1 : NFQRate
    
    for t = 1 : NTrial
        
        if ~isempty(StateVectors_q1{r}{t}) %StateVectors_q1{r}{t} will be empty if there are no flat c intervals after slow start, rate r, trial t.
            
            NInterval = length(StateVectors_q1{r}{t}); %Quantity of flat c intervals after slow start, rate r, trial t.
            
            for i = 1 : NInterval
                
                dummy_rti = StateVectors_q1{r}{t}{i};
                
                X = [X, dummy_rti(:, 1:end-1 )]; %All rows, col 1 -> 2nd-to-last, rate r, trial t, 'c flat' interval i.
                    
                XPlus = [XPlus, dummy_rti(:, 2:end)]; %All rows, col 2 -> last, rate r, trial t, 'c flat' interval i.
                    
                U = [U, 10^(-1) * r * ones(1, length(dummy_rti)-1)]; %FQ pacing rate (Gbps)
                    
            end
                
        end
            
    end
        
 end
           
