%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Plots data for identification of A*.

%Expt2Data{t}{d} = col vector of data type d from trial t.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function PlotData_Expt2(data_c, data_l, data_tput, data_time, Expt2Data)

NDataTypesToPlot = 3;

NTrial = length(Expt2Data); %NTrial = NTrial - 2; %For the paper only include 3 trials.

DataTypeNames = cell(NDataTypesToPlot,1); DataTypeNames{data_c} = 'c (bytes)'; DataTypeNames{data_l} = 'l (\mus)'; DataTypeNames{data_tput} = 't (bits/s)';

FigureSettings(figure)

for d = 1 : NDataTypesToPlot
    
    for t = 1 : NTrial
        
        subplot(NDataTypesToPlot, NTrial, (d-1)*NTrial + t)
        
        Time = Expt2Data{t}{data_time};
        
        VectorDataType = Expt2Data{t}{d};
        
        plot(Time, VectorDataType, 'k', 'linewidth', 2);
        
        axis('tight')
        
        if t == 1, ylabel(DataTypeNames{d}); end
        
        if d == 1, title(['Trial', ' ', num2str(t)]); end
        
        if d == NDataTypesToPlot, xlabel('Time (s)'); end
        
    end
    
end

         
        
            
            
            
        
        
