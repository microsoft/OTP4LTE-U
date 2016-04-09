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

framelen = 307200;
scale = 1;             % 1 for 2048 FFT, 2 for 1024 FFT

% $$$ f = fopen('test_gps_1.bin', 'rb');
% $$$ d = fread(f, Inf, 'int16');
% $$$ d = d(1:2:end) + i*d(2:2:end);
% $$$ fclose(f);
% $$$ start = 58059;
% $$$ start = 374040;
% $$$ start = 380620-4;
% $$$ data = d(start + (1:2*framelen));


% $$$ f = fopen('test_same_1.bin', 'rb');
% $$$ d = fread(f, Inf, 'int16');
% $$$ d = d(1:2:end) + i*d(2:2:end);
% $$$ fclose(f);
% $$$ start = 207616;
% $$$ data = d(start + (1:2*framelen));


f = fopen('test_fixed_1.bin', 'rb');
d = fread(f, Inf, 'int16');
d = d(1:2:end) + i*d(2:2:end);
fclose(f);
start = 128985;
data = d(start + (1:4*framelen));


template = load('..\..\DnlinkTX\tests\test_genTemplateSubframe.outfile.ground');
template = transpose(template(1:2:end) + i*template(2:2:end));

figure(1);
clf(1);
plot(real(data))
hold on;

yl = ylim();
x = 0;
e = length(data);

cnt = 0;
while x < e
  if (mod(cnt, 140) == 0) 
    plot([x;x], yl, '-k', 'LineWidth', 3);
  elseif (mod(cnt, 14) == 0) 
    plot([x;x], yl, '-k', 'LineWidth', 1);
  else
    plot([x;x], yl, ':r');
  end
  
  if (mod(cnt, 14) == 0 || mod(cnt, 14) == 7)
    x = x + (2048+160)/scale;
  else
    x = x + (2048+144)/scale;
  end
  if (x>e) 
    break;
  end
  cnt = cnt + 1;
end



% $$$ figure(2);
% $$$ clf(2);
% $$$ plot_sym_grid(data, 0:12, 2048);


% $$$ figure(3);
% $$$ clf(3);
% $$$ hold on
% $$$ %M = plot_sym_freq(data, 15*14 + 8, 2048);
% $$$ s = 4;
% $$$ %s = 5;
% $$$ M = plot_sym_freq(data, s, 2048);
% $$$ plot(1:600, abs(template((1:600)+s*600)), 'r')


figure(4);
clf(4);
subplot(2,1,1);
% Get RS
ps = 4;
M = plot_sym_freq(data, ps, 2048);
%M = plot_sym_freq(transpose(test), ps, 2048);
chans = exp(i*angle(M(3:6:end) ./ template((3:6:600)+ps*600)));
chan = interp(chans, 6);
chan = [chan(1); chan(1); chan(1:end-2)];
M = plot_sym_freq(data, 9, 2048);
int = 265:336;
X = 1:length(int);
Mf = M(int) ./ chan(int);
plot(X, angle(M(int)), X, angle(Mf));
plot(real(Mf), imag(Mf), '.');
title('MIB, symbol 9');

subplot(2,1,2);
M = plot_sym_freq(data, 10, 2048);
int = 265:336;
X = 1:length(int);
Mf = M(int) ./ chan(int);
plot(X, angle(M(int)), X, angle(Mf));
plot(real(Mf), imag(Mf), '.');
title('MIB, symbol 10');



% $$$ figure(3);
% $$$ clf(3);
% $$$ hold on
% $$$ %M = plot_sym_freq(data, 15*14 + 8, 2048);
% $$$ s = 14*5+7;
% $$$ st = 14*3+7;   % See test_genTemplateSubframe
% $$$ %s = 5;
% $$$ M = plot_sym_freq(data, s, 2048);
% $$$ plot(1:600, abs(template((1:600)+st*600)), 'r')



achan = zeros(600,1);

figure(5);
clf(5);
subplot(2,2,1);
% Get RS
p = 0;
sym = 1;
ps = sym*140+5*14+p;
pst = 2*14+p;     % See test_genTemplateSubframe
M = plot_sym_freq(data, ps, 2048);
%M = plot_sym_freq(transpose(test), ps, 2048);
chans = exp(i*angle(M(6:6:end) ./ template((6:6:600)+pst*600)));
chan = interp(chans, 6);
sh = 5;
chan = [repmat(chan(1), sh, 1); chan(1:end-sh)];
plot(6:6:600, angle(chans), 1:600, angle(chan));
achan = achan + chan;
chan0 = chan; 

subplot(2,2,2);
% Get RS
p = 4;
ps = sym*140+5*14+p;
pst = 2*14+p;     % See test_genTemplateSubframe
M = plot_sym_freq(data, ps, 2048);
%M = plot_sym_freq(transpose(test), ps, 2048);
sh = 2;
chans = exp(i*angle(M((sh+1):6:end) ./ template(((sh+1):6:600)+pst*600)));
chan = interp(chans, 6);
chan = [repmat(chan(1), sh, 1); chan(1:end-sh)];
plot(6:6:600, angle(chans), 1:600, angle(chan));
achan = achan + chan;
chan4 = chan; 

subplot(2,2,3);
% Get RS
p = 7;
ps = sym*140+5*14+p;
pst = 2*14+p;     % See test_genTemplateSubframe
M = plot_sym_freq(data, ps, 2048);
%M = plot_sym_freq(transpose(test), ps, 2048);
chans = exp(i*angle(M(6:6:end) ./ template((6:6:600)+pst*600)));
chan = interp(chans, 6);
sh = 5;
chan = [repmat(chan(1), sh, 1); chan(1:end-sh)];
plot(6:6:600, angle(chans), 1:600, angle(chan));
achan = achan + chan;
chan7 = chan; 

subplot(2,2,4);
% Get RS
p = 11;
ps = sym*140+5*14+p;
pst = 2*14+p;     % See test_genTemplateSubframe
M = plot_sym_freq(data, ps, 2048);
%M = plot_sym_freq(transpose(test), ps, 2048);
sh = 2;
chans = exp(i*angle(M((sh+1):6:end) ./ template(((sh+1):6:600)+pst*600)));
chan = interp(chans, 6);
chan = [repmat(chan(1), sh, 1); chan(1:end-sh)];
plot(6:6:600, angle(chans), 1:600, angle(chan));
achan = achan + chan;
chan11 = chan; 

achan = achan / 4;


figure(6);
clf(6);
subplot(2,2,1);
p = 5;
M = plot_sym_freq(data, sym*140+5*14+p, 2048);
int = 1:144;
X = 1:length(int);
Mf = M(int) ./ chan0(int);
plot(X, angle(M(int)), X, angle(Mf));
plot(real(Mf), imag(Mf), '.');
title(sprintf('SIB1, symbol %d', p));

subplot(2,2,2);
p = 6;
M = plot_sym_freq(data, sym*140+5*14+p, 2048);
int = 1:144;
X = 1:length(int);
Mf = M(int) ./ chan0(int);
plot(X, angle(M(int)), X, angle(Mf));
plot(real(Mf), imag(Mf), '.');
title(sprintf('SIB1, symbol %d', p));

subplot(2,2,3);
p = 7;
M = plot_sym_freq(data, sym*140+5*14+p, 2048);
int = 1:144;
X = 1:length(int);
Mf = M(int) ./ chan0(int);
plot(X, angle(M(int)), X, angle(Mf));
plot(real(Mf), imag(Mf), '.');
title(sprintf('SIB1, symbol %d', p));

subplot(2,2,4);
p = 8;
M = plot_sym_freq(data, sym*140+5*14+p, 2048);
int = 1:144;
X = 1:length(int);
Mf = M(int) ./ chan0(int);
plot(X, angle(M(int)), X, angle(Mf));
plot(real(Mf), imag(Mf), '.');
title(sprintf('SIB1, symbol %d', p));


