%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Outputs all throughput, round-trip-time, congestion window size time-trajectories with retransmit events labeled for FQ type ON or OFF.
%Assumes that end time of interval is the exact time of measurement.

%FQON = 1 -> TCP + FQ
%FQON = 0 -> only TCP

%ObsType = 1 -> Throughput (bits per sec)
%ObsType = 0 -> RTT (microsec)
%ObsType = -1 -> snd_cwnd (bytes)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function ViewData(FQON, NTrials, Fig_Tput, Fig_RTT, Fig_CongW)

[DataFileAddresses, Destinations] = GetDataFileAddressesAndDestinations(FQON);

for file = 1:length(DataFileAddresses)

    [Time_Sec, RTT_MicroSec, Thruput_BitsPerSec, ~, Retransmits_Packets, CongWindow_Bytes] = ExtractRawData(DataFileAddresses{file});
    
    ObsType = 1; FigNum = Fig_Tput;
    PlotObservations(Time_Sec, Thruput_BitsPerSec, ObsType, FigNum, Retransmits_Packets, FQON, Destinations, NTrials, file);
        
    ObsType = 0; FigNum = Fig_RTT;
    PlotObservations(Time_Sec, RTT_MicroSec, ObsType, FigNum, Retransmits_Packets, FQON, Destinations, NTrials, file);
    
    ObsType = -1; FigNum = Fig_CongW;
    PlotObservations(Time_Sec, CongWindow_Bytes, ObsType, FigNum, Retransmits_Packets, FQON, Destinations, NTrials, file);

end
