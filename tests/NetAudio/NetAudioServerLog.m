%% Konfig Kram
close all;
clear all;
BlockSize = '32';
plotServer = 2;

%% Einlesen der Logs
NetAudioLogServer = dir(['NetAudioLogServer_BS' BlockSize '*.txt']);
NetAudioLogServer = {NetAudioLogServer.name};
NetAudioLogServerTab = readtable(NetAudioLogServer{1}, 'FileType', 'text', 'Delimiter', '\t');
minTimeServer = NetAudioLogServerTab.WorldTimeStamp(1);
maxTimeServer = NetAudioLogServerTab.WorldTimeStamp(end);
channel = NetAudioLogServer{1}(28:end-4);

for k=2:numel(NetAudioLogServer) 
    temp = readtable(NetAudioLogServer{k}, 'FileType', 'text', 'Delimiter', '\t');
    NetAudioLogServerTab = [NetAudioLogServerTab; temp];
    minTimeServer = min(minTime, temp.WorldTimeStamp(1));
    maxTimeServer = max(maxTime, temp.WorldTimeStamp(end));
    channel = [channel; NetAudioLogServer{k}(28:end-4)];
end

NetAudioLogClient = dir(['NetAudioLogClient_BS' BlockSize '*.txt']);
NetAudioLogClient = {NetAudioLogClient.name};
NetAudioLogClientTab = readtable(NetAudioLogClient{1}, 'FileType', 'text', 'Delimiter', '\t');
minTimeClient = NetAudioLogClientTab.WorldTimeStamp(1);
maxTimeClient = NetAudioLogClientTab.WorldTimeStamp(end);
for k=2:numel(NetAudioLogClient) 
    temp = readtable(NetAudioLogClient{k}, 'FileType', 'text', 'Delimiter', '\t');
    NetAudioLogClientTab = [NetAudioLogClientTab; temp];
    minTimeClient = min(minTime, temp.WorldTimeStamp(1));
    maxTimeClient = max(maxTime, temp.WorldTimeStamp(end));
end
NetAudioLogClientTab.WorldTimeStamp = NetAudioLogClientTab.WorldTimeStamp - minTimeClient;
NetAudioLogServerTab.WorldTimeStamp = NetAudioLogServerTab.WorldTimeStamp - minTimeServer;


%% Protocolstatus ersetzten
Protocol = {'100', 'NP CLIENT OPEN';...
    '101', 'NP CLIENT CLOSE';...
    '111', 'NP CLIENT SENDING RINGBUFFER FREE SAMPLES';...
    '200', 'NP SERVER OPEN';...
    '201', 'NP SERVER CLOSE';...
    '211', 'NP SERVER GET RINGBUFFER FREE SAMPLES';...
    '222', 'NP SERVER SENDING SAMPLES'};
%NumPro = zeros(size(NetAudioLogServerTab.ProtocolStatus));

%% Daten sammlen Client
Time100 = NetAudioLogClientTab.WorldTimeStamp(find(NetAudioLogClientTab.ProtocolStatus == 100));
Time101 = NetAudioLogClientTab.WorldTimeStamp(find(NetAudioLogClientTab.ProtocolStatus == 101));
Time111 = NetAudioLogClientTab.WorldTimeStamp(find(NetAudioLogClientTab.ProtocolStatus == 111));
Time200 = NetAudioLogClientTab.WorldTimeStamp(find(NetAudioLogClientTab.ProtocolStatus == 200));
Time201 = NetAudioLogClientTab.WorldTimeStamp(find(NetAudioLogClientTab.ProtocolStatus == 201));
Time211 = NetAudioLogClientTab.WorldTimeStamp(find(NetAudioLogClientTab.ProtocolStatus == 211));
Time222 = NetAudioLogClientTab.WorldTimeStamp(find(NetAudioLogClientTab.ProtocolStatus == 222));

TimeClient = {Time100 Time101 Time111 Time200 Time201 Time211 Time222};

%% Daten sammeln Server
Time100 = NetAudioLogServerTab.WorldTimeStamp(find(NetAudioLogServerTab.ProtocolStatus == 100));
Time101 = NetAudioLogServerTab.WorldTimeStamp(find(NetAudioLogServerTab.ProtocolStatus == 101));
Time111 = NetAudioLogServerTab.WorldTimeStamp(find(NetAudioLogServerTab.ProtocolStatus == 111));
Time200 = NetAudioLogServerTab.WorldTimeStamp(find(NetAudioLogServerTab.ProtocolStatus == 200));
Time201 = NetAudioLogServerTab.WorldTimeStamp(find(NetAudioLogServerTab.ProtocolStatus == 201));
Time211 = NetAudioLogServerTab.WorldTimeStamp(find(NetAudioLogServerTab.ProtocolStatus == 211));
Time222 = NetAudioLogServerTab.WorldTimeStamp(find(NetAudioLogServerTab.ProtocolStatus == 222));

TimeServer = {Time100 Time101 Time111 Time200 Time201 Time211 Time222};

%% Plot Protocol
legende = {};
if plotServer == 1
    % Plot Server Samples
    plots1{1} = plot([10 0],[3200 3200]);
    hold on
    plots1{2} = plot(NetAudioLogServerTab.WorldTimeStamp, NetAudioLogServerTab.FreeSamples, '-*');
    legende{1} = 'Maximal Freie Samples';
    legende{2} = 'Freie Samples Server';
else
    % Plot Client Samples
    maxSamples = 3200;
    plots2{1} = plot([10 0],[3200 3200]);
    hold on;
    plots2{2} = plot(NetAudioLogClientTab.WorldTimeStamp, NetAudioLogClientTab.FreeSamples, '-*');
    legende{1} = 'Maximal Freie Samples';
    legende{2} = 'Freie Samples Client';
end
i = 3;
j = 3;
for k = (1:7)
    if plotServer == 1
        if size(TimeServer{k}, 1) ~= 0
            plots1{i} = plot(TimeServer{k}, ones(size(TimeServer{k})),'.');
            legende{i} = Protocol{k,2};
            i = i + 1;
            
        end
        titel = 'Protokolstatus Server';
    else
        if size(TimeClient{k}, 1) ~= 0
            p = 1;
            if k == 3
                p = 1500;
            end
            plots2{j} = plot(TimeClient{k}, ones(size(TimeClient{k})),'.');
            legende{j} = Protocol{k,2};
            j = j + 1;
        end
        titel = 'Protokolstatus Client';
    end
end
title(titel)
xlabel('Zeit in s')
legend(legende);
legend('show');