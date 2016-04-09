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

format = @(data) reshape(data', size(data,1)*size(data,2), 1);

% Thu Jun 18 18:21:44 2015 -0400
% $$$ data_tx_24d1ce = load('Results\tx_sample_24d1ce2268d6_2.txt');
% $$$ data_tx_24d1ce = format(data_tx_24d1ce);
% $$$ data_rx_24d1ce = load('Results\rx_sample_24d1ce2268d6_2.txt');
% $$$ data_rx_24d1ce = format(data_rx_24d1ce);

%  Thu Jul 9 10:35:38 2015 +0100
data_tx_76f17d = load('Results\tx_sample_76f17d243ef56a_2.txt');
data_tx_76f17d = format(data_tx_76f17d);
data_rx_76f17d = load('Results\rx_sample_76f17d243ef56a_2.txt');
data_rx_76f17d = format(data_rx_76f17d);

%  Mon Aug 10 23:55:46 2015 +0100
data_tx_3e694f = load('Results\tx_sample_3e694f93c1794bb4023be8_2.txt');
data_tx_3e694f = format(data_tx_3e694f);
data_rx_3e694f = load('Results\rx_sample_3e694f93c1794bb4023be8_2.txt');
data_rx_3e694f = format(data_rx_3e694f);

% Fri Aug 21 16:44:31 2015 +0100
% $$$ data_tx_d2d65a = load('Results\tx_sample_d2d65a7b11723c87a31_2.txt');
% $$$ data_tx_d2d65a = format(data_tx_d2d65a);
% $$$ data_rx_d2d65a = load('Results\rx_sample_d2d65a7b11723c87a31_2.txt');
% $$$ data_rx_d2d65a = format(data_rx_d2d65a);

% Mon Aug 31 11:52:08 2015 +0100
data_tx_13249b = load('Results\tx_sample_13249b45138333_2.txt');
data_tx_13249b = format(data_tx_13249b);
data_rx_13249b = load('Results\rx_sample_13249b45138333_2.txt');
data_rx_13249b = format(data_rx_13249b);

% Thu Sep 3 00:34:36 2015 +0100
data_tx_d2d65a7 = load('Results\tx_sample_d2d65a7b11723c87a31_2.txt');
data_tx_d2d65a7 = format(data_tx_d2d65a7);
data_rx_d2d65a7 = load('Results\rx_sample_d2d65a7b11723c87a31_2.txt');
data_rx_d2d65a7 = format(data_rx_d2d65a7);




figure
%clf
subplot(2,4,1);
%plot(1:length(data_tx_24d1ce), data_tx_24d1ce);
plot(1:length(data_tx_76f17d), data_tx_76f17d);
subplot(2,4,2);
plot(1:length(data_tx_3e694f), data_tx_3e694f);
subplot(2,4,3);
%plot(1:length(data_tx_d2d65a), data_tx_d2d65a);
plot(1:length(data_tx_d2d65a7), data_tx_13249b);
subplot(2,4,4);
plot(1:length(data_tx_d2d65a7), data_tx_d2d65a7);

subplot(2,4,5);
%plot(1:length(data_rx_24d1ce), data_rx_24d1ce);
plot(1:length(data_rx_76f17d), data_rx_76f17d);
subplot(2,4,6);
plot(1:length(data_rx_3e694f), data_rx_3e694f);
subplot(2,4,7);
%plot(1:length(data_rx_d2d65a), data_rx_d2d65a);
plot(1:length(data_rx_d2d65a7), data_rx_13249b);
subplot(2,4,8);
plot(1:length(data_rx_d2d65a7), data_rx_d2d65a7);

