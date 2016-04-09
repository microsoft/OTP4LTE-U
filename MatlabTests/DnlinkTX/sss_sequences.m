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

function [s, c0, c1, z1] = sss_sequences(Nid2)
    x=zeros(1,31); x(5)=1;
    for k=1:26; x(k+5)=mod(x(k+2)+x(k),2); end
    s_=1-2*x;
    s=@(m) circshift(s_,[0,-m]);

    x=zeros(1,31); x(5)=1;
    for k=1:26; x(k+5)=mod(x(k+3)+x(k),2); end
    c_=1-2*x;
    c0=circshift(c_,[0,-Nid2]);
    c1=circshift(c0,[0,-3]);

    x=zeros(1,31); x(5)=1;
    for k=1:26; x(k+5)=mod(x(k+4)+x(k+2)+x(k+1)+x(k),2); end
    z_=1-2*x;
    z1=@(m) circshift(z_,[0,-mod(m,8)]);
end
