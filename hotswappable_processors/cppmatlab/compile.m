data_sample = rand(62,1);
window_size = uint32(20);
time_us = uint64(50);
first_sample_of_experiment = false;
channel_count = uint16(62);

cfg = coder.config("lib");
cfg.TargetLang = "C++";
cfg.InlineBetweenUserFunctions = "Readability";
cfg.RuntimeChecks = true;
tic
codegen -config cfg run_processor -args {window_size,channel_count,data_sample,time_us,first_sample_of_experiment} -report
toc
