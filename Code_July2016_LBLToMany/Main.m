%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Simulates autonomous *auto* (i.e., baseline) and controlled *ctrl* (i.e., new algorithm) hybrid system (FQ ON, Berkeley -> Boulder).
%Two-continuous-state model: x1 = t - t* (Gb/s), x2 = l - l* (cs).
%Mode q0: FQ passive + congestion avoidance, mode q1: FQ active + congestion avoidance.
%July 7, 2016
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

close all; clear all; clc;

%Preliminary script: computes state vectors, sets constants
Prelim_FQON_CO;

%Stores intervals specific to discrete modes.
[Intervals_q0, Intervals_q1] = StoreIntervalsFQPassiveActiveCA_CO(NTrials);

%Compute linear dynamics (auto/ctrl-q0, ctrl-q1) and sample covariance (auto/ctrl-q0) using FQ passive data ('Intervals_q0').
[AStar, Sigma_q0] = ComputeLinearDynamics(S, Intervals_q0, SenderCongWindSize_Gb);

%Compute linear dynamics and sample covariance for auto-q1 using FQ active data ('Intervals_q1').
[AStar_auto_q1, Sigma_auto_q1] = ComputeLinearDynamics(S, Intervals_q1, SenderCongWindSize_Gb);

%Compute control matrix and sample covariance for ctrl-q1 using FQ active data ('Intervals_q1') and passive linear dynamics ('A*').
[B_ctrl_q1, Sigma_ctrl_q1] = ComputeControlMatrix(S, Intervals_q1, AStar, SenderCongWindSize_Gb, lStar_cs);

% %Simulate hybrid system.
% %System type : auto (baseline)
% auto = 1; Sigma_ctrl_q1 = zeros(n); Sigma_q0 = zeros(n); Sigma_auto_q1 = zeros(n);
% [ScriptO_auto, C_auto, E_auto, M_auto, U_auto] = SimulateHybridSystem(auto, K, x_k, NSubintPerInt, c_k, AStar, B_ctrl_q1, Sigma_ctrl_q1, Sigma_q0, AStar_auto_q1, Sigma_auto_q1, beta, alpha_Gb, c_switch, tStar_Gbps);
% str_auto = 'Autonomous TCP+FQ hybrid system simulation: Berkeley \rightarrow Boulder';
% PlotSimulatedHybridSystem(ScriptO_auto, C_auto, E_auto, M_auto, str_auto, NSubintPerInt, c_switch);

%System type : ctrl (new algorithm)
auto = 0;
[ScriptO_ctrl, C_ctrl, E_ctrl, M_ctrl, U_ctrl] = SimulateHybridSystem(auto, K, x_k, NSubintPerInt, c_k, AStar, B_ctrl_q1, Sigma_ctrl_q1, Sigma_q0, AStar_auto_q1, Sigma_auto_q1, beta, alpha_Gb, c_switch, tStar_Gbps);
str_ctrl = 'Controlled TCP+FQ hybrid system simulation: Berkeley \rightarrow Boulder';
PlotSimulatedHybridSystem(ScriptO_ctrl, C_ctrl, E_ctrl, M_ctrl, str_ctrl, NSubintPerInt, c_switch);
