%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Plots q1-state vectors observed from experiments on the production network and the sample standard deviation of latency and throughput 
    %for 'c flat' intervals of sufficient length.
%Will be shown in comparison to the dynamical response subject to our controller.
%StateVectors_q1{r}{t}{i} = q1-state vectors, (fixed) rate r, trial t, 'c flat' interval i.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function Plot_Response_From_Baseline_Control(StateVectors_q1)

[NFQRate, NTrial] = size(StateVectors_q1);

for r = 1 : NFQRate
    
    for t = 1 : NTrial
        
        if ~isempty(StateVectors_q1{r}{t}) %StateVectors_q1{r}{t} will be empty if there are no flat c intervals after slow start, rate r, trial t.
            
            NInterval = length(StateVectors_q1{r}{t}); %Quantity of flat c intervals after slow start, rate r, trial t.
            
            for i = 1 : NInterval
                
                [~, NTime_i] = size(StateVectors_q1{r}{t}{i}); %Extract the number of time points in interval i.
                
                if NTime_i > 5 %Want interval to be sufficiently long so that we can compute sample standard deviation
                
                    Plot_Observations_From_Single_Mode(StateVectors_q1{r}{t}{i});
                    
                end
                
            end
            
        end
        
    end
    
end
                    
                    
                
                    
                    
                    