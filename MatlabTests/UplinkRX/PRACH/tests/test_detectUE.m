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
    
data = load('test_PRACHcapture.infile');
waveform30 = data(1:2:end) + i*data(2:2:end);
%freqOffset = 2880;
%waveform30 = exp(1i*2*pi*freqOffset*(1:length(waveform30))/24576).* waveform30;
waveform = waveform30(1:16:end);


%DEBUG
%waveform = [zeros(1,2000)+100, waveform, zeros(1,20000)+100];



D = [];

fprintf('\nCorrect preamble:\n');
preambleNum = 19;
for j = 0:0
  [offset, bestVal] = detectUE(waveform(j*3*512+(1:1728)), preambleTable, preambleNum);
  D = [D, bestVal < 1e5];
end

preambleNum = 20;
for j = 0:0
  [offset, bestVal] = detectUE(waveform(j*3*512+(1:1728)), preambleTable, preambleNum);
  D = [D, bestVal < 1e5];
end


fprintf('\nWrong preamble:\n');
preambleNum = 21;
for j = 0:0
  [offset, bestVal] = detectUE(waveform(j*3*512+(1:1728)), preambleTable, preambleNum);
  D = [D, bestVal < 1e5];
end


DD = zeros(size(D));
for j=1:length(D)
  if D(j) == 1
    DD(j) = 0;
  else
    DD(j) = -1;
  end
end


f = fopen('test_detectUE.outfile.ground', 'w');
fprintf(f,'%d,', DD);
fclose(f);




% Not included in the test

filename = 'binintest_uplinkPHY_real_samples1.infile';
f = fopen(filename, 'rb');
din = fread(f, inf, 'int16');
din = din(1:2:end-1) + i*din(2:2:end);
fclose(f);
waveform_LTE = transpose(din)/1000;


fprintf('\nRegular LTE waveform:\n');
preambleNum = 20;
for j = 0:10
  [offset, bestVal] = detectUE(waveform_LTE(j*3*512+(1:1728)), preambleTable, preambleNum);
  D = [D, bestVal < 1e5];
end



