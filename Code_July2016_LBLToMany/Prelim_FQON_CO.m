%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Preliminary script for model identification, simulation, and control of system: FQ ON, Boulder, CO.
%Two-continuous-state model: x1 = t - t* (Gb/s), x2 = l - l* (cs).
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%Select FQ type and destination.
FQON = 1; %FQ ON
file_trial1 = 4; %file index of trial 1 specifies destination of interest: Boulder, CO.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Compute state vectors for FQ ON, destination: CO.
NTrials = 4;
tStar_bps = 10^(10); %bits/s
n = 2; 
[DataFileAddresses, Destinations] = GetDataFileAddressesAndDestinations(FQON); %FQ ON.
[S, SenderCongWindSize_Gb] = ComputeStateVectors(DataFileAddresses, file_trial1, length(Destinations), NTrials, tStar_bps); %FQ ON, all time points, CO, Gb = gigabits.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Simulation constants
[~, l_mus, ~, ~, ~, c_B] = ExtractRawData(DataFileAddresses{file_trial1}); %FQ ON, CO, trial 1
lStar_cs = ConvertMicrosecToCentisec(min(l_mus)); %microseconds -> centiseconds
tStar_Gbps = ConvertbitsToGigabits(tStar_bps); %Gigabits/s
K = 60; %Time horizon length (s)
%c_switch = ConvertBytesToGigabits(8.9*10^7); %Value of c that triggers mode switch; approx. c at which FQ is observed to switch passive -> active, CO = 8.9*10^7 Bytes = 0.712 Gigabits.
c_switch = 0;
NSubintPerInt = round(100/lStar_cs); %100 cs / l* cs = number of subintervals in tau_k (length = 1s)
%In each subinterval, one packet drop may occur -> c dynamics.
beta = 3/4; %Multiplicative decrease of c
alpha_Gb = 0.0169*(1/NSubintPerInt); %Additive increase of c = 0.0169 Gb per interval * 1 interval per 33 subintervals (units: Gb per subinterval)
%mss_Gb = ConvertBytesToGigabits(8948); %Maximum segment size, bytes -> gigabits (destination specific)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Initialize mode q1 = FQ active + congestion avoidance, 'bad' continuous state
%c_k = c_switch*1.25; %initial sender congestion window size (Gb)
c_k = 1;
x_k = [-10; 20]; %initial continouous state (Gb/s, cs)












%Former initialization from data
%k = 2;
%x_k = S{1}(:,k); %trial 1, time 2, FQ ON, CO -> mode q0
%c_Gb_k = ConvertBytesToGigabits(c_B(k)); %initial sender congestion window size, Bytes -> Gigabits





