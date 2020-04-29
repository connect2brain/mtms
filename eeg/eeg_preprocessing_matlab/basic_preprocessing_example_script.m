
% This is an example script for cleaning standard TMS-EEG data recorded
% with the Brainvision system. 

% Data: 
% The script assumes the data in the EEGLAB data format. An example script
% is provided in the C2B project folder. 

% Dependencies: 
% 1. Fastica package https://research.ics.aalto.fi/ica/fastica/
% 2. EEGLAB toolbox, tested with 2019_0 https://sccn.ucsd.edu/eeglab/index.php
% 3. TESA EEGLAB plugin https://nigelrogasch.github.io/TESA/


% Tuomas Mutanen 29.4.2020

%% 1. Initate the script 

clear;
clc;
close all;

% Show MATLAB the locations of the required toolboxes:

% Fastica:
fastica_path = uigetdir([],'Path to Fastica package');
addpath(fastica_path);

% EEGLAB: (See from the TESA documentation how to add TESA plugin to the EEGLAB package)
eeglab_path = uigetdir([],'Path to EEGLAB toolbox');

addpath(eeglab_path);
eeglab;

%% 2. Load the data: 
% Assumes the EEGLAB data format (.set)

% Give the path to the folder containing the example dataset
data_path = uigetdir([],'Path to EEG data folder');

% Load the data to matlab
EEG = pop_loadset('filename','TMS_EEG_testset.set','filepath',data_path);


%% 3. Find the theoretical channel locations:

% Compare the channel labels of the dataset to the general EEG channel
% layout template and look for the theoretical coordinates on a sphere.
%
% Useful for at least visualization and for some spatial filtering methods
% that are not used here. 

EEG=pop_chanedit(EEG, 'lookup','/Users/tpmutane/Documents/5HzPackage/eeglab2019_0/plugins/dipfit/standard_BESA/standard-10-5-cap385.elp',...
    'settype',{'' 'EEG'},'settype',{'1:62' 'EEG'},'changefield',{63 'type' 'EOG'},'changefield',{64 'type' 'EOG'});

%% 4. Epoch the data around the pulse:

time_interval_of_interest = [-1.5 1.5]; % ( in s  w.r.t. TMS trigger):

EEG = pop_epoch( EEG, {  'S128'  }, time_interval_of_interest  , 'newname', 'orig epochs', 'epochinfo', 'yes');

%% 5. Baseline-correct the data:

baseline_time_interval = [-100 -5]; %(in ms w.r.t. TMS)
EEG = pop_rmbase( EEG, baseline_time_interval  ,[]);

% Check visually the average raw data:
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');

%% 6. Let's remove the TMS pulse artifact and interpolate the missing time points with cubic spline:

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

%% 7. a Check the data for bad channels and trials:

% First visualize average response to possibly identify bad channels in 7c:

figure; pop_plottopo(EEG, [1:62] , 'orig epochs', 0, 'ydir',1);

%% 7 b

% Visualize the trials. Click any contaminated trials to turn them
% yellow. In the end click "reject"

pop_eegplot( EEG, 1, 1, 1);



%% 7 c

% Remove possible bad channel

list_of_bad_channels = {'POz'};

EEG = pop_select( EEG, 'nochannel',list_of_bad_channels);


%% 8. a There are alot of blinks or other weird low frequency stuff so we try to remove them with ICA

% Resample the data to make the ICA computation more feasible.
% The function has an integrated anti-aliasing filter.

new_sampling_rate = 1000;

EEG = pop_resample( EEG, new_sampling_rate);


%% 8 b Run ICA

% Divide the data into independent components:
 EEG = pop_tesa_fastica( EEG, 'approach', 'symm', 'g', 'tanh', 'stabilization', 'off' );
 
%% 8 c Visual inspection of independent components

% Set parameters for the automatic component classification. Will enhance the
% visual inspection. 

EEG = pop_tesa_compselect( EEG,'compCheck','on','comps',[],'figSize','small',...
    'plotTimeX',[-1499 1499],'plotFreqX',[1 100],'tmsMuscle','off','tmsMuscleThresh',...
    8,'tmsMuscleWin',[11 30],'tmsMuscleFeedback','off','blink','on','blinkThresh',2.5,...
    'blinkElecs',{'Fp1','Fp2'},'blinkFeedback','off','move',...
    'off','moveThresh',2,'moveElecs',{'F7','F8'},'moveFeedback','off','muscle',...
    'off','muscleThresh',0.6,'muscleFreqWin',[30 100],'muscleFeedback', 'off' ,...
    'elecNoise','off','elecNoiseThresh',4,'elecNoiseFeedback','off' );

%% 8 d  Baseline correction after ICA:

baseline_time_interval = [-100 -5]; %(in ms w.r.t. TMS)
EEG = pop_rmbase( EEG, baseline_time_interval  ,[]);

% Visually inspect the data:
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');
figure; pop_timtopo(EEG, [-1000  1000], [20  40  60  80], 'ERP data and scalp maps of orig epochs');

%% 9. Detrend the epochs:

% Fit and remove the slow drifts using a  linear model:

EEG = pop_tesa_detrend( EEG, 'linear', [-1500 1499] );

% Visualize the outcome
figure; pop_timtopo(EEG, [-1000  1000], [20  40  60  80], 'ERP data and scalp maps of orig epochs');

%% 10 a. Select the channels and trials again:

%Channels

figure; pop_plottopo(EEG, [1:62] , 'orig epochs', 0, 'ydir',1);

% Uncomment to remove channels:
%list_of_bad_channels = {'POz'}; EEG = pop_select( EEG, 'nochannel',list_of_bad_channels );


%% 10 b.

% Inspect and remove bad trials as earlier

% The next two lines are necessary to make the trial rejection work after
% ICA. No idea why. Further inspection needed. Tuomas
eeglab redraw;
EEG = eeg_checkset( EEG );

pop_eegplot( EEG, 1, 1, 1);

%EEG = pop_rejepoch( EEG, 4,0);

%% 11. Select only EEG channels:

% Unselect the EOG channels
EEG = pop_select( EEG, 'nochannel',{'EOG1' 'EOG2'}); 

%% 12. Average reference:

% Move to average reference:
EEG = pop_reref( EEG, []);
            
% Visually inspect the data:
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');
figure; pop_plottopo(EEG, [1:61] , 'orig epochs', 0, 'ydir',1);

%% 13. Simple bandpass filtering of the data:

figure;
EEG = pop_eegfiltnew(EEG, 'locutoff',1,'hicutoff',80,'plotfreqz',1);

% Visually inspect the data:
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');


%%  14.  Finally Snip of the terminals of the time interval that are not interesting.

EEG = pop_select( EEG, 'time', [-1 1] );

%% 15. Visualize the final result:

% Visually inspect the final data:
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');


figure; pop_plottopo(EEG, [1:61] , 'orig epochs', 0, 'ydir',1); clear EEG_plot;

figure; pop_newtimef( EEG, 1, 59, [-1500  1499], [3         0.5] , 'topovec', 59, 'elocs', ...
    EEG.chanlocs, 'chaninfo', EEG.chaninfo, 'caption', 'Oz', 'baseline',[0], 'freqs', [[4 50]], 'plotphase', 'off', 'padratio', 1);
