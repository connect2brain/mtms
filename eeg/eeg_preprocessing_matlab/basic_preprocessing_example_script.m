
% This is an example script for cleaning standard TMS-EEG data recorded
% with the Brainvision system

% Dependencies: 
% 1. Fastica package https://research.ics.aalto.fi/ica/fastica/
% 2. EEGLAB toolbox, tested with 2019_0 https://sccn.ucsd.edu/eeglab/index.php
% 3. TESA EEGLAB plugin https://nigelrogasch.github.io/TESA/

% Tuomas Mutanen 29.4.20

%% 1. Initate the script 

clear;
clc;
close all;

% Show MATLAB the locations of the required toolboxes:

fastica_path = uigetdir([],'Path to Fastica package');%'/Users/tpmutane/Documents/5HzPackage/FastICA_25';
addpath(fastica_path);

eeglab_path = uigetdir([],'Path to EEGLAB toolbox');%'/Users/tpmutane/Documents/5HzPackage/eeglab2019_0/';

addpath(eeglab_path);
eeglab;

%% 2. Load the data:

% Give path to the header of your dataset:

EEG = pop_loadbv();

[ALLEEG EEG CURRENTSET] = pop_newset(ALLEEG, EEG, 0,'setname','orig','gui','off'); 

%% 3. Find the theoretical channel locations:

EEG=pop_chanedit(EEG, 'lookup','/Users/tpmutane/Documents/5HzPackage/eeglab2019_0/plugins/dipfit/standard_BESA/standard-10-5-cap385.elp',...
    'settype',{'' 'EEG'},'settype',{'1:62' 'EEG'},'changefield',{63 'type' 'EOG'},'changefield',{64 'type' 'EOG'});

%% 4. Epoch the data around the pulse:

EEG = pop_epoch( EEG, {  'S128'  }, [-1.5 1.5], 'newname', 'orig epochs', 'epochinfo', 'yes');

%% 5. Baseline-correct the data:

EEG = pop_rmbase( EEG, [-100 -5] ,[]);

% Check visually the average raw data:
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');

%% 6. Let's remove the TMS pulse artifact and interpolate the missing time points with cubic spline:

% First visualize the TMS-pulse artifact to confirm the time-interval to be
% removed.
figure; pop_timtopo(EEG, [-5  10], [NaN], 'ERP data and scalp maps of orig epochs');

% Remove the time-interval containing the pulse artifact. 
EEG = pop_tesa_removedata( EEG, [-2 7] );

% Interpolat the missing 
EEG = pop_tesa_interpdata( EEG, 'cubic', [5 5] );

figure; pop_timtopo(EEG, [-5 10], [NaN], 'ERP data and scalp maps of orig epochs');
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');

%% 7. a Check the data for bad channels and trials:

figure; pop_plottopo(EEG, [1:62] , 'orig epochs', 0, 'ydir',1);

%% 7 b
pop_eegplot( EEG, 1, 1, 1);

%% 7 c
EEG = pop_select( EEG, 'nochannel',{'POz'});


%% 8. a There are alot of blinks or other weird low frequency stuff so we try to remove them with ICA

% Resample the data to make ICA more feasible:
% The function has anti-aliasing filter integrated
EEG = pop_resample( EEG, 1000);

%% 8 b Run ICA,  (I had to correct the internal EEGLAB functions)
 EEG = pop_tesa_fastica_tm( EEG, 'approach', 'symm', 'g', 'tanh', 'stabilization', 'off' );
            %parameters for automatic component selection
EEG = pop_tesa_compselect( EEG,'compCheck','on','comps',[],'figSize','small',...
    'plotTimeX',[-1499 1499],'plotFreqX',[1 100],'tmsMuscle','off','tmsMuscleThresh',...
    8,'tmsMuscleWin',[11 30],'tmsMuscleFeedback','off','blink','on','blinkThresh',2.5,...
    'blinkElecs',{'Fp1','Fp2'},'blinkFeedback','off','move',...
    'off','moveThresh',2,'moveElecs',{'F7','F8'},'moveFeedback','off','muscle',...
    'off','muscleThresh',0.6,'muscleFreqWin',[30 100],'muscleFeedback', 'off' ,...
    'elecNoise','off','elecNoiseThresh',4,'elecNoiseFeedback','off' );

% Baseline correction after ICA:
EEG = pop_rmbase( EEG, [-100 -1] ,[]);
            
% Visually inspect the data:
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');
figure; pop_timtopo(EEG, [-1000  1000], [20  40  60  80], 'ERP data and scalp maps of orig epochs');

%% 9. Detrend the epochs:

EEG = pop_tesa_detrend( EEG, 'linear', [-1500 1499] );
figure; pop_timtopo(EEG, [-1000  1000], [20  40  60  80], 'ERP data and scalp maps of orig epochs');

%% 10 a. Select the channels and trials again:

%Channels
figure; pop_plottopo(EEG, [1:62] , 'orig epochs', 0, 'ydir',1);
EEG = pop_select( EEG, 'nochannel',{'POz'});


%% 10 b.

% Trials
pop_eegplot( EEG, 1, 1, 1);


%% 12. Select only EEG channels:

EEG = pop_select( EEG, 'nochannel',{'EOG1' 'EOG2'});
[ALLEEG EEG CURRENTSET] = pop_newset(ALLEEG, EEG, 12,'gui','off'); 

%% 13. Average reference:

EEG = pop_reref( EEG, []);
            
% Visually inspect the data:
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');
figure; pop_plottopo(EEG, [1:61] , 'orig epochs', 0, 'ydir',1);

%% 14. Simple bandpass filtering of the data:

EEG = pop_eegfiltnew(EEG, 'locutoff',1,'hicutoff',60,'plotfreqz',1);

% Visually inspect the data:
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');


figure; pop_newtimef( EEG, 1, 59, [-1500  1499], [3         0.5] , 'topovec', 59, 'elocs', EEG.chanlocs, 'chaninfo', EEG.chaninfo, 'caption', 'Oz', 'baseline',[0], 'freqs', [[4 200]], 'plotphase', 'off', 'padratio', 1);

%% 15. Visualize the final result:

EEG = pop_select( EEG, 'time',[-0.5 0.5] );

% Visually inspect the data:
figure; pop_timtopo(EEG, [-100  200], [20  40  60  80], 'ERP data and scalp maps of orig epochs');


EEG_plot = pop_select( EEG, 'time',[-0.1 0.4] );
figure; pop_plottopo(EEG_plot, [1:61] , 'orig epochs', 0, 'ydir',1); clear EEG_plot;