%% Load
ITANetAudioTest_Client = readtable( 'ITANetAudioTest_Client.log', 'FileType', 'text', 'Delimiter', '\t');
ITANetAudioTest_Server = readtable( 'ITANetAudioTest_Server.log', 'FileType', 'text', 'Delimiter', '\t');
ITANetAudioTest_Client_AudioStream = readtable( 'ITANetAudioTest_Client_AudioStream.log', 'FileType', 'text', 'Delimiter', '\t');

%% Settings
B = 512;
TL = 512*4;
RB = 512*8;


%% Analyse
TS = min( [ ITANetAudioTest_Client.WorldTimeStamp; ITANetAudioTest_Server.WorldTimeStamp; ITANetAudioTest_Client_AudioStream.WorldTimeStamp ] );
TE = max( [ ITANetAudioTest_Client.WorldTimeStamp; ITANetAudioTest_Server.WorldTimeStamp; ITANetAudioTest_Client_AudioStream.WorldTimeStamp ] );

AudioStreamingUnderrunIndices = find( ITANetAudioTest_Client_AudioStream.FreeSamples == RB );
AudioStreamingOverrunIndices = find( ITANetAudioTest_Client_AudioStream.FreeSamples == 0 );

ClientTransmittedBufferInfoIndices = find( strcmpi( ITANetAudioTest_Client.TransmittedRingBufferFreeSamples,'true' ));

%% Plot
% Client
plot( ITANetAudioTest_Client.WorldTimeStamp - TS, RB - ITANetAudioTest_Client.FreeSamples,  'LineWidth', 1 )
hold on
plot( ITANetAudioTest_Client.WorldTimeStamp( ClientTransmittedBufferInfoIndices ) - TS, RB - ITANetAudioTest_Client.FreeSamples( ClientTransmittedBufferInfoIndices ), '>' )
hold on

% Estimated by server
plot( ITANetAudioTest_Server.WorldTimeStamp - TS, RB - ITANetAudioTest_Server.EstimatedFreeSamples, 'LineWidth', 1 )
hold on

% Target latency
plot( [ 0 TE - TS ], repmat( TL, 1, 2 ), 'LineWidth', 4  )
hold on

% Ringbuffer capacity
plot( [ 0 TE - TS ], repmat( RB, 1, 2 ), 'LineWidth', 4  )
hold on

% Underruns (might be empty)
plot( ITANetAudioTest_Client_AudioStream.WorldTimeStamp( AudioStreamingUnderrunIndices ) - TS, RB - ITANetAudioTest_Client_AudioStream.FreeSamples( AudioStreamingUnderrunIndices ) , 'ro' )
hold on

% Overruns (might be empty)
plot( ITANetAudioTest_Client_AudioStream.WorldTimeStamp( AudioStreamingOverrunIndices ) - TS, RB - ITANetAudioTest_Client_AudioStream.FreeSamples( AudioStreamingOverrunIndices ) , 'm*' )
hold on

legend( { 'RingBufferSamples', 'BufferInfoTransmit', 'Estimated', 'TargetLatency', 'RingBufferCapacity', 'Underruns', 'Overruns' } )
ylabel( 'NumSamples' )
xlabel( 'Time' )
ylim( [ -B/10 RB + B/10 ] );
