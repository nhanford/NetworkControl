%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Plots simulation of one mode. 
%X = matrix of observed continuous states.
%Two-continuous-state model: x1 = l - l* (ms) (row 1 of X), x2 = t - t* (Gbps) (row 2 of X)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function Plot_Observations_From_Single_Mode(X)

[~, K] = size(X);

%The sample standard deviation of state i up to time k is a measure of variability of state i at time k.
[SampleSD_l, SampleSD_t] = GetSampleStandardDeviation(X);

%Plot simulation.
FigureSettings(figure)

subplot(2, 2, 1) %Row 1 of X: l - l*
plot( 1:K, X(1,:), '-k', 'linewidth', 2 ); title('x1 = l - l* (ms)');

subplot(2, 2, 2) %Row 2 of X: t - t*
plot( 1:K, X(2,:), '-k', 'linewidth', 2 ); title('x2 = t - t* (Gbps)');

subplot(2, 2, 3) %Sample standard deviation of latency
plot( 1:K, SampleSD_l, '-k', 'linewidth', 2 ); title('Sample \sigma_l (ms)'); xlabel('Time (s)');

subplot(2, 2, 4) %Sample standard deviation of throughput
plot( 1:K, SampleSD_t, '-k', 'linewidth', 2 ); title('Sample \sigma_t (Gbps)'); xlabel('Time (s)');

%str = 'FQ On, Boulder, CO, Controlled FQ Active + CA, 2-State Model';
%annotation( 'textbox', [0.37 0.9 0.5 0.1], 'String', str, 'LineStyle', 'none' );
