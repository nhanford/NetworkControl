%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Gets trial number from file index, as each row corresponds to a distinct trial.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function row = GetTrialNumber(file, NTrials, NDestinations)

row = 1*(file <= NDestinations) + NTrials*(file > NDestinations*(NTrials-1)); %Is row = first or row = last?
if (row==0) %If not, iterate through middle rows...
    sum = 0;
    for i = 2:NTrials-1, sum = sum + i*(file <= i*NDestinations && file >= 1 + (i-1)*NDestinations); end
    row = sum;
end