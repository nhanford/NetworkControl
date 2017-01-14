%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Computes [mode q0 = 'c not flat' + congestion avoidance] state vectors.

%x_k(1) = l_k - l* [ms]; x_k(2) = t_k - t* [Gbps].

%StateVectors_q0{t}(:,j) = q0-state vector j from trial t (j *not* = time point, data may correspond to slow start or q1)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function StateVectors_q0 = ComputeStateVectors_q0(data_l, data_tput, Data_q0, K_q0, lStar_ms, tStar_Gbps)

NTrial = length(Data_q0); StateVectors_q0 = cell(NTrial);

for t = 1 : NTrial
    
    l_mus = Data_q0{t}{data_l}(K_q0); 
    
    t_bps = Data_q0{t}{data_tput}(K_q0);
    
    StateVectors_q0{t} = CalculateStates_1Sample(l_mus, lStar_ms, t_bps, tStar_Gbps);
        
end




    
    







