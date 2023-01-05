data_sample = rand(62,1); % HACK: helps compiler to understand that the data of data_sample varies, not just zeros for example
window_size = uint32(20);
time = double(50);
first_sample_of_experiment = false;
channel_count = uint16(62);

setenv('TMPDIR', './tempdir');

cfg = coder.config("lib");
cfg.TargetLang = "C++";
cfg.InlineBetweenUserFunctions = "Readability";
cfg.RuntimeChecks = false;

tic
codegen -config cfg run_processor -args {window_size,channel_count,data_sample,time,first_sample_of_experiment} -report
toc
