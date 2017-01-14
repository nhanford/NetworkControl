%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Computes linear time-invariant dynamics and sample covariance: Berkeley -> specified destination.
%Two-continuous-state model.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [AStar, Sigma] = ComputeLinearDynamics(S, Intervals, SenderCongWindSize_Gb)
%S = cell of state vectors for a specific destination.
%Intervals correspond to a specific discrete mode.

[n, ~] = size(S{1});

%Compute optimal linear dynamics.
[X, XPlus, ~] = ShiftTimeHorizon(S, Intervals, SenderCongWindSize_Gb);

cvx_begin
    variable A(n,n)
    minimize (norm(XPlus - A*X,'fro'))
cvx_end
AStar = A;

%Compute sample covariance.
Error = XPlus - AStar*X;
[~,NErrorSamples] = size(Error);
CenteredError = Error - repmat(mean(Error,2),1,NErrorSamples); %mean(Error,2) = mean of each row.
Sigma = 1/NErrorSamples*(CenteredError*CenteredError');

