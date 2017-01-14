%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Simulates FQ ON hybrid system, 'autonomous' *auto* (i.e., baseline) or 'controlled' *ctrl* (i.e., our new algorithm).
%Two-continuous-state model: x1 = t - t* (Gb/s), x2 = l - l* (cs).
%Mode q0: FQ passive + congestion avoidance, mode q1: FQ active + congestion avoidance.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%DEFINITIONS
%kth interval = tau_k = [k, k+1] (s)
%For time horizon 1:K, there are K-1 intervals.
%M(k) = mode in tau_k (1 = q1, 0 = q0)
%ScriptO(:,k) = observed state at start of tau_k
%ith subinterval of tau_k = tau_k_i.
%C((k-1)*NSubintPerInt + i) = c at start of tau_k_i
%E((k-1)*NSubintPerInt + i) = one packet is dropped in tau_k_i, yes = 1/no = 0, E for "event"
%c = sender congestion window size
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [X, C, E, M, UStar] = SimulateHybridSystem(auto, K, x_k, NSubintPerInt, c_k, AStar, B_ctrl_q1, Sigma_ctrl_q1, Sigma_q0, AStar_auto_q1, Sigma_auto_q1, beta, alpha_Gb, c_switch, tStar_Gbps)  
%auto = 1 -> TCP/FQ baseline operation, auto = 0 -> controlled operation using our new algorithm.

[n, ~] = size(AStar);

%Initialization
C = zeros(1,(K-1)*NSubintPerInt+1); %computed for each subinterval plus endpoint
E = zeros(1,(K-1)*NSubintPerInt); %yes=1/no=0 packet drop for each subinterval, #subintervals total = #intervals total * #subintervals/interval
M = zeros(1,K-1); %specified for each interval
UStar = zeros(1,K-1); %we apply control at all time points except K
X = zeros(n,K); %Contains observed continuous state vectors, X(:,k) = x_k.

for k = 1:K
    
    %Observe packet drops on [k-1, k), sender congestion window size on [k-1, k], and continouous state at time k.
    if k > 1 %Observations prior to and including time K
        [C, c_k, E] = SimulatePacketDrops(C, E, alpha_Gb, beta, k-1, NSubintPerInt);
        X(:,k) = GetNextState(A, X(:,k-1), B, UStar(k-1), Sigma);
        
        %Limit throughput at time k according to sender congestion window size on [k-1,k).
        X(:,k) = Connect_X_C_dynamics(X(:,k), tStar_Gbps, C, k-1, NSubintPerInt);
        
    else %Initial conditions, k = 1
        C(k) = c_k;
        X(:,k) = x_k;
    end

    %Determine mode on [k, k+1) and set dynamics accordingly.
    if k <= K-1 %Last mode is on [K-1, K), stored in M(K-1).
        if c_k >= c_switch %mode q1
            M(k) = 1;
            if auto %mode q1, auto
                A = AStar_auto_q1; Sigma = Sigma_auto_q1; B = zeros(n,1); UStar(k) = 0;
            else %mode q1, ctrl
                A = AStar; Sigma = Sigma_ctrl_q1; B = B_ctrl_q1; UStar(k) = GetOptimalControl(AStar, B, k, X);
            end
        else %mode q0, auto/ctrl systems have same continuous q0-dynamics.
            M(k) = 0; 
            A = AStar; Sigma = Sigma_q0; B = zeros(n,1); UStar(k) = 0;
        end
    end
    
end
%Notes:
%Packet drops on [k-1, k) determine sender congestion window size at time k, c_k.
%c_k determines mode on [k, k+1).












