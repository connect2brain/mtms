
% This is an example script for cleaning standard TMS-EEG data recorded
% with the Brainvision system. 

% Depedencies:
% 
% 1. Matlab, tested with R2018b
% 
% 1. Fastica package, version  https://research.ics.aalto.fi/ica/fastica/
% 
% 2. EEGLAB toolbox, tested with version 2019_0 https://sccn.ucsd.edu/eeglab/index.php
% 
% 3. TESA EEGLAB plugin, version 1.0.1 https://nigelrogasch.github.io/TESA/
% 
% Installation:
% 
% 1. Download and extract the Fastica package. Define the path to the Fastica package in the beginning of the Matlab script.
% 
% 2. Download and extract EEGLAB package. Define the path to the EEGLAB package in the beginning of the Matlab script.
% 
% 3. Download and extract the TESA package to ~/path_to_eeglab/eeglab2019_0/plugins

% Tuomas Mutanen 7.05.2020

%% 1. Initate the script 

clear;
clc;
close all;

disp('Shell 1 started: Initiating the script...')

% Show MATLAB the locations of the required toolboxes:

% Path to Fastica package 
fastica_path = '/my/path/to/fastica/FastICA_25';
addpath(fastica_path);

% Path to EEGLAB toolbox:
eeglab_path = '/my/path/to/eeglab/eeglab2019_0/';

addpath(eeglab_path);
eeglab;

disp('Shell 1 ended')
disp('-------------------------------------------------------------------')
%% 2. Load the data: 

% Assumes the EEGLAB data format (.set)

disp('Cell 2 started: Loading the data...')

% Give the path to the folder containing the example dataset. Data placed
% on c2b-shared project folder: 
% //tw-nbe.org.aalto.fi/project/c2b-shared/SW_development/TMS_EEG_testset.set

data_path = 'my/path/to/EEG/data/folder/';

% Load the data to matlab
EEG = pop_loadset('filename','TMS_EEG_testset.set','filepath',data_path);

disp('Cell 2 ended')
disp('-------------------------------------------------------------------')

%% 3. Find the theoretical channel locations:

disp('Cell 3 started:  Finding the theoretical channel locations...')


% Compare the channel labels of the dataset to the general EEG channel
% layout template and look for the theoretical coordinates on a sphere.
%
% Useful for at least visualization and for some spatial filtering methods
% that are not used here. 

EEG=pop_chanedit(EEG, 'lookup',fullfile(eeglab_path,'plugins','dipfit','standard_BESA','standard-10-5-cap385.elp'),...
    'settype',{'' 'EEG'},'settype',{'1:62' 'EEG'},'changefield',{63 'type' 'EOG'},'changefield',{64 'type' 'EOG'});

disp('Cell 3 ended')
disp('-------------------------------------------------------------------')

%% 4. Epoch the data around the pulse:

disp('Cell 4 started:  Epoching the data around the pulse...')

time_interval_of_interest = [-1.5 1.5]; % ( in s  w.r.t. TMS trigger):

EEG = pop_epoch( EEG, {  'S128'  }, time_interval_of_interest  , 'newname', 'orig epochs', 'epochinfo', 'yes');

disp('Cell 4 ended')
disp('-------------------------------------------------------------------')

%% 5. Baseline-correct the data:

disp('Cell 5 started: Performing the baseline correction to the data...')

baseline_time_interval = [-100 -5]; %(in ms w.r.t. TMS)
EEG = pop_rmbase( EEG, baseline_time_interval  ,[]);

% Check visually the average raw data:
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');

disp('Cell 5 ended')
disp('-------------------------------------------------------------------')

%% 6. Let's remove the TMS pulse artifact and interpolate the missing time points with cubic spline:

disp('Cell 6 started: Removing the TMS pulse artifact...')

% First visualize the TMS-pulse artifact to confirm the time-interval to be
% removed.

time_interval_of_interest = [-5 10]; % ( in ms  w.r.t. TMS trigger)
figure; pop_timtopo(EEG, time_interval_of_interest, [NaN], 'ERP data and scalp maps of orig epochs');

% Remove the time-interval containing the pulse artifact. 

artifact_time_interval = [-2 7]; % ( in ms  w.r.t. TMS trigger)

EEG = pop_tesa_removedata( EEG, artifact_time_interval);

% Interpolate the snipped time window:

EEG = pop_tesa_interpdata( EEG, 'cubic', [5 5] ); % Uses 5 ms of data before and after the snipped interval in the interpolation

% Visualize the outcome:
figure; pop_timtopo(EEG, [-5 10], [NaN], 'ERP data and scalp maps of orig epochs');
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');

disp('Cell 6 ended')
disp('-------------------------------------------------------------------')

%% 7. a Check the data for bad channels and trials:

disp('Cell 7 a started: Check the data for bad channels:')

% First visualize average response to possibly identify bad channels in 7c:

figure; pop_plottopo(EEG, [1:62] , 'orig epochs', 0, 'ydir',1);

disp('Cell 7 a ended')
disp('-------------------------------------------------------------------')


%% 7 b

disp('Cell 7 b started: Check the data for bad trials. Click any contaminated trials to turn them yellow. In the end click "reject"')

% Visualize the trials. Click any contaminated trials to turn them
% yellow. In the end click "reject"

pop_eegplot( EEG, 1, 1, 1);

disp('Finalize Cell 7 b by clicking "reject" when you have identified the bad trials.')
disp('-------------------------------------------------------------------')


%% 7 c

disp('Cell 7 c started: Removing possible bad channel..')


% Remove possible bad channel

list_of_bad_channels = {'POz'};

EEG = pop_select( EEG, 'nochannel',list_of_bad_channels);

disp('Cell 7 c ended')
disp('-------------------------------------------------------------------')


%% 8. a There are alot of blinks or other weird low frequency stuff so we try to remove them with ICA

disp('Cell 8 a: Downsampling prior to ICA...');

% Resample the data to make the ICA computation more feasible.
% The function has an integrated anti-aliasing filter.

new_sampling_rate = 1000;

EEG = pop_resample( EEG, new_sampling_rate);

disp('Cell 8 a ended')
disp('-------------------------------------------------------------------')


%% 8 b Run ICA

disp('Cell 8 b: Running the ICA...');

% Divide the data into independent components:
 EEG = pop_tesa_fastica( EEG, 'approach', 'symm', 'g', 'tanh', 'stabilization', 'off' );
 
 
disp('Cell 8 b ended')
disp('-------------------------------------------------------------------')

 
%% 8 c Visual inspection of independent components

disp('Cell 8 c: Select bad ICA components:');

% Set parameters for the automatic component classification. Will enhance the
% visual inspection. 

EEG = pop_tesa_compselect( EEG,'compCheck','on','comps',[],'figSize','small',...
    'plotTimeX',[-1499 1499],'plotFreqX',[1 100],'tmsMuscle','off','tmsMuscleThresh',...
    8,'tmsMuscleWin',[11 30],'tmsMuscleFeedback','off','blink','on','blinkThresh',2.5,...
    'blinkElecs',{'Fp1','Fp2'},'blinkFeedback','off','move',...
    'off','moveThresh',2,'moveElecs',{'F7','F8'},'moveFeedback','off','muscle',...
    'off','muscleThresh',0.6,'muscleFreqWin',[30 100],'muscleFeedback', 'off' ,...
    'elecNoise','off','elecNoiseThresh',4,'elecNoiseFeedback','off' );

disp('Finalize Cell 8 c by accepting the ICA cleaning or redo')
disp('-------------------------------------------------------------------')

%% 8 d  Baseline correction after ICA:

disp('Cell 8 d: Baseline correction after ICA.');

baseline_time_interval = [-100 -5]; %(in ms w.r.t. TMS)
EEG = pop_rmbase( EEG, baseline_time_interval  ,[]);

% Visually inspect the data:d
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');
figure; pop_timtopo(EEG, [-1000  1000], [20  40  60  80], 'ERP data and scalp maps of orig epochs');

disp('Cell 8 d ended')
disp('-------------------------------------------------------------------')

 

%% 9. Detrend the epochs:

disp('Cell 9: Detrending the epochs...');

% Fit and remove the slow drifts using a  linear model:

EEG = pop_tesa_detrend( EEG, 'linear', [-1500 1499] );

% Visualize the outcome
figure; pop_timtopo(EEG, [-1000  1000], [20  40  60  80], 'ERP data and scalp maps of orig epochs');

disp('Cell 9 ended')
disp('-------------------------------------------------------------------')


%% 10 a. Select the channels and trials again:

disp('Cell 10 a started: Check the data again for bad channels:')


%Channels

figure; pop_plottopo(EEG, [1:62] , 'orig epochs', 0, 'ydir',1);

% Uncomment to remove channels:
%list_of_bad_channels = {'POz'}; EEG = pop_select( EEG, 'nochannel',list_of_bad_channels );

disp('Cell 10 a ended')
disp('-------------------------------------------------------------------')



%% 10 b.

disp('Cell 10 b started: Check the data for bad trials. Click any contaminated trials to turn them yellow. In the end click "reject"')

% Inspect and remove bad trials as earlier

% The next two lines are necessary to make the trial rejection work after
% ICA. No idea why. Further inspection needed. Tuomas
eeglab redraw;
EEG = eeg_checkset( EEG );

pop_eegplot( EEG, 1, 1, 1);

disp('Finalize Cell 10 b by clicking "reject" when you have identified the bad trials.')
disp('-------------------------------------------------------------------')


%EEG = pop_rejepoch( EEG, 4,0);

%% 11. Select only EEG channels:


disp('Cell 11 started: Removing EOG channels...')


% Unselect the EOG channels
EEG = pop_select( EEG, 'nochannel',{'EOG1' 'EOG2'}); 

disp('Cell 11 ended')
disp('-------------------------------------------------------------------')



%% 12. Average reference:

disp('Cell 12: Go to average reference:')


% Move to average reference:
EEG = pop_reref( EEG, []);
            
% Visually inspect the data:
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');
figure; pop_plottopo(EEG, [1:61] , 'orig epochs', 0, 'ydir',1);

disp('Cell 12 ended')
disp('-------------------------------------------------------------------')



%% 13. Simple bandpass filtering of the data:


disp('Cell 13: Bandpass filter at 1-80 Hz...')


figure;
EEG = pop_eegfiltnew(EEG, 'locutoff',1,'hicutoff',80,'plotfreqz',1);

% Visually inspect the data:
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');

disp('Cell 13 ended')
disp('-------------------------------------------------------------------')


%%  14.  Finally Snip of the terminals of the time interval that are not interesting.

disp('Cell 14: Snipping of the fist and last 500 ms of the epochs to remove possible edge effects')

EEG = pop_select( EEG, 'time', [-1 1] );

disp('Cell 14 ended')
disp('-------------------------------------------------------------------')


%% 15. Visualize the final result:

disp('Cell 15: Visualizng the final results...')

% Visually inspect the final data:
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');


figure; pop_plottopo(EEG, [1:61] , 'orig epochs', 0, 'ydir',1); clear EEG_plot;

figure; pop_newtimef( EEG, 1, 59, [-1500  1499], [3         0.5] , 'topovec', 59, 'elocs', ...
    EEG.chanlocs, 'chaninfo', EEG.chaninfo, 'caption', 'Oz', 'baseline',[0], 'freqs', [[4 50]], 'plotphase', 'off', 'padratio', 1);

disp('Cell 15 ended')
disp('-------------------------------------------------------------------')

