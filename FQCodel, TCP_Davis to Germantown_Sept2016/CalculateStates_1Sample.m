%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Calculates states from a vector of observations of latency and a vector of observations of throughput.

%StateVectors_1Sample(:,j) = jth [l - l* (ms); t - t* (Gbps)] (j *not* = time point, data may correspond to slow start or a different mode)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function StateVectors_1Sample = CalculateStates_1Sample(l_mus, lStar_ms, t_bps, tStar_Gbps)

l_ms = Microsec_To_Millisec(l_mus);

t_Gbps = Bits_To_Gigabits(t_bps);

x1 = transpose(l_ms) - lStar_ms; %row vector of x1 = l - l* (ms)

x2 = transpose(t_Gbps) - tStar_Gbps; %row vector of x2 = t - t* (Gbps)

StateVectors_1Sample = [x1; x2];

