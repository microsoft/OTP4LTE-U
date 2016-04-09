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

% 
% function enocdeConvolutional(aBitArray)
% 
% Convolutional encoding with tail-biting (36.212, Section 5.1.3.1)
% 
% Input: 
% - aBitArray: horizontal array of bits to be encoded
% 
% Output:
% - 3 horizontal arrays of encoded bits, to be combined using interleaveConvolutionalRateMatching
%
function aDecodedMessage=decodeConvolutionalCircularViterbi(D,nM)

S = zeros(1,nM);
n=size(D,2);
%PreviousState = zeros(,n+1,2);
aPreviousStateCount = zeros(2^length(S),n+2);

for t=1:(n)
  for k=1:2^length(S)
      S(0 +1) = rem(k-1,2);
      S(1 +1) = rem(floor((k-1)/2),2);
      S(2 +1) = rem( floor((k-1)/4),2 );
      S(3 +1) = rem( floor((k-1)/8),2);
      S(4 +1) = rem( floor((k-1)/16),2);
      S(5 +1) = rem( floor((k-1)/32),2);
      
      Output(1,k,t,0 +1) = mod(0 + S(4 +1) + S(3 +1) + S(1 + 1) + S(0 +1), 2);
      Output(2,k,t,0 +1) = mod(0 + S(5 +1) + S(4 +1) + S(3 + 1) + S(0 +1), 2);
      Output(3,k,t,0 +1) = mod(0 + S(5 +1) + S(4 +1) + S(2 + 1) + S(0 +1), 2);

      S((0:4) +1) = S((1:5) +1);
      S(5 +1) = 0;
      nextState = S(0 +1)*(2^0) + S(1 +1)*(2^1) + S(2 +1)*(2^2) + S(3 +1)*(2^3) + S(4 +1)*(2^4) + S(5 +1)*(2^5);
      NextState(k,t,0 +1) = nextState;
      
      nPreviousStates = aPreviousStateCount(nextState +1,t+1);
      PreviousState(nextState +1, t+1,nPreviousStates +1) = k -1;
      aPreviousStateCount(nextState +1,t+1) = nPreviousStates + 1;      
      assert(nPreviousStates+1 <= 2);
      
%       if ( PreviousState(2,1+1,0 +1) == 3)
%           8
%       end
      
      S(0 +1) = rem(k-1,2);
      S(1 +1) = rem(floor((k-1)/2),2);
      S(2 +1) = rem( floor((k-1)/4),2 );
      S(3 +1) = rem( floor((k-1)/8),2);
      S(4 +1) = rem( floor((k-1)/16),2);
      S(5 +1) = rem( floor((k-1)/32),2);
      
      Output(1,k,t,1 +1) = mod(1 + S(4 +1) + S(3 +1) + S(1 + 1) + S(0 +1), 2);
      Output(2,k,t,1 +1) = mod(1 + S(5 +1) + S(4 +1) + S(3 + 1) + S(0 +1), 2);
      Output(3,k,t,1 +1) = mod(1 + S(5 +1) + S(4 +1) + S(2 + 1) + S(0 +1), 2);
      
      S((0:4) +1) = S((1:5) +1);
      S(5 +1) = 1;
      nextState = S(0 +1)*(2^0) + S(1 +1)*(2^1) + S(2 +1)*(2^2) + S(3 +1)*(2^3) + S(4 +1)*(2^4) + S(5 +1)*(2^5);
      NextState(k,t,1 +1) = nextState;
      
      nPreviousStates = aPreviousStateCount(nextState +1,t+1);
      PreviousState(nextState +1, t+1,nPreviousStates +1) = k -1;
      aPreviousStateCount(nextState +1,t+1) = nPreviousStates + 1;
      assert(nPreviousStates+1 <= 2);

  end
end

PreviousState(:,1,1)=PreviousState(:,n+1,1);
PreviousState(:,1,2)=PreviousState(:,n+1,2);


% ToDo: BER Performance Tests. In the simple tests below, the decoding can
% tolerate bit-flips in three of the code-blocks below. Uncommenting the fourth block
% will give errors in decoding

%randomly flip bits
% D(1,30)= mod(D(1,30)+1,2);
% D(1,40)= mod(D(1,40)+1,2);
% D(1,20)= mod(D(1,20)+1,2);
% D(1,10)= mod(D(1,10)+1,2);
% D(1,5)= mod(D(1,5)+1,2);
% D(1,3)= mod(D(1,3)+1,2);
% D(2,3)= mod(D(2,3)+1,2);
% D(3,3)= mod(D(3,3)+1,2);
% 
% D(1,31)= mod(D(1,31)+1,2);
% %D(1,41)= mod(D(1,41)+1,2);
% D(1,21)= mod(D(1,21)+1,2);
% D(1,11)= mod(D(1,11)+1,2);
% D(1,6)= mod(D(1,6)+1,2);
% D(1,4)= mod(D(1,4)+1,2);
% D(2,4)= mod(D(2,4)+1,2);
% D(3,4)= mod(D(3,4)+1,2);

% D(1,28)= mod(D(1,28)+1,2);
% D(1,38)= mod(D(1,38)+1,2);
% D(1,18)= mod(D(1,18)+1,2);
% D(1,8)= mod(D(1,8)+1,2);
% D(1,3)= mod(D(1,3)+1,2);
% D(1,2)= mod(D(1,2)+1,2);
% D(2,2)= mod(D(2,2)+1,2);
% D(3,2)= mod(D(3,2)+1,2);

% D(1,29)= mod(D(1,29)+1,2);
% D(1,39)= mod(D(1,39)+1,2);
% D(1,19)= mod(D(1,19)+1,2);
% D(1,9)= mod(D(1,9)+1,2);
% D(1,4)= mod(D(1,4)+1,2);
% D(1,2)= mod(D(1,2)+1,2);
% D(2,1)= mod(D(2,1)+1,2);
% D(3,1)= mod(D(3,1)+1,2);


%Circular Viterbi Decoder
M = nM; % TODO: replace instances of M with nM
nMaxDepth = n+10*n;

Metric(1:2^length(S),1:(nMaxDepth+1))=10000;
Metric(:,1)=0;

CurrentState = 0;
nTend = 0;


for t=1:(nMaxDepth) % ToDo: fix approximate maximum time limit
  for k=1:(2^length(S))
      tp = rem(t-1,n)+1;
      d = findHammingDistanceErasure(D(:,tp), Output(:,k,tp,0 +1)) + Metric(k,t);
      if (d < Metric(NextState(k,tp,0 +1) +1,t+1))
          Metric(NextState(k,tp,0 +1) +1,t+1) = d;
          Message(k,t) = 0;
          nextState = NextState(k,tp,0 +1);
          PreviousBestState(nextState +1,t+1)= k -1;% ToDo: Check how to get rid of memory requirements of these traceback states
          
          a = PreviousState(nextState +1, tp+1,1 +1);
          b = PreviousState(nextState +1, tp+1,0 +1);
          statei = min(a,b);
          statej = max(a,b);
          if (k == statei +1)
              aDecisionWord(nextState +1,t+1) = 0;
          elseif (k == statej +1)
              aDecisionWord(nextState +1,t+1) = 1;
          else
              assert(1==0);
          end
      end
      
      d = findHammingDistanceErasure(D(:,tp), Output(:,k,tp,1 +1)) + Metric(k,t);
      if (d < Metric(NextState(k,tp,1 +1) +1,t+1))
          Metric(NextState(k,tp,1 +1) +1,t+1) = d;
          Message(k,t) = 1;
          nextState = NextState(k,tp,1 +1);
          PreviousBestState(nextState +1,t+1)= k -1;
          
          a = PreviousState(nextState +1, tp+1,1);
          b = PreviousState(nextState +1, tp+1,2);
          statei = min(a,b);
          statej = max(a,b);
          
          if (k == statei +1)
              aDecisionWord(nextState +1,t+1) = 0;
          elseif (k == statej +1)
              aDecisionWord(nextState +1,t+1) = 1;
          else
              assert(1==0);
          end
      end
      
      
  end
  
  if (t > n)
         if (aDecisionWord(:,t-n)==aDecisionWord(:,t))
             m = m+1;
             
             if (m >= M)
                 %t
                 %assert(1==2);% stopping rule 2 test for stopping rule 1;
                 for state=1:(2^length(S))
                     if ( (Metric(state,t) - Metric(state,t-n)) == (Metric(1,t) - Metric(1,t-n)))
                         continue;
                     else
                         break;
                     end
                 end
                 
                 if (state == 2^length(S))
                     break;
                     %assert(1==2); % both stopping rules satisfied
                 end
             end
             
         else
             m = 0;             
         end
  end
        
end


nTmax=t;
for t=nTmax-n+1:nTmax
    tp = rem(t-1,n)+1;
    aDecisionWord(:,tp) = aDecisionWord(:,t);
end
% Start trace back
t = n+10*M;
nCurrentState = 45;
for t=n+10*M:-1:1
    tp = rem(t-1,n)+1;
   
    a = PreviousState(nCurrentState +1, tp,1 +1);
    b = PreviousState(nCurrentState +1, tp,0 +1);
    statei = min(a,b);
    statej = max(a,b);
    
    if (aDecisionWord(nCurrentState +1,tp) == 0)
        nPreviousState = statei;
    else
        nPreviousState = statej;
    end
    
    if (t <= n+1 & t>1) % I think there is something very simple that can be done here
        if (NextState(nPreviousState +1,t-1,0 +1) == nCurrentState)
            aDecodedMessage(t-1)=0;
        elseif (NextState(nPreviousState +1,t-1,1 +1) == nCurrentState)
            aDecodedMessage(t-1)=1;
        else
            assert(1==0);
        end
    end
    debugstate(t)=nCurrentState;
    nCurrentState =nPreviousState;
    
end

%fprintf('\n\n number of errors is %d\n',length(find(aDecodedMessage ~= aBitArray')));




