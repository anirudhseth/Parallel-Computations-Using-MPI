#compare plots<br/>
fromMPI = importdata('output.txt')<br/>
u_ode= @(x) power(x,2) - x<br/>
N=1000<br/>
x = linspace(0, 1, N)<br/>
u = u_ode(x)<br/>
plot(u)<br/>
hold all<br/>
plot(fromMPI)<br/>



#solve pde analytically <br/>
#https://se.mathworks.com/help/symbolic/solve-a-single-differential-equation.html<br/>
syms y(x)<br/>
ode = diff(y,x,2) ==x*y+2-x *x *(x-1)<br/>
cond1 = y(0) == 0<br/>
cond2 = y(1) == 0<br/>
conds = [cond1 cond2];<br/>
ySol(x) = dsolve(ode,conds)<br/>
