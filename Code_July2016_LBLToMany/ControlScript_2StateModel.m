%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Simulates and plots controlled single-mode q1 : FQ active + congestion avoidance (FQ ON, Berkeley -> Boulder).
%Two-continuous-state model: x1 = t - t* (Gb/s), x2 = l - l* (cs).
%July 1, 2016
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
close all; clear all; clc;

%Preliminary script: computes state vectors, sets constants
Prelim_FQON_CO; 

%Stores intervals specific to discrete modes, destination: CO.
[Intervals_q0, Intervals_q1] = StoreIntervalsFQPassiveActiveCA_CO(NTrials);

%Compute linear dynamics using FQ passive data ('Intervals_q0').
[AStar, ~] = ComputeLinearDynamics(S, Intervals_q0, SenderCongWindSize_Gb);

%Compute control matrix using FQ active data ('Intervals_q1') and known continuous dynamics ('A*').
[B_ctrl_q1, Sigma_ctrl_q1] = ComputeControlMatrix(S, Intervals_q1, AStar, SenderCongWindSize_Gb, lStar_cs);

%Perform controllability test.
ControllabilityMatrix = ctrb(AStar, B_ctrl_q1);
EigenvaluesControllabilityMatrix = eig(ControllabilityMatrix);

%Simulate online control.
Sigma_ctrl_q1 = zeros(n); %Removes noise.
[X, U] = SimulateOnlineControl(K, x_k, AStar, B_ctrl_q1, Sigma_ctrl_q1);

%Visualize observations from simulating mode q1.
PlotSingleModeSimulation(X);







