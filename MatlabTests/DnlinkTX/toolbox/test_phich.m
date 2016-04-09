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

clear
warning('off','lte:defaultValue')
for NCellID = 0:503
    for NDLRB = [6, 15, 25, 50, 75, 100]
        for nGroup = 0:ceil(1*NDLRB/8)-1 % assumes Ng = 1, CyclicPrefix = Normal
            for ack = 0:1
                nSeq = 0;
                enb.NCellID = NCellID;
                enb.NDLRB = NDLRB;
                enb.CellRefP = 1;
                enb.NSubframe = 0; % NSubframe has no effect
                enb.PHICHDuration = 'Normal';
                enb.Ng = 'One';
                expected = ltePHICH(enb, [nGroup nSeq ack]);
                enb.Ng = 1;
                actual = phich(enb, nGroup, nSeq, ack);
                if ~isequal(expected, actual)
                    [expected actual]
                    enb
                    die
                end
            end
        end
    end
end