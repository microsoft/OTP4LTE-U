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

data = load('../../../UplinkRX/samples/pucch_capture2.txt');
data = data(1,1:2:end) + i*data(1,2:2:end);
data = reshape(data, 600, 14);


ue.NULRB = 50;
ue.NSubframe = 8;
ue.NCellID = 11;
chs.ResourceIdx = 0; % sr-PUCCH-ResourceIndex (RRC_CS) for SR, f(n1Pucch-AN in SIB2, DCI) for ACK/NACK
chs.DeltaShift = 3; % deltaPUCCH-Shift in SIB2
chs.ResourceSize = 0; % nRB-CQI in SIB2


o = [];

for r = 0:7
  chs.ResourceIdx = r; % sr-PUCCH-ResourceIndex (RRC_CS) for SR, f(n1Pucch-AN in SIB2, DCI) for ACK/NACK
  pucch;
  corr = round(corr' / 95 * 14);
  o = [o; corr];
  %fprintf('Correlation with ResourceIdx %d/1: %d %d\n', chs.ResourceIdx, real(corr), imag(corr));
end


f = fopen('test_c_pucch.outfile.ground', 'w');
fprintf(f,'%d,', floor([real(o) imag(o)]'));
fclose(f);
