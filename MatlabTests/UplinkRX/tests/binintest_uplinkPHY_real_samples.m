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

clear all;
addpath ../../DnlinkTX/;
addpath ..;


filename = 'binintest_uplinkPHY_real_samples1.infile';


f = fopen(filename, 'rb');
din = fread(f, inf, 'int16');
din = din(1:2:end-1) + i*din(2:2:end);
fclose(f);

waveform = din;


uplinkPHY_real_samples;


%%% Input files for various stages

%copyfile(filename, 'binintest_uplinkPHY_real_samples.infile');
%copyfile(filename, 'binintest_uplinkPHY_real_samples_demodulate.infile');
%copyfile(filename, 'binintest_uplinkPHY_real_samples_equalize.infile');
%copyfile(filename, 'binintest_uplinkPHY_real_samples_deprecode.infile');
%copyfile(filename, 'binintest_uplinkPHY_real_samples_psch_decode.infile');



%%% Output files for various stages

f = fopen('binintest_uplinkPHY_real_samples.outfile.ground', 'w');
fprintf(f, '%d,', rx.trblk);
%fprintf(f, '%d,', rx.e);
fclose(f);

f = fopen('binintest_uplinkPHY_real_samples_demodulate.outfile.ground', 'w');
g=transpose(reshape(rx.grid, size(rx.grid,1)*size(rx.grid,2),1));
fprintf(f, '%d,', round([real(g); imag(g)] / 2408));
fclose(f);

f = fopen('binintest_uplinkPHY_real_samples_equalize.outfile.ground', 'w');
fprintf(f, '%d,', round([real(rx.precoded)'; imag(rx.precoded)'] * 3e3 * 2.5));
fclose(f);

f = fopen('binintest_uplinkPHY_real_samples_deprecode.outfile.ground', 'w');
fprintf(f, '%d,', round([real(rx.symbols)'; imag(rx.symbols)'] * 570));
fclose(f);

% Without interleaving
f = fopen('binintest_uplinkPHY_real_samples_psch_decode.outfile.ground', 'w');
fprintf(f, '%d,', rx.e);
fclose(f);
