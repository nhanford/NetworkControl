%Simulates online control for mode q1.
%Sigma can be set to zeros(n) in main script to remove noise.

function [X, U] = SimulateOnlineControl(K, x_k, AStar, B, Sigma)

[n, ~] = size(AStar);

X = zeros(n,K); X(:,1) = x_k; %Observation matrix, ScriptO(:,k) = x_k.

U = zeros(K-1,1); %U(k) = u_k.

for k = 1:K-1
   
    U(k) = GetOptimalControl(AStar, B, k, X);
    
    X(:,k+1) = GetNextState(AStar, X(:,k), B, U(k), Sigma);
    
end
    


