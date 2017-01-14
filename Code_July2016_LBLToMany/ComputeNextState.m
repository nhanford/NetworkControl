%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Computes the state vector at time k+1, using the previous at time k.
%Ten-continuous-state model
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function x_kPLUS1 = ComputeNextState(x_k, A, Sigma, B, UStar, k, n, T)

epsilon_k = transpose(mvnrnd(zeros(n,1),Sigma)); %Zero-mean gaussian noise with known covariance.

x_kPLUS1 = A*x_k + g(x_k, T) + B*UStar(k) + epsilon_k; 


