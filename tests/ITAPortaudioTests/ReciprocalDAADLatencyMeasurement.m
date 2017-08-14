%% Generate excitation signal
sweep_raw = ita_generate_sweep;
ita_write_wav( sweep_raw, 'sweep.wav', 'nBits', 32, 'overwrite' );

%% Evaluate recording
recordA = ita_read( 'recordA.wav' );
recordB = ita_read( 'recordB.wav' );

recordA.pt
recordB.pt
