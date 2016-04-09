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

% Random symbols
rng(0);
X = randi(4, 600, 1) -1;
Y = exp(1j * (pi/4 + X * pi/2));
grid = repmat(Y, 1, 14);

% Insert DRS
nPRB = 5;
ue = struct('NSubframe', 0, 'NULRB', 50, 'NCellID', 11);
chs = struct('PRBSet', 1+(0:nPRB-1)');
drms = drs(ue, chs);
drsIndices = drsi(ue, chs);
grid(drsIndices) = drms;

% Scale
grid = round(grid * 3000 * 2.5);

% Save wanted data
data = grid(puschi(ue, chs));
f = fopen('test_c_equalize.outfile.ground', 'w');
fprintf(f,'%d,', floor([real(data) imag(data)]'));
fclose(f);

g = grid * 0.333; % TODO: should modulate, then apply channel

f = fopen('test_c_equalize.infile', 'w');
fprintf(f,'%d,', floor([real(g(:)) imag(g(:))]'));
fclose(f);
