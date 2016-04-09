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

function z = pucch1(ns, ue, chs, ack, drs)
    cinit = ue.NCellID;
    c = gold(cinit, 8*7*20 + 8*14 + 8);
    ncs_cell = @(ns, l) 2.^(0:7) * c(8*7*ns + 8*l + (1:8));

    c = 3; % normal cyclic prefix
    n_0 = mod(chs.ResourceIdx, c * 12 / chs.DeltaShift); % assume Ncs = 0
    n_1 = mod(c * (n_0 + 1), c * 12 / chs.DeltaShift + 1) - 1; % assume Ncs = 0
    n_m = [n_0 n_1];
    n_ = @(ns) n_m(mod(ns, 2) + 1);
    noc = @(ns) floor(n_(ns) * chs.DeltaShift / 12);
    ncs = @(ns, l) mod(ncs_cell(ns, l) + (n_(ns) * chs.DeltaShift + mod(noc(ns), chs.DeltaShift)), 12);
    alpha = @(ns, l) 2 * pi * ncs(ns, l) / 12;
    phi = [3 1 -1 -1 3 3 -3 1 3 1 3 3]; assert(ue.NCellID == 11); % TODO: whole table
    r_ = exp(1j * phi * pi / 4);
    r = @(alpha) exp(1j * alpha * (0:11)) .* r_;
    
    if drs
        symbols = [2 3 4];
        S = [1 1];
        W = [1 1 1; 1 exp(1j*2*pi/3) exp(1j*4*pi/3); 1 exp(1j*4*pi/3) exp(1j*2*pi/3)];
        d = 1; % ack argument is ignored for DRS
    else
        symbols = [0 1 5 6];
        S = [1 exp(1j * pi / 2)];
        W = [1 1 1 1; 1 -1 1 -1; 1 -1 -1 1];
        
        % 36.211 Table 5.4.1-1
        % NB: no difference between format 1, 1a-NACK, 1b-NACK-NACK
        %     no difference between format 1a-ACK, 1b-ACK-ACK
        if isequal(ack, [])
            d = 1;
        elseif isequal(ack, 0)
            d = 1;
        elseif isequal(ack, 1)
            d = -1;
        elseif isequal(ack, [0 0])
            d = 1;
        elseif isequal(ack, [0 1])
            d = -1j;
        elseif isequal(ack, [1 0])
            d = 1j;
        elseif isequal(ack, [1 1])
            d = -1;
        end
    end
    
    z = [];
    s = S(mod(n_(ns), 2)+1); 
    w = W(noc(ns)+1, :);
    for m = 1:length(symbols)           
        a = alpha(ns, symbols(m));
        y = d * r(a).';            
        z = [z; s * w(m) * y];
    end
end

   