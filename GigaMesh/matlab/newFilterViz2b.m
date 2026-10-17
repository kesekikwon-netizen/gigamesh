clear all; %close all; 

radius = 25;


peaksSurf = peaks(60).*1.5;
rasterSurf = peaksSurf(6:55,6:55);

rasterSurf = zeros(50,50);

%------------------------------------------
% tilt
for x=1:2*radius
   for y=1:2*radius
       rasterSurf(x,y) = rasterSurf(x,y)+ 0.1*x + 0.15*y;
   end
end
%------------------------------------------


x=[-24:25]; myFunc = (x).^3; myFunc = radius * myFunc / max( myFunc );
rasterSurf = repmat((myFunc),50,1);

%------------------------------------------


rasterSurf = rasterSurf - rasterSurf(26,26) + radius; %central pixel is always 0
rasterSurfCutAway = rasterSurf;

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
            %rasterSurfCutAway(x,y) = nan;
           end
       else
            voxelStack(x,y) = sqrt( toBeSqrt ); 
            rasterSurfCutAway(x,y) = nan;
       end
   end
end
voxelStackBot = -voxelStack + radius;
voxelStackTop =  voxelStack + radius;

startAZ = -20;
animFig = figure; hold on; 
set( animFig, 'color', [ 1.0 1.0 1.0 ] ); % set background to white
colormap hot; caxis( [0 2*radius]);
view( startAZ, 25 );
grid on; axis off;

h0 = surface( zeros(50,50)+.001 ); set( h0, 'EdgeColor', [ .3 .3 .3 ], 'FaceColor', 'black' );
%h4 = surface( rasterSurfCutAway );
axis tight; axis equal; axis vis3d;

ctr = 1;
    rel = 1;
    h3 = surface( rasterSurf-(radius-voxelStack)*ctr*rel ); 
    h1 = surface( voxelStackTop-(radius-voxelStack)*ctr*rel ); set( h1, 'EdgeAlpha', 0.1, 'FaceAlpha', 0.1 );
    h2 = surface( voxelStackBot-(radius-voxelStack)*ctr*rel ); set( h2, 'EdgeAlpha', 0.1, 'FaceAlpha', 0.1 );


startAZ = -20;
animFig = figure; hold on; 
set( animFig, 'color', [ 1.0 1.0 1.0 ] ); % set background to white
colormap hot; caxis( [0 2*radius]);
view( startAZ, 25 );
grid on; axis off;
h0 = surface( zeros(50,50)+.001 ); set( h0, 'EdgeColor', [ .3 .3 .3 ], 'FaceColor', 'black' );
h1 = surface( voxelStackTop );set( h1, 'EdgeAlpha', 0.1, 'FaceAlpha', 0.1 );
h2 = surface( voxelStackBot ); set( h2, 'EdgeAlpha', 0.1, 'FaceAlpha', 0.1 );
h3 = surface( rasterSurf+0*voxelStack );
h4 = surface( rasterSurfCutAway );
axis tight; axis equal; axis vis3d;




rasterSurfFinal = rasterSurf-(radius-voxelStack);
rasterSurfFinal( find( rasterSurfFinal < 0 ) ) = 0;
rasterVolume = min( rasterSurfFinal, voxelStack.*2 );
figResult = figure; hold on; axis off;
set( figResult, 'color', [ 1.0 1.0 1.0 ] ); % set background to white
colormap hot; caxis( [0 2*radius]);
surface( rasterVolume );
h0 = surface( zeros(50,50)+.001 ); set( h0, 'EdgeColor', [ .3 .3 .3 ], 'FaceColor', 'black' );
h1 = surface( zeros(50,50)+2*radius ); set( h0, 'EdgeColor', [ .3 .3 .3 ], 'FaceColor', 'black' );
view( startAZ, 25 );
axis tight; axis equal; axis vis3d;
delete( h1 );
view( -90, 90 );





