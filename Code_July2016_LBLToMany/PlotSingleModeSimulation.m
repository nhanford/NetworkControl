%Plots simulation of one mode. X = matrix of observed continuous states.

function PlotSingleModeSimulation(X)

[n, K] = size(X); state_throughput = 1;

%The sample standard deviation of state i up to time k is a measure of variability of state i at time k.
[SampleSD_t, SampleSD_l] = GetSampleStandardDeviation(X);

%Visualize simulation.
figure
for subfig = 1:2*n
    subplot(2,2,subfig)
    if subfig <= n
        plot( 1:K, X(subfig,:), '-k', 'linewidth', 1 );
        if subfig == state_throughput, title('x1: throughput - nominal throughput (Gb/s)');
        else title('x2: latency - nominal latency (cs)');
        end
    else
        if subfig == 3, plot( 1:K, SampleSD_t, '-k', 'linewidth', 1 ); title('Sample std dev of throughput (Gb/s)');
        else plot( 1:K, SampleSD_l, '-k', 'linewidth', 1 ); title('Sample std dev of latency (cs)');
        end
        xlabel('Time (s)');
    end  
end
str = 'FQ On, Boulder, CO, Controlled FQ Active + CA, 2-State Model';
annotation( 'textbox', [0.37 0.9 0.5 0.1], 'String', str, 'LineStyle', 'none' );
