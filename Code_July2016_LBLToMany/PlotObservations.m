%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Plots observations from a data file: throughput vs. time, RTT vs. time, or snd_cwnd vs. time.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function PlotObservations(Time_Sec, Observations, ObsType, FigNum, Retransmits_Packets, FQOn, Destinations, NTrials, file)

%ObsType = 1 -> Throughput (bits per sec)
%ObsType = 0 -> RTT (microsec)

NDestinations = length(Destinations); xMax = 61;

MAX = max(Observations); 
OrderMag = floor( log10(MAX)); %Extract order of magnitude.
yMax = MAX + 10^OrderMag;

row = GetTrialNumber(file, NTrials, NDestinations);

if ObsType == 1, yAxisName = strcat('Tput (bits/s)', ', Trial:', num2str(row)); titlestr = ' Throughput';
elseif ObsType == 0, yAxisName = strcat('RTT (\mus)', ', Trial:', num2str(row)); titlestr = ' Round trip time';
else yAxisName = strcat('s\_cwnd (bytes)', ', Trial:', num2str(row)); titlestr = ' Sender congestion window size';
end

if (FQOn == 1), toplabel = 'Data transfer from UC Berkeley, TCP + fair queueing *ON*,';
else toplabel = 'Data transfer from UC Berkeley, TCP + fair queueing *OFF*,';
end

%handle = figure(FigNum);

FigureSettings(figure(FigNum));

subplot(NTrials, NDestinations, file)

Yes_Retransmits_Indices = find(Retransmits_Packets); 
%Returns the linear indices corresponding to the nonzero entries of Retransmits_Packets.
%Retransmits_Packets > 0 -> occurance of retransmit event(s).

plot(Time_Sec, Observations,':b','linewidth',2); hold on
plot(Time_Sec(Yes_Retransmits_Indices),Observations(Yes_Retransmits_Indices),'*r','linewidth',2); hold on %Mark retransmit events.
axis([0 xMax 0 yMax]);

if (mod(file,NDestinations) == 1), ylabel(yAxisName); end
if (file <= NDestinations), title(Destinations{file}); end %Label each col by appropriate destination.
if (file > (NTrials-1)*NDestinations), xlabel('Time (s)'); end
if (file == NTrials*NDestinations), 
    %legend('Observation','Nominal','Retransmit event');
    legend('Observation','Retransmit event');
    annotation('textbox',[0.3 0.89 0.5 0.1],'String',strcat(toplabel, titlestr),'LineStyle','none');
end
