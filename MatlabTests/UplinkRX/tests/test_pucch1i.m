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

addpath ../ 
ue.NULRB = 50;
ue.NSubframe = 8;
ue.NCellID = 11;
chs.ResourceIdx = 4; % sr-PUCCH-ResourceIndex (RRC_CS) for SR, f(n1Pucch-AN in SIB2, DCI) for ACK/NACK
chs.DeltaShift = 3; % deltaPUCCH-Shift in SIB2
chs.ResourceSize = 0; % nRB-CQI in SIB2

startRB = [];
ind = pucch1i(ue, chs, true);
s = mod(ind(1), 600);
% DEBUG: Seems that the lower bit is shifter by one RE. Find out why!
s = max(s-2, 0);
startRB = [startRB, s];
s = mod(ind(1+36), 600);
% DEBUG: Seems that the lower bit is shifter by one RE. Find out why!
s = max(s-2, 0);
startRB = [startRB, s];



chs.ResourceIdx = 13; % sr-PUCCH-ResourceIndex (RRC_CS) for SR, f(n1Pucch-AN in SIB2, DCI) for ACK/NACK

ind = pucch1i(ue, chs, true);
s = mod(ind(1), 600);
% DEBUG: Seems that the lower bit is shifter by one RE. Find out why!
s = max(s-2, 0);
startRB = [startRB, s];
s = mod(ind(1+36), 600);
% DEBUG: Seems that the lower bit is shifter by one RE. Find out why!
s = max(s-2, 0);
startRB = [startRB, s];


f = fopen('test_pucch1i.outfile.ground', 'w');
fprintf(f,'%d,', startRB);
fclose(f);


