%% Load
ITANetAudioTest_Client = readtable( 'ITANetAudioTest_Client.log', 'FileType', 'text', 'Delimiter', '\t');
ITANetAudioTest_Server = readtable( 'ITANetAudioTest_Server.log', 'FileType', 'text', 'Delimiter', '\t');
ITANetAudioTest_Client_AudioStream = readtable( 'ITANetAudioTest_Client_AudioStream.log', 'FileType', 'text', 'Delimiter', '\t');


%% Settings
RB = 512*4;
TL = 512*3;


%% Analyse
TS = min( [ ITANetAudioTest_Client.WorldTimeStamp; ITANetAudioTest_Server.WorldTimeStamp; ITANetAudioTest_Client_AudioStream.WorldTimeStamp ] );

AudioStreamingUnderrunIndices = find( ITANetAudioTest_Client_AudioStream.FreeSamples == RB );

%% Plot
plot( ITANetAudioTest_Client.WorldTimeStamp - TS, ITANetAudioTest_Client.FreeSamples - RB )
hold on

plot( ITANetAudioTest_Server.WorldTimeStamp - TS, ITANetAudioTest_Server.EstimatedFreeSamples - RB )
hold on

% Underruns
plot( ITANetAudioTest_Client_AudioStream.WorldTimeStamp( AudioStreamingUnderrunIndices ) - TS, ITANetAudioTest_Client_AudioStream.FreeSamples( AudioStreamingUnderrunIndices ) , 'r*' )

legend( 'RingBufferSamples', 'Estimated', 'Underruns' )
ylabel( 'NumSamples' )
xlabel( 'Time' )

%% Old
% NetAudioLogClient = readtable( 'NetAudioLogClient_BS32_Ch2.txt', 'FileType', 'text', 'Delimiter', '\t' );
% NetAudioLogBaseData = readtable( 'NetAudioLogBaseData_BS32_Ch2.txt', 'FileType', 'text', 'Delimiter', '\t' );
% 
% % Save Base Data
% Channel = NetAudioLogBaseData.Channel(1);
% SampleRate = NetAudioLogBaseData.Samplerate(1);
% BufferSize = NetAudioLogBaseData.BufferSize(1);
% RingBufferSize = NetAudioLogBaseData.RingBufferSize(1);
% TargetSampleLatency = NetAudioLogBaseData.TargetSampleLatency(1);
% 
% minTime = min(NetAudioLogStream.WorldTimeStamp(1),NetAudioLogNet.WorldTimeStamp(1));
% TimeNet = NetAudioLogNet.WorldTimeStamp - minTime;
% TimeStream = NetAudioLogStream.WorldTimeStamp - minTime;
% 
% TimeState = [TimeStream NetAudioLogStream.StreamingStatus];
% TimeUnderrun = TimeState(find(TimeState(:,2)==3), 1);
% TimeOverrun = TimeState(find(TimeState(:,2)==4), 1);
% TimeStream = TimeState(find(TimeState(:,2)==2), 1);
% 
% %% Plot Freie Smaples und Bufferstutus
% Data = [NetAudioLogNet.WorldTimeStamp NetAudioLogNet.FreeSamples; NetAudioLogStream.WorldTimeStamp NetAudioLogStream.FreeSamples];
% Data = sortrows(Data,1 );
% Data(:,1) = Data(:,1) - minTime();
% Data(:,2) = (Data(:,2)) / BufferSize;
% medianBlock = (max(Data(:,2)) - min(Data(:,2))) / 2;
% subplot(2,2,1:2)
% plot( Data(:,1), Data(:,2) )
% hold on
% plot( TimeUnderrun, zeros(size(TimeUnderrun)) + max(Data(:,2)),'r*')
% plot( TimeOverrun, zeros(size(TimeOverrun)) + min(Data(:,2)),'r+')
% title(['Freie Blöcke im Ring Buffer (' num2str(RingBufferSize) ' Samples)'])
% xlabel('Zeit in s')
% ylabel('Anzahl der Blöcke')
% 
% %% Plot Latenz
% DiffTime = diff(NetAudioLogClient.WorldTimeStamp * 1000);
% median = mean(DiffTime(:,1));
% medianVec = zeros(size(DiffTime(:,1))); 
% medianVec = medianVec + median;
% DiffTime = [NetAudioLogClient.WorldTimeStamp(2:end) - minTime DiffTime NetAudioLogClient.ProtocolStatus(2:end)];
% DiffTime = sortrows(DiffTime,3);
% LatenzWaiting = DiffTime(find(DiffTime(:,3)<222), (1:2));
% LatenzRunnning = DiffTime(find(DiffTime(:,3)>221), (1:2));
% medianRunning = mean(LatenzRunnning(:,2));
% medianRunningVec = zeros(size(LatenzRunnning(:,1))); 
% medianRunningVec = medianRunningVec + medianRunning;
% sollLatenz = (BufferSize / SampleRate) * 1000;
% sollLatenzVec = zeros(size(LatenzRunnning(:,1))); 
% sollLatenzVec = sollLatenzVec + sollLatenz;
% 
% subplot(2,2,1:4)
% plot( LatenzRunnning(:,1), LatenzRunnning(:,2), 'b.')
% hold on
% plot( LatenzRunnning(:,1), medianRunningVec, 'r')
% plot( LatenzRunnning(:,1), sollLatenzVec, 'g')
% plot( TimeUnderrun, zeros(size(TimeUnderrun)) + sollLatenz,'r*')
% plot( TimeOverrun, zeros(size(TimeOverrun)) + medianRunning,'r*')
% AnzahlUnderruns = size(TimeUnderrun);
% AnzahlUnderruns = AnzahlUnderruns(1);
% AnzahlUnderruns = num2str(AnzahlUnderruns);
% RelativeUnderruns = 100 * size(TimeUnderrun) / size(TimeState);
% Durchsatz = [num2str((32 * SampleRate * Channel)/1000) ' kbit/s']
% title(['Latenz pro Block (' num2str(BufferSize) ' Samples) bei ' num2str(Channel) ' Kanälen'])
% legend('Latenz', ['Latenz (' num2str(medianRunning) ' ms)'], ['SollLatenz (' num2str(sollLatenz) ' ms)' ], ['Underruns (Anz. ' AnzahlUnderruns ' - ' num2str(RelativeUnderruns) '%)'], 'Overruns')
% xlabel('Zeit in s')
% ylabel('Latenz in ms')
% legend('show') 
