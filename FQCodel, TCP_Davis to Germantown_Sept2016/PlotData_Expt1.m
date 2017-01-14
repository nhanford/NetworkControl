%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Plots data used to identify FQ pacing rate factor.

%FQRatesData{r}{t}{d} = col vector of data type d from trial t at pacing rate r.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function PlotData_Expt1(data_c, data_l, data_tput, data_time, FQRateRange, FQRatesData)

NDataTypeToPlot = 3; 

[~, NTrial] = size(FQRatesData);

style = cell(NTrial,1); style{1} = 'k'; style{2} = 'g'; style{3} = 'r'; style{4} = 'b'; style{5} = 'm';

titles = cell(NDataTypeToPlot,1); titles{data_c} = 'c (bytes)'; titles{data_l} = 'l (\mus)'; titles{data_tput} = 't (bits/s)';

FigureSettings(figure);

for r = FQRateRange

    for d = 1:NDataTypeToPlot

        subplot(length(FQRateRange),NDataTypeToPlot,(r-min(FQRateRange))*NDataTypeToPlot + d);
        
        for t = 1:NTrial
    
            VectorDataType = FQRatesData{r}{t}{d};
            
            Time = FQRatesData{r}{t}{data_time};
    
            plot(Time, VectorDataType, style{t}, 'linewidth', 2); hold on
            
        end
 
        axis('tight');
        
        if d == 1, ylabel(strcat(num2str(r), '00 Mbps')); end
        if r == min(FQRateRange), title(titles{d}); end
        if r == max(FQRateRange), xlabel('Time (s)'); end

    end
    
end

legend('Trial 1', 'Trial 2', 'Trial 3', 'Trial 4', 'Trial 5');

%Revised plots (8/9) matched original plots (8/4) for data points that were checked.
        
        
    
    
    

