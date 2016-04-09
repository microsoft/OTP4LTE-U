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

% $$$ f = fopen('dump1.bin', 'rb');
% $$$ data = fread(f, Inf, 'int16');
% $$$ fclose(f);
% $$$ data = data(1:2:end) + i*data(2:2:end);
st = 2.54e6;
en = 2.56e6;


% $$$ f = fopen('dump_ipa_w_prach_2.bin', 'rb');
% $$$ data = fread(f, Inf, 'int16');
% $$$ fclose(f);
% $$$ data = data(1:2:end) + i*data(2:2:end);
% $$$ data = data(1:20000);
% $$$ f = fopen('dump_ipa_w_prach_2.txt', 'w');
% $$$ d = [real(data), imag(data)]';
% $$$ fprintf(f, '%d,', d(:));
% $$$ fclose(f);



% $$$ f = fopen('dump1.bin', 'rb');
% $$$ data = fread(f, Inf, 'int16');
% $$$ fclose(f);
% $$$ data = data(1:2:end) + i*data(2:2:end);
st = 125649;
en = 140870;

% $$$ load('prach_1.mat');
% $$$ data = rxdata; 
st = 124922;
en = 140116;

% $$$ data = load('../../UplinkRX/PRACH/tests/test_detectUE.infile');
% $$$ data = data(1:2:end) + i*data(2:2:end);


load('prach_ok.mat');
% First PRACH
st = 18308+78;
en = 32209;
% Second PRACH
st = 325504+90;
en = 339417;

d = data(st:en);
b = [real(d), imag(d)]';
b = round(b(:) * 2^15);
f = fopen('test_detectUE2.infile', 'w');
fprintf(f, '%d,', b);
fclose(f);
aaa

figure; plot(abs(fft(data(st+1000+(1:12288)))));


