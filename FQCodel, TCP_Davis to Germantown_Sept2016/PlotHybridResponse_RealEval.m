%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Plots dynamical response of hybrid system subject to our control policy or to TCP-CoDel algorithm from real experiments.
%q0 (BLUE DOTTED LINE) <-> q1 (RED SOLID LINE).
%Two-continuous-state model: x1 = l - l* (ms), x2 = t - t* (Gbps).
%X(:,k) = observed continuous state at time k
%M(k) = mode in tau_k = [k, k+1) (1 -> q1, 0 -> q0)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function PlotHybridResponse_RealEval(M, X)
[n, K] = size(X); state_latency = 1; state_throughput = n;

TimeHorizon = 1:K;

Indices_q1 = find(M); %Indices for non-zero entries, M(k) = 1 -> q1

Indices_q0 = setdiff(TimeHorizon,Indices_q1); %Indices such that M(k) = 0 -> q0

%The sample standard deviation of state i up to time k is a measure of variability of state i at time k.
[SampleSD_l, SampleSD_t] = GetSampleStandardDeviation(X);
SampleSD = [SampleSD_l'; SampleSD_t'];

%x1, x2, std dev x1, std dev x2
FigureSettings(figure)
subplot(2,2,1)
PlotModeSpecificIntervals(Indices_q1, X, state_latency, 'r-'); %[k, k+1) red solid line -> mode q1 in tau_k
PlotModeSpecificIntervals(Indices_q0, X, state_latency, 'g-'); %[k, k+1) blue dotted line -> mode q0 in tau_k
title('x1 = l - l* (ms)');

subplot(2,2,2)
PlotModeSpecificIntervals(Indices_q1, X, state_throughput, 'r-'); %[k, k+1) red solid line -> mode q1 in tau_k
PlotModeSpecificIntervals(Indices_q0, X, state_throughput, 'g-'); %[k, k+1) blue dotted line -> mode q0 in tau_k
title('x2 = t - t* (Gbps)');

subplot(2,2,3)
PlotModeSpecificIntervals(Indices_q1, SampleSD, state_latency, 'r-'); %[k, k+1) red solid line -> mode q1 in tau_k
PlotModeSpecificIntervals(Indices_q0, SampleSD, state_latency, 'g-'); %[k, k+1) blue dotted line -> mode q0 in tau_k
title('Sample \sigma_l (ms)');
xlabel('Time (s)');

subplot(2,2,4)
PlotModeSpecificIntervals(Indices_q1, SampleSD, state_throughput, 'r-'); %[k, k+1) red solid line -> mode q1 in tau_k
PlotModeSpecificIntervals(Indices_q0, SampleSD, state_throughput, 'g-'); %[k, k+1) blue dotted line -> mode q0 in tau_k
title('Sample \sigma_t (Gbps)');
xlabel('Time (s)');
