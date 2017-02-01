%% Einlesen der Logs
close all;
clear all;
blockSize = 32;
NetAudioLogNet = dir('NetAudioLogNet*.txt');
NetAudioLogNet = {NetAudioLogNet.name};
NetAudioLogNetTab = readtable(NetAudioLogNet{1}, 'FileType', 'text', 'Delimiter', '\t');
for k=2:numel(NetAudioLogNet) 
    temp = readtable(NetAudioLogNet{k}, 'FileType', 'text', 'Delimiter', '\t');
    NetAudioLogNetTab = [NetAudioLogNetTab; temp];
end
NetAudioLogStream = dir('NetAudioLogStream*.txt');
NetAudioLogStream = {NetAudioLogStream.name};
NetAudioLogStreamTab = readtable(NetAudioLogStream{1}, 'FileType', 'text', 'Delimiter', '\t');
for k=2:numel(NetAudioLogStream) 
    temp = readtable(NetAudioLogStream{k}, 'FileType', 'text', 'Delimiter', '\t');
    NetAudioLogStreamTab = [NetAudioLogStreamTab; temp];
end
NetAudioLogClient = dir('NetAudioLogClient*.txt');
NetAudioLogClient = {NetAudioLogClient.name};
NetAudioLogClientTab = readtable(NetAudioLogClient{1}, 'FileType', 'text', 'Delimiter', '\t');
for k=2:numel(NetAudioLogClient) 
    temp = readtable(NetAudioLogClient{k}, 'FileType', 'text', 'Delimiter', '\t');
    NetAudioLogClientTab = [NetAudioLogClientTab; temp];
end
NetAudioLogBaseData = dir('NetAudioLogBaseData*.txt');
NetAudioLogBaseData = {NetAudioLogBaseData.name};
NetAudioLogBaseData = readtable(NetAudioLogBaseData{1}, 'FileType', 'text', 'Delimiter', '\t');


% Save Base Data
Channel = NetAudioLogBaseData.Channel(1);
SampleRate = NetAudioLogBaseData.Samplerate(1);
BufferSize = NetAudioLogBaseData.BufferSize(1);
RingBufferSize = NetAudioLogBaseData.RingBufferSize(1);
TargetSampleLatency = NetAudioLogBaseData.TargetSampleLatency(1);

minTime = min(NetAudioLogStreamTab.WorldTimeStamp(1),NetAudioLogNetTab.WorldTimeStamp(1));
TimeNet = NetAudioLogNetTab.WorldTimeStamp - minTime;
TimeStream = NetAudioLogStreamTab.WorldTimeStamp - minTime;

TimeState = [TimeStream NetAudioLogStreamTab.StreamingStatus];
TimeUnderrun = TimeState(find(TimeState(:,2)==3), 1);
TimeOverrun = TimeState(find(TimeState(:,2)==4), 1);
TimeStream = TimeState(find(TimeState(:,2)==2), 1);

%% Plot Freie Smaples und Bufferstatus
Data = [NetAudioLogNetTab.WorldTimeStamp NetAudioLogNetTab.FreeSamples; NetAudioLogStreamTab.WorldTimeStamp NetAudioLogStreamTab.FreeSamples];
Data = sortrows(Data,1 );
Data(:,1) = Data(:,1) - minTime();
Data(:,2) = (Data(:,2)) / BufferSize;
medianBlock = (max(Data(:,2)) - min(Data(:,2))) / 2;
subplot(2,2,1:2)
plot( Data(:,1), Data(:,2) )
hold on
plot( TimeUnderrun, zeros(size(TimeUnderrun)) + max(Data(:,2)),'r*')
plot( TimeOverrun, zeros(size(TimeOverrun)) + min(Data(:,2)),'r+')
title(['Freie Blöcke im Ring Buffer (' num2str(RingBufferSize) ' Samples)'])
xlabel('Zeit in s')
ylabel('Anzahl der Blöcke')

%% Calculate Latency
DiffTime = diff(NetAudioLogClientTab.WorldTimeStamp * 1000);
median = mean(DiffTime(:,1));
medianVec = zeros(size(DiffTime(:,1))); 
medianVec = medianVec + median;
DiffTime = [NetAudioLogClientTab.WorldTimeStamp(2:end) - minTime DiffTime NetAudioLogClientTab.ProtocolStatus(2:end)];
DiffTime = sortrows(DiffTime,3);
LatenzWaiting = DiffTime(find(DiffTime(:,3)<222), (1:2));
LatenzRunnning = DiffTime(find(DiffTime(:,3)>221), (1:2));
medianRunning = mean(LatenzRunnning(:,2));
medianRunningVec = zeros(size(LatenzRunnning(:,1))); 
medianRunningVec = medianRunningVec + medianRunning;
sollLatenz = (BufferSize / SampleRate) * 1000;
sollLatenzVec = zeros(size(LatenzRunnning(:,1))); 
sollLatenzVec = sollLatenzVec + sollLatenz;

%% Plot Latency
subplot(2,2,1:4)
plot( LatenzRunnning(:,1), LatenzRunnning(:,2), 'b.')
hold on
%plot( LatenzRunnning(:,1), medianRunningVec, 'r')
%plot( LatenzRunnning(:,1), sollLatenzVec, 'g')
%plot( TimeUnderrun, zeros(size(TimeUnderrun)) + sollLatenz,'r*')
%plot( TimeOverrun, zeros(size(TimeOverrun)) + medianRunning,'r*')
AnzahlUnderruns = size(TimeUnderrun);
AnzahlUnderruns = AnzahlUnderruns(1);
AnzahlUnderruns = num2str(AnzahlUnderruns);
RelativeUnderruns = 100 * size(TimeUnderrun) / size(TimeState);
Durchsatz = [num2str((32 * SampleRate * Channel)/1000) ' kbit/s'];
title(['Latenz pro Block (' num2str(BufferSize) ' Samples) bei ' num2str(Channel) ' Kanälen'])
legend('Latenz', ['Latenz (' num2str(medianRunning) ' ms)'], ['SollLatenz (' num2str(sollLatenz) ' ms)' ], ['Underruns (Anz. ' AnzahlUnderruns ' - ' num2str(RelativeUnderruns) '%)'], 'Overruns')
xlabel('Zeit in s')
ylabel('Latenz in ms')
legend('show') 
name = ['Latency' num2str(BufferSize) '  ' num2str(RingBufferSize) ' - ' num2str(Channel)];
print(name,'-dsvg')
