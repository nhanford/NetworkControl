%Plots intervals of mode qi with special line style and returns plot handle for use in function PlotSimulatedHybridSystem.

function PlotModeSpecificIntervals(Indices_qi, X, state, LineStyle_qi)

[~, K] = size(X);

for i = 1:length(Indices_qi)
    
    iStart = Indices_qi(i); %Start index of a qi-interval.
    
    if iStart ~= K, %Time horizon = 1:K -> final interval = [K-1, K].
        
        interval_qi = [iStart, iStart + 1]; %Any time interval = [start index, start index + 1].
 
        plot(interval_qi, X(state,interval_qi), LineStyle_qi, 'linewidth', 3); hold on %Plot qi-interval in special style.
        
    end
    
end
