a = [ -10 5 0 ];
b = [ 5 0 0 ];

aN = a / norm(a);
bN = b / norm(b);

axb = cross(a,b);
n = axb / norm( axb );

cross(a,b) == cross(b,a)

angle_sin = (axb/n) /  ( norm(a) * norm(b) )
angle_cos = dot(a,b) / ( norm(a) * norm(b) )
asin( angle_sin ) * 180/pi
acos( angle_cos ) * 180/pi

% http://tomyeah.com/signed-angle-between-two-vectors3d-in-cc/
% because N flips one has to use N from the polygon plane:
%signed_angle = atan2(  N * ( V1 x V2 ), V1 * V2  ); 
% so if we use n = a x b we actually don't get a signed angle
% however within the Mesh we can use n e.g. from a Face
signed_angle = atan2(  dot( n , cross( a , b ) ), dot( a,b) )*180/pi
signed_angle = atan2(  dot( n , cross( aN , bN ) ), dot( aN,bN ) )*180/pi
% n must be normalized