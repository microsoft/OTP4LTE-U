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

addpath ..

  
rng(0);
clear i;


nrb = 3; % TODO: waveform errors if nrb > 3
ue.NULRB = 6;
ue.NSubframe = 0;
ue.NCellID = 0;
ue.RNTI = 0;
chs.Modulation = 'QPSK';
chs.PRBSet = (0:(nrb-1))';
chs.RV = 0;
rv = chs.RV;


trblklen = 16;
A = trblklen;
D = A + 24 + 4; % TB + CRC + trellis termination
dl = rand(D, 3) < 0.5; % use a valid TB length
E = 864;


% $$$ trblklen = 208;
% $$$ A = trblklen;
% $$$ D = A + 24 + 4; % TB + CRC + trellis termination
% $$$ dl = rand(D, 3) < 0.5; % use a valid TB length
% $$$ E = 2304;



d = [];
e = [];
dout = [];

% Transmit %
el = rateMatchTurbo(dl, E, rv);
% Receive %
doutl = derateMatchTurbo(el, D, rv);
assert(isequal(dl, doutl));
dp = reshape(dl, size(dl,1)*size(dl,2), 1)';
d = [d, dp];
e = [e; el];
dout = [dout; doutl];



trblklen = 208;
A = trblklen;
D = A + 24 + 4; % TB + CRC + trellis termination
dl = rand(D, 3) < 0.5; % use a valid TB length
E = 2304;

% Transmit %
el = rateMatchTurbo(dl, E, rv);
% Receive %
doutl = derateMatchTurbo(el, D, rv);
assert(isequal(dl, doutl));
dp = reshape(dl, size(dl,1)*size(dl,2), 1)';
d = [d, dp];
e = [e; el];
dout = [dout; doutl];




trblklen = 1200;
A = trblklen;
D = A + 24 + 4; % TB + CRC + trellis termination
dl = rand(D, 3) < 0.5; % use a valid TB length
E = 4000;

% Transmit %
el = rateMatchTurbo(dl, E, rv);
% Receive %
doutl = derateMatchTurbo(el, D, rv);
assert(isequal(dl, doutl));
dp = reshape(dl, size(dl,1)*size(dl,2), 1)';
d = [d, dp];
e = [e; el];
dout = [dout; doutl];



f = fopen('test_derateMatchTurbo.outfile.ground', 'w');
fprintf(f,'%d,', d);
fclose(f);

f = fopen('test_derateMatchTurbo.infile', 'w');
fprintf(f,'%d,', e);
fclose(f);



