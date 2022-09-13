event_info.event_id = uint16(0);
event_info.execution_condition = uint8(0);
event_info.time_us = uint64(0);

piece1.mode = uint8(0);
piece1.duration_in_ticks = uint16(200);
piece2.mode = uint8(2);
piece2.duration_in_ticks = uint16(160);
piece3.mode = uint8(1);
piece3.duration_in_ticks = uint16(160);

pieces = [piece1, piece2, piece3];

pulse_command.channel = uint8(5);
pulse_command.event_info = event_info;
pulse_command.pieces = pieces;

f = @() jsonencode(pulse_command);

times = zeros(100, 1);
for i=1:100
    t = timeit(f);
    times(i) = t;
end
