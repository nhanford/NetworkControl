%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Stores intervals specific to discrete modes, destination: CO.*

%q0 = FQ passive + congestion avoidance
%q1 = FQ active + congestion avoidance

%FQ passive = FQ is NOT limiting the sending rate. Congestion window size INCREASES with time.
%FQ active = FQ IS limiting the sending rate. Congestion window size CONSTANT with time.

%*FQ ON, all trials, well-defined intervals (i.e., STOP time - START time > 1)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [Intervals_q0_CO, Intervals_q1_CO] = StoreIntervalsFQPassiveActiveCA_CO(NTrials)

%Out of slow start by time 2.
Trial1_FQPassive_CO = [2 7; 8 15; 16 34; 52 60]; %col 1 = START time, col 2 = STOP time.
Trial1_FQActive_CO = [34 51];

%Out of slow start by time 4.
Trial2_FQPassive_CO = [10 28];
Trial2_FQActive_CO = [4 9; 28 60];

%Out of slow start at time 2.
Trial3_FQPassive_CO = [];
Trial3_FQActive_CO = [2 60];

%Out of slow start by time 2.
Trial4_FQPassive_CO = [2 11; 12 21; 22 31; 32 49];
Trial4_FQActive_CO = [49 60];

Intervals_q0_CO = cell(NTrials,1); 
Intervals_q0_CO{1} = Trial1_FQPassive_CO; Intervals_q0_CO{2} = Trial2_FQPassive_CO; Intervals_q0_CO{3} = Trial3_FQPassive_CO; Intervals_q0_CO{4} = Trial4_FQPassive_CO;

Intervals_q1_CO = cell(NTrials,1);
Intervals_q1_CO{1} = Trial1_FQActive_CO; Intervals_q1_CO{2} = Trial2_FQActive_CO; Intervals_q1_CO{3} = Trial3_FQActive_CO; Intervals_q1_CO{4} = Trial4_FQActive_CO;
