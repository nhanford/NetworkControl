%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Computes dynamics matrix using [mode q0 = 'c not flat' + congestion avoidance] state vectors.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function AStar = ComputeAStar(StateVectors_q0)

[ n, ~ ] = size(StateVectors_q0{1});

[X, XPlus] = GetMatrices_For_AStar_ID(StateVectors_q0);

cvx_begin

    variable A(n,n)

    minimize ( norm( A*X - XPlus, 'fro' ) )

cvx_end

AStar = A;
