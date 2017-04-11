%% Load
ITANetAudioTest_Client = readtable( 'ITANetAudioTest_Client_Client.log', 'FileType', 'text', 'Delimiter', '\t');
ITANetAudioTest_Server = readtable( 'ITANetAudioTest_Server_Server.log', 'FileType', 'text', 'Delimiter', '\t');
ITANetAudioTest_Client_AudioStream = readtable( 'ITANetAudioTest_Client_AudioStream.log', 'FileType', 'text', 'Delimiter', '\t');
ITANetAudioTest_Client_NetworkStream = readtable( 'ITANetAudioTest_Client_NetworkStream.log', 'FileType', 'text', 'Delimiter', '\t');


%% Settings
B = 256;
TL = B * 3;
RB = B * 10;


%% Analyse
all_times = [ ITANetAudioTest_Client.WorldTimeStamp; ITANetAudioTest_Server.WorldTimeStamp; ITANetAudioTest_Client_AudioStream.WorldTimeStamp; ITANetAudioTest_Client_NetworkStream.WorldTimeStamp ];
TS = min( all_times );
TE = max( all_times );

AudioStreamingUnderrunIndices = find( ITANetAudioTest_Client_AudioStream.FreeSamples == RB );
AudioStreamingOverrunIndices = find( ITANetAudioTest_Client_AudioStream.FreeSamples == 0 );

ClientTransmittedBufferInfoIndices = find( strcmpi( ITANetAudioTest_Client.TransmittedRingBufferFreeSamples,'true' ));

ServerNetCommTiming = diff( ITANetAudioTest_Server.WorldTimeStamp );
ClientNetCommTiming = diff( ITANetAudioTest_Client.WorldTimeStamp );
ClientAudioStreamTiming = diff( ITANetAudioTest_Client_AudioStream.WorldTimeStamp );
ClientNetStreamTiming = diff( ITANetAudioTest_Client_NetworkStream.WorldTimeStamp );

%% Plot
figure
subplot( 2, 1, 1 )

% Client
plot( ITANetAudioTest_Client.WorldTimeStamp - TS, RB - ITANetAudioTest_Client.FreeSamples,  'LineWidth', 1 )
hold on
plot( ITANetAudioTest_Client.WorldTimeStamp( ClientTransmittedBufferInfoIndices ) - TS, RB - ITANetAudioTest_Client.FreeSamples( ClientTransmittedBufferInfoIndices ), '^' )
hold on
plot( ITANetAudioTest_Client_NetworkStream.WorldTimeStamp - TS, RB - ITANetAudioTest_Client_NetworkStream.FreeSamples )

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

legend( { 'RealBufferStatus', 'BufferInfoTransmit', 'NetworkStreamBufferStatus', 'EstimatedBufferStatus', 'TargetLatency', 'RingBufferCapacity', 'Underruns', 'Overruns' } )
ylabel( 'NumSamples' )
xlabel( 'Time' )
ylim( [ -B/10 RB + B/10 ] );
hold off

% Timing
subplot( 2, 1, 2 )
plot( ITANetAudioTest_Client.WorldTimeStamp( 2:end ) - TS, ClientNetCommTiming )
hold on
plot( ITANetAudioTest_Client_AudioStream.WorldTimeStamp( 2:end ) - TS, ClientAudioStreamTiming )
hold on
plot( ITANetAudioTest_Client_NetworkStream.WorldTimeStamp( 2:end ) - TS, ClientNetStreamTiming )
hold on
plot( ITANetAudioTest_Server.WorldTimeStamp( 2:end ) - TS, ServerNetCommTiming )
hold on

title( 'Clock Timing' )
legend( { 'ClientNet', 'ClientAudioStream', 'ClientNetStream', 'ServerNet' } )
ylabel( 'Process timing' )
xlabel( 'Streaming time' )

hold off
