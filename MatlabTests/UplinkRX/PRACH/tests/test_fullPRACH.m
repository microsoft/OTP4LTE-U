%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% MIT License
%
% Copyright (c) 2016 Microsoft
%
% Permission is hereby granted, free of charge, to any person obtaining a copy
% of this software and associated documentation files (the "Software"), to deal
% in the Software without restriction, including without limitation the rights
% to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
% copies of the Software, and to permit persons to whom the Software is
% furnished to do so, subject to the following conditions:
%
% The above copyright notice and this permission notice shall be included in
% all copies or substantial portions of the Software.
%
% THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
% IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
% FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
% AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
% LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
% OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
% THE SOFTWARE.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

addpath ..;
 
prachSettings = struct(...
    'nUlRb', 50, 'frameNumber', 0, 'subFrameNumber', 1,...
    'prachConfigurationIndex', 0, 'prachFrequencyOffset', 22,...
    'rachRootSequence', 0, 'zeroCorrelationZoneConfig', 12, ...
    'highSpeedFlag', 0, 'betaPrach', 1, 'nPreambles', 64);

dumpPreambleTable(prachSettings);
preambleTable = loadPreambleTable();
    
% $$$ f = fopen('test_PRACHcapture.infile');
% $$$ data = fread(f, [2 30720], 'int16'); % 1 ms = 1 subframe
% $$$ fclose(f);
% $$$ waveform30 = complex(data(1,:), data(2,:));


%data = load('test_PRACHcapture.infile');
data = load('test_fullPRACH.infile');
waveform30 = data(1:2:end) + i*data(2:2:end);
%freqOffset = 2880;
%waveform30 = exp(1i*2*pi*freqOffset*(1:length(waveform30))/24576).* waveform30;
waveform = waveform30(1:8:end);



[index, offset, bestVal] = detectPRACH(waveform, preambleTable);


index
offset
bestVal


