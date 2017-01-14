%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Computes optimal control at time k using LQR-based approach.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function uStar_k = GetOptimalControl(AStar, BStar, k, X)

[n, ~] = size(AStar);
   
N_X = 5; %Number of time points used to approximate mean continuous state.

%Sample mean at time k using the most recent N_X observations.
if k < N_X, mu_k = mean( X( :, 1:k ), 2 ); %NSamples_k = k;
else mu_k = mean( X( :, (k+1-N_X):k ), 2 ); %NSamples_k = N_X;
end

penalty = (norm(X(:,k),2))^2;

cvx_begin

    variables xhat_kPlus1(n,1) u_k(1,1);
        
    minimize ( quad_form( xhat_kPlus1, penalty*eye(n) ) + quad_form( xhat_kPlus1 - mu_k, penalty*eye(n) ) + square( u_k ) ); 
        
    subject to
        
        xhat_kPlus1 == AStar*X(:,k) + BStar*u_k; %X(:,k) = x_k
            
        u_k >= 0;
       
cvx_end

uStar_k = u_k;

%mean( Matrix , 2 ) : mean of each row.
%(A*x-b)'*Q*(Ax-b) is replaced with quad_form( A * x - b, Q )
%conj( x ) .* x is replaced with square( x )