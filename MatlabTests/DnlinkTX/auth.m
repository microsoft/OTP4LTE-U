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

function auth
    K = hex2bin('000102030405060708090A0B0C0D0E0F');
    %K = hex2bin('00112233445566778899AABBCCDDEEFF');
    RAND = hex2bin('55555555555555555555555555555556');
    SQN = hex2bin('000000000001'); % TODO: why not 0?
    AMF = hex2bin('8000');
    
    XDOUT = xor(K, RAND);
    AK = XDOUT(25:72);
    CDOUT = [SQN; AMF];
    MAC = xor(XDOUT(1:64), CDOUT);
    
    AUTN = [xor(SQN, AK); AMF; MAC];
    RES = xor(K, RAND);

    % Compare against CMW captures.
    assert(isequal(AUTN, hex2bin('56515053525C8000555457565151D352')));
    assert(isequal(RES, hex2bin('55545756515053525D5C5F5E59585B59')));
end