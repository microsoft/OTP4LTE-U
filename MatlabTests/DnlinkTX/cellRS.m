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

function crs = cellRS(cellID, subframe, ndlrb, cellrefp)
    NDLRB_MAX = 110;
    assert(ismember(cellrefp, [1 2]));
    % Assume only 1 RX antenna.
    crs = zeros(ndlrb * 12, 14, cellrefp); % 12 subcarriers, 14 symbols per RB
    vshift = mod(cellID, 6);

    for port = 0:cellrefp-1
        for slot = [0 1]
            ns = subframe * 2 + slot;
            for l = [0 4]
                v = 3 * xor(port, l);
                ci = 2^10*(7*(ns+1)+l+1)*(2*cellID+1)+2*cellID+1; % normal CP
                c = gold(ci, 4*NDLRB_MAX-1);
                for m = 0:(2*ndlrb-1)
                    m_ = m + NDLRB_MAX - ndlrb;
                    r = rss(c, m_);
                    k = 6 * m + mod(v + vshift, 6);
                    assert(crs(k+1, 7*slot+l+1, port+1) == 0); % DEBUG
                    crs(k+1, 7*slot+l+1, port+1) = r;
                end
            end
        end
    end
end
