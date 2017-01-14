%Computes control matrix for two-continuous-state model.
%State vectors, control input from FQ active data ('q1').
%Dynamics from FQ passive data ('q0').

function [B, Sigma] = ComputeControlMatrix(S, Intervals_q1, AStar_q0, SenderCongWindSize_Gb, lStar_cs)
%S = state vectors for a specific destination.

[ ~, n ] = size( AStar_q0 );

[ X_q1, XPlus_q1, C_Gb_q1 ] = ShiftTimeHorizon( S, Intervals_q1, SenderCongWindSize_Gb );
%C contains sender congestion window size (gigabits) for same time points as in X.
%FQ pacing rate_k = 2*sender congestion window size_k/latency_k

row_l = n;

Latency_cs_q1 = X_q1( row_l, : ) + lStar_cs*ones( 1, length(C_Gb_q1) ); %latency_k = (l_k - l*) + l*

U_q1 = 2*C_Gb_q1./Latency_cs_q1; %FQ pacing rate estimate (gigabits/cs)

cvx_begin

    variable ControlMatrix(n,1);
    
    minimize ( norm( XPlus_q1 - AStar_q0*X_q1 - ControlMatrix*U_q1, 'fro' ) );
    
cvx_end

B = ControlMatrix;

%Compute sample covariance.
Error = XPlus_q1 - AStar_q0*X_q1 - B*U_q1;
[~,NErrorSamples] = size(Error);
CenteredError = Error - repmat(mean(Error,2),1,NErrorSamples); %mean(Error,2) = mean of each row.
Sigma = 1/NErrorSamples*(CenteredError*CenteredError');

