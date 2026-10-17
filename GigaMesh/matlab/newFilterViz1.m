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

rasterSurfSphere    = zeros( radius*2, radius*2 ); 

rasterSum   = zeros( radius*2, radius*2 ); 

for x=1:2*radius
   for y=1:2*radius
       toBeSqrt = (radius*.98)^2 -(x-0.5-radius)^2 -(y-0.5-radius)^2;
       if( toBeSqrt<0 )
           voxelStack(x,y) = nan;
           if( toBeSqrt > -75 )
              voxelStack(x,y) = 0;
           end
       else
            voxelStack(x,y) = sqrt( toBeSqrt ); 
       end
   end
end
voxelStackBot = -voxelStack + radius;
voxelStackTop =  voxelStack + radius;

startAZ = -20;
figShow = figure; hold on; 
set( figShow, 'color', [ 1.0 1.0 1.0 ] ); % set background to white
colormap hot; caxis( [0 2*radius]);
view( startAZ, 25 );
grid on; axis off;
h1 = surface( voxelStackTop );
h2 = surface( voxelStackBot );
surface( zeros(50,50));
axis tight; axis equal; axis vis3d;
angles = startAZ:0.3:startAZ+180;
rel = 1/length(angles);
ctr=0;
ctrFrames=0;
for azView = angles
    delete( h1 );
    delete( h2 );
    h1 = surface( voxelStackTop-(radius-voxelStack)*ctr*rel );
    h2 = surface( voxelStackBot-(radius-voxelStack)*ctr*rel );
    view( azView, 25 );
    %pause(0.01);
    filename = sprintf( 'frames/spherical_filtermask_seq_frame_%05i.png', ctrFrames );
    saveas( figShow, filename );
    ctrFrames = ctrFrames + 1;
    ctr=ctr+1;
end

elSteps = 25:01:90;
azDelta = ( 180 - azView )/(length(elSteps)-1);
ctr=0;
for elView = elSteps
    view( azView+ctr*azDelta, elView );
    %pause(0.01);
    filename = sprintf( 'frames/spherical_filtermask_seq_frame_%05i.png', ctrFrames );
    saveas( figShow, filename );
    ctrFrames = ctrFrames + 1;
    ctr=ctr+1;
end

