%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Outputs data file addresses for (A, B) identification.

%Expt2Add{t} = Address of data from trial t.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


function Expt2Add = GetDataFileAddresses_Expt2(FolderName, NTest)

Expt2Add = cell(NTest,1);

for t = 1 : NTest
        
    Expt2Add{t} = strcat(FolderName, 'trial', num2str(t), '.csv');
    
end


