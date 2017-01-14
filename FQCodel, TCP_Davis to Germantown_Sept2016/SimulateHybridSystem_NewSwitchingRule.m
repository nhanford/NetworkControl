%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Simulates hybrid system, TCP-CoDel or our controller.
%Two-continuous-state model: x1 = l - l* (ms), x2 = t - t* (Gb/s)
%TCPCoDel = 1 -> TCP-CoDel controller, TCPCoDel = 0 -> Our controller
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%DEFINITIONS
%kth interval = tau_k = [k, k+1] (s)
%For time horizon 1:K, there are K-1 intervals.
%M(k) = mode in tau_k (1 = q1, 0 = q0)
%X(:,k) = observed state at start of tau_k
%ith subinterval of tau_k = tau_k_i.
%C((k-1)*NSubintPerInt + i) = c at start of tau_k_i
%E((k-1)*NSubintPerInt + i) = one packet is dropped in tau_k_i, yes = 1/no = 0, E for "event"
%c = sender congestion window size
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [X, C, E, M, UStar] = SimulateHybridSystem_NewSwitchingRule(TCPCoDel, K, x_k, NSubintPerInt, c_k, AStar, BStar, Sigma_Ours_q1, Sigma_q0, ABar, Sigma_TCPCoDel_q1, beta, alpha_Gb, cStar_Gb, tStar_Gbps) 
[n, ~] = size(AStar);

%Initialization
C = zeros(1,(K-1)*NSubintPerInt+1); %computed for each subinterval plus endpoint
E = zeros(1,(K-1)*NSubintPerInt); %yes=1/no=0 packet drop for each subinterval, #subintervals total = #intervals total * #subintervals/interval
M = zeros(1,K-1); %specified for each interval
UStar = zeros(1,K-1); %we apply control at all time points except K
X = zeros(n,K); %Contains observed continuous state vectors, X(:,k) = x_k.
tcounter = 0; lcounter = 0;

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

    lMAX = 15; tMIN = -9.98;
    %Determine mode on [k, k+1) and set dynamics accordingly.
    if k <= K-1 %Last mode is on [K-1, K), stored in M(K-1).
        if X(1,k) >= lMAX && lcounter <= 5 %LATENCY TOO LARGE -> MODE Q1
            M(k) = 1;
            if TCPCoDel %mode q1, TCP-CoDel controller
                A = ABar; Sigma = Sigma_TCPCoDel_q1; B = zeros(n,1); UStar(k) = 0;
            else %mode q1, Our controller
                A = AStar; Sigma = Sigma_Ours_q1; B = BStar; UStar(k) = GetOptimalControl(AStar, B, k, X);
            end
            lcounter = lcounter + 1;
        elseif X(2,k) <= tMIN && tcounter <= 5%THROUGHPUT TOO SMALL -> MODE Q0
            M(k) = 0; 
            A = AStar; Sigma = Sigma_q0; B = zeros(n,1); UStar(k) = 0;
            tcounter = tcounter + 1;
        else
            if lcounter == 6, lcounter = 0; end
            if tcounter == 6, tcounter = 0; end
        end %OTHERWISE DO NOTHING
        
    end
    
end
%Notes:
%Packet drops on [k-1, k) determine sender congestion window size at time k, c_k.
%c_k determines mode on [k, k+1).












