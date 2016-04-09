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

% Shared between uplink and downlink.
function a = sch_decode(e, trblklen, RV, disableCRC)
    A = trblklen;
    D = A + 24 + 4; % TB + CRC + trellis termination
    d = derateMatchTurbo(e, D, RV);
    softbits = d' * 2 - 1; % TODO: proper soft demodulation
    [~, x, ~, ~] = decodeTurbo(softbits, A + 24, 3, 1);
    a = x(1:A)'; c = x(A+(1:24))';
    if (isequal(crc24a(a), c)) 
      fprintf('CRC passes: ');
      fprintf('%d', crc24a(a));
      fprintf(' - ');
      fprintf('%d', c);
      fprintf('\n');
    else
      fprintf('CRC fails: ');
      fprintf('%d', crc24a(a));
      fprintf(' - ');
      fprintf('%d', c);
      fprintf('\n');
    end
    if nargin <= 3 || ~disableCRC
      assert(isequal(crc24a(a), c));
    end
end