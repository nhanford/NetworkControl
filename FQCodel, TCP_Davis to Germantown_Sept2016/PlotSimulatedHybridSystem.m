%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Plots simulated hybrid system: q0 (BLUE DOTTED LINE) <-> q1 (RED SOLID LINE).
%Two-continuous-state model: x1 = l - l* (ms), x2 = t - t* (Gbps).
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%DATA STORAGE KEY

%X(:,k) = observation at time k
%iStart = start index of ith subinterval of tau_k ('tau_k_i'), initial k = 1
%iStart = (k-1)*NSubintPerInt + i
%C(iStart) = sender congestion window size at iStart
%E(iStart) = packet drop in tau_k_i? (1 -> yes, 0 -> no)
%Either one packet was dropped, or no packets were dropped in tau_k_i. 
%M(k) = mode in tau_k (1 -> q1, 0 -> q0)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function PlotSimulatedHybridSystem(X, C, E, M, NSubintPerInt, cStar)
[n, K] = size(X); state_latency = 1; state_throughput = n;

TimeHorizon = 1:K;

SubTimeHorizon = 1:length(C); %1 : (K-1)*NSubintPerInt+1

Indices_q1 = find(M); %Indices for non-zero entries, M(k) = 1 -> q1

Indices_q0 = setdiff(TimeHorizon,Indices_q1); %Indices such that M(k) = 0 -> q0

Indices_packetdropevent = find(E); %Indices for non-zero entries, E(iStart) = 1 -> one packet was dropped in tau_k_i

StartIndices_tauk = find(mod(SubTimeHorizon,NSubintPerInt) == 1); %Start indices of tau_k

%The sample standard deviation of state i up to time k is a measure of variability of state i at time k.
[SampleSD_l, SampleSD_t] = GetSampleStandardDeviation(X);
SampleSD = [SampleSD_l'; SampleSD_t'];

%x1, x2, std dev x1, std dev x2
FigureSettings(figure)
subplot(2,2,1)
PlotModeSpecificIntervals(Indices_q1, X, state_latency, 'r-'); %[k, k+1] red solid line -> mode q1 in tau_k
PlotModeSpecificIntervals(Indices_q0, X, state_latency, 'g-'); %[k, k+1] blue dotted line -> mode q0 in tau_k
title('x1 = l - l* (ms)');

subplot(2,2,2)
PlotModeSpecificIntervals(Indices_q1, X, state_throughput, 'r-'); %[k, k+1] red solid line -> mode q1 in tau_k
PlotModeSpecificIntervals(Indices_q0, X, state_throughput, 'g-'); %[k, k+1] blue dotted line -> mode q0 in tau_k
title('x2 = t - t* (Gbps)');

subplot(2,2,3)
PlotModeSpecificIntervals(Indices_q1, SampleSD, state_latency, 'r-'); %[k, k+1] red solid line -> mode q1 in tau_k
PlotModeSpecificIntervals(Indices_q0, SampleSD, state_latency, 'g-'); %[k, k+1] blue dotted line -> mode q0 in tau_k
title('Sample \sigma_l (ms)');
xlabel('Time (s)');

subplot(2,2,4)
PlotModeSpecificIntervals(Indices_q1, SampleSD, state_throughput, 'r-'); %[k, k+1] red solid line -> mode q1 in tau_k
PlotModeSpecificIntervals(Indices_q0, SampleSD, state_throughput, 'g-'); %[k, k+1] blue dotted line -> mode q0 in tau_k
title('Sample \sigma_t (Gbps)');
xlabel('Time (s)');

%Packet drop events (a maximum of 1 packet can be dropped in each subinterval), sender congestion window size
%Red circle at start of subinterval indicates that one packet was dropped in that subinterval.
%Absence of red circle at start of subinterval indicates that no packets were dropped in that subinterval.
FigureSettings(figure)
plot(SubTimeHorizon, C, '-k', 'linewidth', 2); hold on
plot(Indices_packetdropevent, C(Indices_packetdropevent), 'or', 'linewidth', 2); hold on
plot(StartIndices_tauk, zeros(1,K),'d','color',[0 .5 0],'linewidth',2); hold on
plot(SubTimeHorizon, cStar*ones(size(SubTimeHorizon)), 'b','linewidth',2);
xlabel('Time (1/15 s)'); %There are 15 subintervals in each 1-second interval.
ylabel('Sender congestion window size "c" (Gb)');
legend('c (Gb)', 'packet drop', '1s interval', 'c^*');
axis('tight');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% TEST CODE

% %Test identification of q0/q1 indices.
% K = 10;
% TimeHorizon = 1:K;
% M = [1 0 1 0 0 1 0 1 0 0];
% Indices_q1 = find(M) %-> 1 3 6 8
% Indices_q0 = setdiff(TimeHorizon,Indices_q1) % -> 2 4 5 7 9 10
% % Passed 7/5

% % Test identification of start indices of tau_k.
% K = 4;
% NSubintPerInt = 5;
% SubTimeHorizon = 1:(K-1)*NSubintPerInt + 1;
% StartIndices_tauk = find(mod(SubTimeHorizon,NSubintPerInt) == 1) %-> 1 6 11 16 
% % Passed 7/5

% % Test plotting observations
% % Run "test ID of q0/q1 indices".
% ScriptO = [linspace(1,5,K); linspace(2,10,K)];
% Fig_handle = figure;
% FigureSettings(Fig_handle)
% Handle_plot_q1 = PlotModeSpecificIntervals(Indices_q1, ScriptO, subfig, 'r-');
% Handle_plot_q0 = PlotModeSpecificIntervals(Indices_q0, ScriptO, subfig, 'b:');
% Handle_legend_q1 = legend(Handle_plot_q1, 'mode q1: CA-FQ active', 'Location', 'East');
% copyobj(Handle_legend_q1, Fig_handle);
% legend(Handle_plot_q0, 'mode q0: CA-FQ passive', 'Location', 'West');
% title('x1: throughput - nominal throughput (Gb/s)');

% %Passed 7/5










   
    
 









    

