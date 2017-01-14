%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Outputs data file addresses, data used to identify FQ pacing rate factor.

%FQRatesAdd{r}{t} = Address of data at r Gbps from trial t.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function FQRatesAdd = GetDataFileAddresses_Expt1(FolderName, NFQRate, NTest)

FQRatesAdd = cell(NFQRate,1);

for r = 1:NFQRate
    
    DummyCell = cell(NTest,1);
    
    for t = 1:NTest
        
        DummyCell{t} = strcat(FolderName, 'trial', num2str(t), '\', num2str(r), '00Mbps.csv');
    
    end
    
    FQRatesAdd{r} = DummyCell;
    
end
        
        









