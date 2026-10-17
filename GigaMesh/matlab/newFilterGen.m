clear all; close all; 

radius = 25;


rasterSurf = peaks(50);
%rasterSurf = zeros(50,50);


for x=1:2*radius
   for y=1:2*radius
       rasterSurf(x,y) = rasterSurf(x,y)+ 0.1*x + 0.15*y;
   end
end

rasterSurf = rasterSurf - rasterSurf(26,26); %central pixel is always 0


voxelStack    = zeros( radius*2, radius*2 );
voxelStack90p = zeros( radius*2, radius*2 );

rasterSurfSphere    = zeros( radius*2, radius*2 ); 
rasterSurfSphere90p = zeros( radius*2, radius*2 ); 

rasterSum   = zeros( radius*2, radius*2 ); 
rasterSum90p   = zeros( radius*2, radius*2 ); 

for x=1:2*radius
   for y=1:2*radius
       toBeSqrt = (radius*.98)^2 -(x-0.5-radius)^2 -(y-0.5-radius)^2;
       if( toBeSqrt<0 )
           voxelStack(x,y) = nan;
           if( toBeSqrt > -75 )
               voxelStack(x,y) = 0;
           end
       else
            voxelStack(x,y) = 2*sqrt( toBeSqrt ); 
       end
       toBeSqrt = (radius*0.9)^2 -(x-1-radius)^2 -(y-1-radius)^2;
       if( toBeSqrt<0 )
            voxelStack90p(x,y) = nan;
       else
            voxelStack90p(x,y) = 2*sqrt( toBeSqrt ); 
       end
   end
end


rasterSurfSphere    = rasterSurf + ( voxelStack ./2 );
rasterSurfSphere90p = rasterSurf + ( voxelStack90p ./2 );

figure; hold on; colormap hot;
surface( voxelStack );
title( 'voxelStack' );
axis tight; axis equal;

figure; hold on; colormap hot; 
surface( rasterSurf );
title( 'rasterSurf' );
axis tight; axis equal;

figure; hold on;  colormap hot;
surface( rasterSurfSphere );
title( 'rasterSurfSphere' );
axis tight; axis equal;

figure; hold on;  colormap hot;
surface( rasterSurfSphere );
surface( voxelStack );
axis tight; axis equal;


for x=1:2*radius
   for y=1:2*radius
       toBeSqrt = (radius*.98)^2 -(x-1-radius)^2 -(y-1-radius)^2;
       if( toBeSqrt<0 )
           rasterSum(x,y)=nan;
           if( toBeSqrt > -65 )
               rasterSum(x,y) = 0;
           end
       elseif( rasterSurfSphere(x,y) < 0 )
           rasterSum(x,y) = 0;
       elseif( rasterSurfSphere(x,y) < voxelStack(x,y) )
           rasterSum(x,y) = rasterSurfSphere(x,y);
       else
           rasterSum(x,y) = voxelStack(x,y);
       end
       
       toBeSqrt = (radius*0.9)^2 -(x-1-radius*0.9)^2 -(y-1-radius*0.9)^2;
       if( toBeSqrt<0 )
           rasterSum90p(x,y)=nan;
       elseif( rasterSurfSphere90p(x,y) < 0 )
           rasterSum90p(x,y) = nan;
       elseif( rasterSurfSphere90p(x,y) < voxelStack90p(x,y) )
           rasterSum90p(x,y) = rasterSurfSphere90p(x,y);
       else
           rasterSum90p(x,y) = voxelStack90p(x,y);
       end
    end
end

figure; hold on;  colormap hot;
surface( rasterSum );
axis tight; axis equal;


surfCheck = rasterSum - ( voxelStack./2 );

figure; hold on;  colormap hot;
surface( surfCheck );
%surface( peaks(50)+radius );
%surface( omes(50,50)+radius );
axis tight; axis equal;


surfCheck90p = rasterSum90p - ( voxelStack90p./2 );

figure; hold on;  colormap hot;
surface( surfCheck90p );
%surface( peaks(50)+radius );
%surface( omes(50,50)+radius );
axis tight; axis equal;


