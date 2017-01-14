%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Computes the dynamics matrix that represents the dynamical response subject to the TCP-CoDel algorithm (ABar).
    %using [mode q1 = 'c flat' + congestion avoidance (ca)] state vectors.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function ABar = ComputeABar(StateVectors_q1)

[~, X, XPlus] = GetMatrices_For_q1_Dynamics_ID(StateVectors_q1);

cvx_begin

    variable A(2,2)
    
    minimize( norm( XPlus - A*X, 'fro' ) )

cvx_end

ABar = A;