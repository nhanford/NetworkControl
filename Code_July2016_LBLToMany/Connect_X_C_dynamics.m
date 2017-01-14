%Limits throughput at time k+1 according to sender congestion window size on tau_k = [k, k+1).
%Continuous dynamics: x(1) = t - t*, x(2) = l - l*.

function x_kPlus1 = Connect_X_C_dynamics(x_kPlus1, tStar_Gbps, C, k, NSubintPerInt)

t_kPlus1 = x_kPlus1(1) + tStar_Gbps; %Gb/s

Start1 = (k-1)*NSubintPerInt + 1; %Start index of first subinterval of tau_k
StartN = k*NSubintPerInt; %Start index of last subinterval of tau_k

max_t_kPlus1 = 1/NSubintPerInt*sum(C(Start1: StartN)); %Gb/s

t_kPlus1 = min(t_kPlus1, max_t_kPlus1);

x_kPlus1(1) = t_kPlus1 - tStar_Gbps;



