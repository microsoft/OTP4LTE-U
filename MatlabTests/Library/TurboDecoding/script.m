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

rng(0);
tbs = 5504;

nBlockLength = tbs;
nCodeRate = 1/3;
aEb_No = 0:(0.5):10;


aBitArray = vertcat(randi(2,1,nBlockLength)-1);
D=encodeTurbo1(aBitArray);
len = size(D,2);
Do = (2*D-1)*15;
D = horzcat(D(1,:), D(2,:), D(3,:));


% DEBUG
D = load('../../../Library/Ziria/tests/test_turbo.infile');
len = 40+4;
al_len = ceil(len / 16)*16;

% Scaling to match C code
D = (2*D-1)*7;


Lc = zeros(1, al_len);
Lp=Lc;
La=Lc;

Lc(1:len) = D(1:len);             % systematic
Lp(1:len) = D(len+(1:len));       % parity
La(1:len) = zeros(1,len);         % a priori


BlockLen = 48;
gamma = zeros(1,BlockLen*4);

w = 16;
for j=0:BlockLen/w-1
  gamma(j*4*w+0*w+(1:w)) = -Lc(j*w+(1:w)) - La(j*w+(1:w)) - Lp(j*w+(1:w));
  gamma(j*4*w+1*w+(1:w)) = -Lc(j*w+(1:w)) - La(j*w+(1:w)) + Lp(j*w+(1:w));
  gamma(j*4*w+2*w+(1:w)) = Lc(j*w+(1:w)) + La(j*w+(1:w)) - Lp(j*w+(1:w));
  gamma(j*4*w+3*w+(1:w)) = Lc(j*w+(1:w)) + La(j*w+(1:w)) + Lp(j*w+(1:w));
  
  x0=gamma(j*4*w+0*w+(1:w));
  x1=gamma(j*4*w+1*w+(1:w));
  x2=gamma(j*4*w+2*w+(1:w));
  x3=gamma(j*4*w+3*w+(1:w));
  
  interlv8 = @(a1, a2) (reshape([a1; a2], 1, 16));
  interlv16 = @(a1, a2) (reshape([reshape(a1, 2, 4); reshape(a2, 2, 4)], 1, 16));

  %% re-arrangement: xmm3, xmm5, xmm2, xmm0
  %% xmm0=p0,o0,n0,m0,l0,k0,j0,i0,h0,g0,f0,e0,d0,c0,b0,a0
  %% xmm1=p1,o1,n1,m1,l1,k1,j1,i1,h1,g1,f1,e1,d1,c1,b1,a1
  %% xmm2=p2,o2,n2,m2,l2,k2,j2,i2,h2,g2,f2,e2,d2,c2,b2,a2
  %% xmm3=p3,o3,n3,m3,l3,k3,j3,i3,h3,g3,f3,e3,d3,c3,b3,a3
  %%
  %% 
  %% xmm0=p0,p1,p2,p3,o0,o1,o2,o3,n0,n1,n2,n3,m0,m1,m2,m3
  %% xmm1=l0,l1,l2,l3,k0,k1,k2,k3,j0,j1,j2,j3,i0,i1,i2,i3
  %% xmm3=h0,h1,h2,h3,g0,g1,g2,g3,f0,f1,f2,f3,e0,e1,e2,e3
  %% xmm4=d0,d1,d2,d3,c0,c1,c2,c3,b0,b1,b2,b3,a0,a1,a2,a3

  
  x4 = x0;
  % interleave_low_16 x0=(x0,x1)
  x0 = interlv8(x0(1:8), x1(1:8));
  % interleave_high x4=(x4,x1)
  x4 = interlv8(x4(9:16), x1(9:16));

  x5 = x2;
  % interleave_low x2=(x2,x3)
  x2 = interlv8(x2(1:8), x3(1:8));
  % interleave_high x5=(x5,x3)
  x5 = interlv8(x5(9:16), x3(9:16));

  x1 = x0;
  % interleave_low x0=(x0,x2)
  x0 = interlv16(x0(1:8), x2(1:8));
  % interleave_high x1=(x1,x2)
  x1 = interlv16(x1(9:16), x2(9:16));

  x3 = x4;
  % interleave_low x3=(x3,x5)
  x3 = interlv16(x3(1:8), x5(1:8));
  % interleave_high x4=(x4,x5)
  x4 = interlv16(x4(9:16), x5(9:16));
  
  gamma(j*w*4+(1:w*4)) = [x0, x1, x3, x4];
end




% A16 char CharOne[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
% A16 char AlphaBeta[(MAX_BLOCK_LEN + 6) * 16];//alpha:1-5508;beta:3-5510
% A16 char gamma[(MAX_BLOCK_LEN + 16) * 4];
% A16 char NePo[(MAX_BLOCK_LEN + 4) * 16];//ne:1:5508*8,po:5508*8+1:5508*8+5508*8
% A16 char tntp[(MAX_BLOCK_LEN + 3 + 13) * 2];//tn:1:5507,tp:5521:11027

% A16 char ext21[MAX_BLOCK_LEN + 3 + 13];//extrinsic LLR array from dec2 to dec1
% A16 char ext12[MAX_BLOCK_LEN + 3 + 13];//extrinsic LLR array from dec1 to dec2
% A16 char ext1[MAX_BLOCK_LEN + 3 + 13]; //extrinsic llR array to dec1 after deinterleaver
% A16 char ext2[MAX_BLOCK_LEN + 3 + 13]; //extrinsic llr array to dec2 after interleaver; 
