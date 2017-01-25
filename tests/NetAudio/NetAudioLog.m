NetAudioLogNet = readtable( 'NetAudioLogNet.txt' );
NetAudioLogStream = readtable( 'NetAudioLogStream.txt' );

plot( NetAudioLogNet.WorldTimeStamp, NetAudioLogNet.FreeSamples/10240 )
hold on
plot( NetAudioLogStream.WorldTimeStamp, NetAudioLogStream.FreeSamples/10240 )
hold on
%plot( NetAudioLogNet.WorldTimeStamp, NetAudioLogNet.Bufferstatus )
plot( NetAudioLogStream.WorldTimeStamp, NetAudioLogStream.StreamingStatus )