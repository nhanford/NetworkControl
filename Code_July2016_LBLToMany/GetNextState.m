%Measures 'noisy' state at time kStart+1 given state at time kStart and control input at time kStart. 
    
function x_kStartPlus1 = GetNextState(AStar, x_kStart, B, u_kStart, Sigma)
    
    n = length(x_kStart); state_throughput = 1; state_latency = n;
    
    e_kStart = transpose( mvnrnd(zeros(n,1),Sigma) );

    if e_kStart(state_throughput) > 0, e_kStart(state_throughput) = -e_kStart(state_throughput); end %x1 more negative is destabilizing.

    if e_kStart(state_latency) < 0, e_kStart(state_latency) = -e_kStart(state_latency); end %x2 more positive is destabilizing.

    x_kStartPlus1 = AStar*x_kStart + B*u_kStart + e_kStart; %Add zero-mean 'destabilizing' noise.