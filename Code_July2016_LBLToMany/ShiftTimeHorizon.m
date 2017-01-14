%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Computes mode-specific data matices for identification of linear dynamics and control matrix.
%Two-continuous-state model.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [X, XPlus, C] = ShiftTimeHorizon(StateVectors, Intervals, SenderCongWindSize)
%Intervals correspond to a specific discrete mode and are well-defined (i.e., tSTOP - tSTART > 1).
%C contains sender congestion window size for same time points as in X.
%X, XPlus, C are computed for the intervals specified and thus are mode-specific data matrices.

NTrials = length(StateVectors);

X = []; XPlus = []; C = [];
    
for trial = 1:NTrials %for each trial
        
    StateVecs_trial = StateVectors{trial}; 
    
    SCWSize_trial = SenderCongWindSize{trial};
        
    Intervals_trial = Intervals{trial};
        
    if ~isempty(Intervals_trial)
            
        [Nintervals, ~] = size(Intervals_trial); %row i holds interval i.
            
        for i = 1:Nintervals %for each interval
                
            tSTART = Intervals_trial(i,1); %col 1 = START time
                
            tSTOP = Intervals_trial(i,2); %col 2 = STOP time
                
            X = [X StateVecs_trial(:, tSTART : tSTOP-1)];   %initial:end-1
                    
            C = [C SCWSize_trial(tSTART : tSTOP-1)];        %initial:end-1
                
            XPlus = [XPlus StateVecs_trial(:, tSTART+1 : tSTOP)]; %initial+1:end

        end
            
    end
        
end
        
%X = [X StateVecs_trial(:,1:end-1)];
%XPlus = [XPlus StateVecs_trial(:,2:end)];


