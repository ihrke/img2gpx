ic = [ 465 552 316; 684 996 1182 ];
pc = [  61.9150 61.8082 61.7082; 9.8511 9.6830 9.4830 ];

A = [];
p = [];
for i=1:size(ic,2)
	A = [A; ic(1,i) ic(2,i) 1 0 0 0; 0 0 0 ic(1,i) ic(2,i) 1 ]
	p = [p; pc(1,i);pc(2,i) ];
end

a = A'*A\A'*p;

M = [a(1) a(2) a(3); a(4) a(5) a(6) ]
