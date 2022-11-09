function waveform = get_default_waveform(channel)
    if channel == 1 || channel == 2
        falling_phase_duration_in_ticks = 1480;
    elseif channel == 3 || channel == 4
        falling_phase_duration_in_ticks = 1564;
    elseif channel == 5
        falling_phase_duration_in_ticks = 1776;
    end

    piece1.waveform_phase = uint8(0);
    piece1.duration_in_ticks = uint16(2400);
    piece2.waveform_phase = uint8(1);
    piece2.duration_in_ticks = uint16(1200);
    piece3.waveform_phase = uint8(2);
    piece3.duration_in_ticks = uint16(falling_phase_duration_in_ticks);
    
    coder.cstructname(piece1, 'waveform_piece');

    waveform = [piece1, piece2, piece3];

end