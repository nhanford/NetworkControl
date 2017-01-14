%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Measures 'noisy' state at time k+1 given state at time k and control input at time k. 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
function x_kPlus1 = GetNextState(AStar, x_k, BStar, u_k, Sigma)
    
    n = length(x_k); state_latency = 1; state_throughput = n;
    
    e_k = transpose( mvnrnd(zeros(n,1),Sigma) );

    if e_k(state_throughput) > 0, e_k(state_throughput) = -e_k(state_throughput); end %x1 more negative is destabilizing.

    if e_k(state_latency) < 0, e_k(state_latency) = -e_k(state_latency); end %x2 more positive is destabilizing.

    x_kPlus1 = AStar*x_k + BStar*u_k + e_k; %Add zero-mean 'destabilizing' noise.