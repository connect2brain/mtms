data_sample = zeros(62, 1);
window_size = uint32(20);
time_us = uint64(50);
first_sample_of_experiment = false;

cfg = coder.config("lib");
cfg.TargetLang = "C++";
cfg.InlineBetweenUserFunctions = "Readability";
codegen -config cfg run_processor -args {window_size,data_sample,time_us,first_sample_of_experiment} -report