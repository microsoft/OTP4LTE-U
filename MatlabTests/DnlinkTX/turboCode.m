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

function dk = turboCode(ck)
    K = length(ck);
    table = [40 3 10;48 7 12;56 19 42;64 7 16;72 7 18;80 11 20;88 5 22;96 11 24;104 7 26;112 41 84;120 103 90;128 15 32;136 9 34;144 17 108;152 9 38;160 21 120;168 101 84;176 21 44;184 57 46;192 23 48;200 13 50;208 27 52;216 11 36;224 27 56;232 85 58;240 29 60;248 33 62;256 15 32;264 17 198;272 33 68;280 103 210;288 19 36;296 19 74;304 37 76;312 19 78;320 21 120;328 21 82;336 115 84;344 193 86;352 21 44;360 133 90;368 81 46;376 45 94;384 23 48;392 243 98;400 151 40;408 155 102;416 25 52;424 51 106;432 47 72;440 91 110;448 29 168;456 29 114;464 247 58;472 29 118;480 89 180;488 91 122;496 157 62;504 55 84;512 31 64;528 17 66;544 35 68;560 227 420;576 65 96;592 19 74;608 37 76;624 41 234;640 39 80;656 185 82;672 43 252;688 21 86;704 155 44;720 79 120;736 139 92;752 23 94;768 217 48;784 25 98;800 17 80;816 127 102;832 25 52;848 239 106;864 17 48;880 137 110;896 215 112;912 29 114;928 15 58;944 147 118;960 29 60;976 59 122;992 65 124;1008 55 84;1024 31 64;1056 17 66;1088 171 204;1120 67 140;1152 35 72;1184 19 74;1216 39 76;1248 19 78;1280 199 240;1312 21 82;1344 211 252;1376 21 86;1408 43 88;1440 149 60;1472 45 92;1504 49 846;1536 71 48;1568 13 28;1600 17 80;1632 25 102;1664 183 104;1696 55 954;1728 127 96;1760 27 110;1792 29 112;1824 29 114;1856 57 116;1888 45 354;1920 31 120;1952 59 610;1984 185 124;2016 113 420;2048 31 64;2112 17 66;2176 171 136;2240 209 420;2304 253 216;2368 367 444;2432 265 456;2496 181 468;2560 39 80;2624 27 164;2688 127 504;2752 143 172;2816 43 88;2880 29 300;2944 45 92;3008 157 188;3072 47 96;3136 13 28;3200 111 240;3264 443 204;3328 51 104;3392 51 212;3456 451 192;3520 257 220;3584 57 336;3648 313 228;3712 271 232;3776 179 236;3840 331 120;3904 363 244;3968 375 248;4032 127 168;4096 31 64;4160 33 130;4224 43 264;4288 33 134;4352 477 408;4416 35 138;4480 233 280;4544 357 142;4608 337 480;4672 37 146;4736 71 444;4800 71 120;4864 37 152;4928 39 462;4992 127 234;5056 39 158;5120 39 80;5184 31 96;5248 113 902;5312 41 166;5376 251 336;5440 43 170;5504 21 86;5568 43 174;5632 45 176;5696 45 178;5760 161 120;5824 89 182;5888 323 184;5952 47 186;6016 23 94;6080 47 190;6144 263 480]; % 36.212 Table 5.1.3-3
    f = table(table(:,1)==K, 2:3);
    assert(~isempty(f));
    dk = zeros(K+4, 3);
    reg1 = zeros(3, 1);
    reg2 = zeros(3, 1);
        
    for k = 1:K
        cki = ck(mod(f(1) * (k-1) + f(2) * (k-1)^2, K) + 1);
        in1 = mod(ck(k) + reg1(2) + reg1(3), 2);
        in2 = mod(cki + reg2(2) + reg2(3), 2);
        dk(k,1) = ck(k);
        dk(k,2) = mod(in1 + reg1(1) + reg1(3), 2);
        dk(k,3) = mod(in2 + reg2(1) + reg2(3), 2);
        reg1 = [in1; reg1(1:end-1)];
        reg2 = [in2; reg2(1:end-1)];
    end
    % Termination
    xk=[]; xk_=[]; zk=[]; zk_=[];
    for k = 1:3
        in1 = mod(reg1(2) + reg1(3), 2);
        in2 = mod(reg2(2) + reg2(3), 2);
        xk(k)  = in1;
        xk_(k) = in2;
        zk(k)  = mod(reg1(1) + reg1(3), 2);
        zk_(k) = mod(reg2(1) + reg2(3), 2);
        reg1 = [0; reg1(1:end-1)];
        reg2 = [0; reg2(1:end-1)];
    end
    dk(K+1:end,:) = [xk(1) zk(2) xk_(1) zk_(2);...
                     zk(1) xk(3) zk_(1) xk_(3);...
                     xk(2) zk(3) xk_(2) zk_(3)]';
end
