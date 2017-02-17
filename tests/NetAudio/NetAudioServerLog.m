%% Einlesen der Logs
close all;
clear all;
BlockSize = '32';
NetAudioLogServer = dir(['NetAudioLogServer_BS' BlockSize '*.txt']);
NetAudioLogServer = {NetAudioLogServer.name};
NetAudioLogServerTab = readtable(NetAudioLogServer{1}, 'FileType', 'text', 'Delimiter', '\t');
minTime = NetAudioLogServerTab.WorldTimeStamp(1);
maxTime = NetAudioLogServerTab.WorldTimeStamp(end);
channel = NetAudioLogServer{1}(28:end-4);

for k=2:numel(NetAudioLogServer) 
    temp = readtable(NetAudioLogServer{k}, 'FileType', 'text', 'Delimiter', '\t');
    NetAudioLogServerTab = [NetAudioLogServerTab; temp];
    minTime = min(minTime, temp.WorldTimeStamp(1));
    maxTime = max(maxTime, temp.WorldTimeStamp(end));
    channel = [channel; NetAudioLogServer{k}(28:end-4)];
end
NetAudioLogServerTab.WorldTimeStamp = NetAudioLogServerTab.WorldTimeStamp - minTime;

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


%% Protocolstatus ersetzten
Protocol = {'100', 'NP CLIENT OPEN';...
    '101', 'NP CLIENT CLOSE';...
    '111', 'NP CLIENT SENDING RINGBUFFER FREE SAMPLES';...
    '200', 'NP SERVER OPEN';...
    '201', 'NP SERVER CLOSE';...
    '211', 'NP SERVER GET RINGBUFFER FREE SAMPLES';...
    '222', 'NP SERVER SENDING SAMPLES'};
%NumPro = zeros(size(NetAudioLogServerTab.ProtocolStatus));
Time100 = NetAudioLogServerTab.WorldTimeStamp(find(NetAudioLogServerTab.ProtocolStatus == 100));
Time101 = NetAudioLogServerTab.WorldTimeStamp(find(NetAudioLogServerTab.ProtocolStatus == 101));
Time111 = NetAudioLogServerTab.WorldTimeStamp(find(NetAudioLogServerTab.ProtocolStatus == 111));
Time200 = NetAudioLogServerTab.WorldTimeStamp(find(NetAudioLogServerTab.ProtocolStatus == 200));
Time201 = NetAudioLogServerTab.WorldTimeStamp(find(NetAudioLogServerTab.ProtocolStatus == 201));
Time211 = NetAudioLogServerTab.WorldTimeStamp(find(NetAudioLogServerTab.ProtocolStatus == 211));
Time222 = NetAudioLogServerTab.WorldTimeStamp(find(NetAudioLogServerTab.ProtocolStatus == 222));

Time = {Time100 Time101 Time111 Time200 Time201 Time211 Time222};

%% Plot Protocol
%plot(NetAudioLogServerTab.WorldTimeStamp, NetAudioLogServerTab.ProtocolStatus)
hold on;
plot(NetAudioLogServerTab.WorldTimeStamp, NetAudioLogServerTab.FreeSamples)
legende = {};
legende{1} = 'Freie Samples';
i = 2;
for k = (1:7)
    if size(Time{k}, 1) ~= 0
        plot(Time{k}, k* 100 * ones(size(Time{k})),'.')
        legende{i} = Protocol{k,2};
        i = i + 1;
    end
end
legend(legende);
legend('show');