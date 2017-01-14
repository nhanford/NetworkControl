%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Computes B* using [mode q1 = 'c flat' + congestion avoidance (ca)] state vectors and A*.

%A* was identified using [mode q0 = 'c not flat' + ca] state vectors.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function BStar = ComputeBStar(AStar, StateVectors_q1)

[U, X, XPlus] = GetMatrices_For_q1_Dynamics_ID(StateVectors_q1);

cvx_begin

    variable B(2,1)
    
    minimize( norm( XPlus - AStar*X - B*U, 'fro' ) )

cvx_end

BStar = B;

