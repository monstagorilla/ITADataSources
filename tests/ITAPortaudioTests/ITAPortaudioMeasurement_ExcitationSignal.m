sweep_raw = ita_generate_sweep;
sweep_2ch = ita_merge( sweep_raw * 0.1, sweep_raw * 0 );
ita_write_wav( sweep_2ch, 'ITAPortaudioMeasurement_ExcitationSignal.wav', 'nBits', 32, 'overwrite' );
