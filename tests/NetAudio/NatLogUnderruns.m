%% Einlesen der Logs
close all;
clear all;
ChannelVec = [2];
BlockSize = '32';
NetAudioLogClient = dir(['NetAudioLogClient_BS' BlockSize '*.txt']);
NetAudioLogClient = {NetAudioLogClient.name};
NetAudioLogClientTab = readtable(NetAudioLogClient{1}, 'FileType', 'text', 'Delimiter', '\t');
minTime = NetAudioLogClientTab.WorldTimeStamp(1);
maxTime = NetAudioLogClientTab.WorldTimeStamp(end);
for k=2:numel(NetAudioLogClient) 
    temp = readtable(NetAudioLogClient{k}, 'FileType', 'text', 'Delimiter', '\t');
    NetAudioLogClientTab = [NetAudioLogClientTab; temp];
    minTime = min(minTime, temp.WorldTimeStamp(1));
    maxTime = max(maxTime, temp.WorldTimeStamp(end));
end
NetAudioLogStream = dir(['NetAudioLogStream_BS' BlockSize '*.txt']);
NetAudioLogStream = {NetAudioLogStream.name};
NetAudioLogStreamTab = readtable(NetAudioLogStream{1}, 'FileType', 'text', 'Delimiter', '\t');
for k=2:numel(NetAudioLogStream) 
    temp = readtable(NetAudioLogStream{k}, 'FileType', 'text', 'Delimiter', '\t');
    NetAudioLogStreamTab = [NetAudioLogStreamTab; temp];
end
NetAudioLogBaseData = dir(['NetAudioLogBaseData_BS' BlockSize '*.txt']);
NetAudioLogBaseData = {NetAudioLogBaseData.name};
NetAudioLogBaseData = readtable(NetAudioLogBaseData{1}, 'FileType', 'text', 'Delimiter', '\t');
%% Berechne Underruns
TimeStream = NetAudioLogStreamTab.WorldTimeStamp - minTime;
TimeState = [TimeStream NetAudioLogStreamTab.StreamingStatus];
TimeUnderrun = TimeState(find(TimeState(:,2)==3), 1);
TimeUnderrun = sortrows(TimeUnderrun, 1);
%% Save Base Data
Channel = NetAudioLogBaseData.Channel(1);
SampleRate = NetAudioLogBaseData.Samplerate(1);
BufferSize = NetAudioLogBaseData.BufferSize(1);
RingBufferSize = NetAudioLogBaseData.RingBufferSize(1);
TargetSampleLatency = NetAudioLogBaseData.TargetSampleLatency(1);
%% Calculate Latency
DiffTime = diff(NetAudioLogClientTab.WorldTimeStamp * 1000);
DiffTime = [NetAudioLogClientTab.WorldTimeStamp(2:end) - minTime DiffTime NetAudioLogClientTab.ProtocolStatus(2:end) NetAudioLogClientTab.Channel(2:end)];
LatenzWaiting = DiffTime(find(DiffTime(:,3)==221), (1:4));
LatenzRunnning = DiffTime(find(DiffTime(:,3)==222), (1:4));
LatenzRunnning = sortrows(LatenzRunnning,1);
%% Get Anzahl Dropouts
plotMean = ChannelVec;
AnzUnderrun = ChannelVec;
channel = [];
latenz = [];
for k = ChannelVec
     plotTime = LatenzRunnning(find(LatenzRunnning(:,4)==k), 1);
     plotLatency = LatenzRunnning(find(LatenzRunnning(:,4)==k), 2);
     plotChannel = LatenzRunnning(find(LatenzRunnning(:,4)==k), 4);
     plotMean(k) = round(mean(plotLatency(2:end)),3);
     underruns = TimeUnderrun < plotTime(end) & TimeUnderrun > plotTime(1);
     AnzUnderrun(k) = sum(underruns);
end
%% Plot Anzahl der Underruns
plot(AnzUnderrun(:,1), AnzUnderrun(:,2), '*')
title(['Anzahl der Underruns bei x Kanälen'])
xlabel('Anzahl der Kanle')
ylabel('Anzahl der Underruns')