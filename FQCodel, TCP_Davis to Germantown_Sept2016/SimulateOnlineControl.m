%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Simulates online control for mode q1.
%Sigma can be set to zeros(n) in main script to remove noise.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [X, U] = SimulateOnlineControl(K, x_k, A, B, Sigma)

[n, ~] = size(A);

X = zeros(n,K); X(:,1) = x_k; %Observation matrix, ScriptO(:,k) = x_k.

U = zeros(K-1,1); %U(k) = u_k.

for k = 1:K-1
   
    if sum(B) ~= 0, U(k) = GetOptimalControl(A, B, k, X);
        
    else U(k) = 0; %B is set to (0, 0)^T if our control algorithm is not applicable.
        
    end
    
    X(:,k+1) = GetNextState(A, X(:,k), B, U(k), Sigma);
   
end
    


