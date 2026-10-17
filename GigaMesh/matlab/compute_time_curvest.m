e = [];
for i=[1:100]
   [X,Y,Z] = peaks(i*100); 
   t=cputime; 
   [K,H,P1,P2] = surfature(X,Y,Z); 
   e(end+1) = cputime-t
end