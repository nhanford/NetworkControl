close all; clear all; clc;

NTrial = 5; NFQRate = 10; lStar_ms = 68; tStar_Gbps = 10; data_c = 1; data_l = 2; data_tput = 3; data_time = 4;

k_ca = 5;           %Conservative start index for congestion avoidance.
                    %Slow start is over by ~2 s, evident by end of sharp rise in throughput (Data plots: Expt 1, Expt 2).
K_ca = k_ca : 30;   %Congestion avoidance points, last time point: 30.

%% EXPT 1: Mode q1 = 'c flat' + congestion avoidance. GOALS: Identify FQ pacing rate factor (gamma); compute q1-state vectors.

FQRatesAdd = GetDataFileAddresses_Expt1('Data, Expt 1, Identify FQ pacing rate factor\', NFQRate, NTrial); 
%OUTPUT: FQRatesAdd{r}{t} = Address of data at r Gbps from trial t.

FQRatesData = GetData_Expt1(data_c, data_l, data_tput, data_time, FQRatesAdd); 
%OUTPUT: FQRatesData{r}{t}{d} = col vector of data type d from trial t at pacing rate r.

PlotData_Expt1(data_c, data_l, data_tput, data_time, 1:5, FQRatesData); 
%OUTPUT: Plots for max FQ pace rate = 1, ..., 5.

PlotData_Expt1(data_c, data_l, data_tput, data_time, 6:10, FQRatesData); 
%OUTPUT: Plots for max FQ pace rate = 6, ..., 10.

gamma = ComputeFQPacingRateFactor(data_c, data_l, FQRatesData, K_ca);

StateVectors_q1 = ComputeStateVectors_q1(data_c, data_l, data_tput, FQRatesData, lStar_ms, K_ca, tStar_Gbps);
%OUTPUT: StateVectors_q1{r}{t}{i}(:,j) = q1-state vector, (fixed) rate r, trial t, time point j of 'c flat' interval i (>= k_ca).

%% EXPT 2: Mode q0 = 'c not flat' + congestion avoidance. GOALS: Compute q0-state vectors; identify A*.
%Sender congestion window size was *not* flat for Expt2Data (8/29)

Expt2Add = GetDataFileAddresses_Expt2('Data, Expt 2, Identify (A, B)\', NTrial); 
%OUTPUT: Expt2Add{t} = Address of data from trial t.

Expt2Data = GetData_Expt2(data_c, data_l, data_tput, data_time, Expt2Add); 
%OUTPUT: Expt2Data{t}{d} = col vector of data type d from trial t.

PlotData_Expt2(data_c, data_l, data_tput, data_time, Expt2Data); 
%OUTPUT: Plots of sender congestion window size, latency, throughput.

StateVectors_q0 = ComputeStateVectors_q0(data_l, data_tput, Expt2Data, K_ca, lStar_ms, tStar_Gbps);
%OUTPUT: StateVectors_q0{t} = ['c not flat' + congestion avoidance] state vectors, trial t.

AStar = ComputeAStar(StateVectors_q0); %Dynamics matrix

%% GOAL: Identify B* using q1-state vectors and A*.

BStar = ComputeBStar(AStar, StateVectors_q1);

%% Simulate q1-subsystem model

K = 30; %Time horizon length

l_0_ms = Microsec_To_Millisec(7.2*10^4); t_0_Gbps = Bits_To_Gigabits(2.8*10^8); %Values in ballpark of those observed at 5 s when max FQ pacing rate was fixed at 300 Mbps.
x_k = [l_0_ms - lStar_ms; t_0_Gbps - tStar_Gbps]; %Initial condition

%q1-Dynamical response with our controller
[X_Ours, U] = SimulateOnlineControl(K, x_k, AStar, BStar, zeros(2));
Plot_Observations_From_Single_Mode(X_Ours);

%q1-Dynamical response with baseline controller
ABar = ComputeABar(StateVectors_q1);
[X_Baseline, ~] = SimulateOnlineControl(K, x_k, ABar, zeros(2,1), zeros(2)); %B is set to (0, 0)^T since our control algorithm is not applicable.
Plot_Observations_From_Single_Mode(X_Baseline);

%% Simulate hybrid system model

Setup_Hybrid_Simulation;
TCPCoDel = 0; %Choose 1 for standard TCP-CoDel implementation, choose 0 for our controller.
[X, C, E, M, UStar] = SimulateHybridSystem(TCPCoDel, K, x_k, NSubintPerInt, c_k, AStar, BStar, zeros(2), zeros(2), ABar, zeros(2), beta, alpha_Gb, cStar_Gb, tStar_Gbps); 
% 
%[X, C, E, M, UStar] = SimulateHybridSystem_NewSwitchingRule(TCPCoDel, K, x_k, NSubintPerInt, c_k, AStar, BStar, zeros(2), zeros(2), ABar, zeros(2), beta, alpha_Gb, cStar_Gb, tStar_Gbps); 

PlotSimulatedHybridSystem(X, C, E, M, NSubintPerInt, cStar_Gb);

%% Evaluate our control policy vs. TCP-CoDel algorithm under real conditions

TCPCoDel = 0;

EvaluatePolicy(TCPCoDel);





    
        
        