%Computes optimal control at time k using LQR.

function uStar_k = GetOptimalControl(AStar, B, k, X)

[n, ~] = size(AStar);
   
N_X = 5; %Number of time points used to approximate mean continuous state.

%Sample mean at time k using the most recent "LengthToStore" observations.
if k < N_X, mu_k = mean( X( :, 1:k ), 2 ); %NSamples_k = k;
else mu_k = mean( X( :, (k+1-N_X):k ), 2 ); %NSamples_k = LengthToStore;
end

%penalty = (k+1)^2;
%penalty = 1; %bad performance with this penalty, (x1,x2) goes away from origin.
%penalty = k+1;
%penalty = exp(k);
penalty = (norm(X(:,k),2))^2;
%penalty = exp(norm(X(:,k),2)); %Similar behavior to above but higher control input

cvx_begin

    variables xhat_kPlus1(n,1) u_k(1,1);
        
    %minimize ( quad_form( xhat_kPlus1, (k+1)^2*eye(n) ) + quad_form(xhat_kPlus1 - ( NSamples_k*mu_k + xhat_kPlus1 )/( NSamples_k + 1 ), (k+1)^2*eye(n) ) + square( u_k ) ); %More complex notion of mean.
    minimize ( quad_form( xhat_kPlus1, penalty*eye(n) ) + quad_form( xhat_kPlus1 - mu_k, penalty*eye(n) ) + square( u_k ) ); 
        
    subject to
        
        xhat_kPlus1 == AStar*X(:,k) + B*u_k; %X(:,k) = x_k
            
        u_k >= 0;
       
cvx_end

uStar_k = u_k;

%mean( Matrix , 2 ) : mean of each row.
%Y = std(X,1) normalizes by N (sample size) and produces the square root of the second moment of the sample about its mean.
%(A*x-b)'*Q*(Ax-b) is replaced with quad_form( A * x - b, Q )
%conj( x ) .* x is replaced with square( x )
%y’ * y is replaced with sum_square( y )