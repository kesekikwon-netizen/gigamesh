fileNameIn = 'test.facen';
%fileNameIn = 'HOS_G10.facen';

% array for labels
facenr = [];
% element array for polylines:
phi = [];
theta = [];
radius = [];
lineVals = [];

fid = fopen( fileNameIn, 'r' );

% fetch first line
tline = fgetl( fid );
while ischar( tline )
    % lines with comments begin with # and can be ignored.
    if( tline(1) == '#' )
        disp( tline );
        tline = fgetl( fid );
        continue;
    end
    % parse the rest:
    lineVals( end+1, : ) = sscanf( tline, '%f' );
    % first element is the label no. 
    % labels from gigamesh start at 0 - so we have to add 1
    %facenr( end+1 ) = lineVals( 1 ) +1;
    % second element is the length of the polyline
    %phi( end+1 )    = lineVals( 2 );
    %theta( end+1 )  = lineVals( 3 );
    %radius( end+1 ) = lineVals( 4 );
    tline = fgetl( fid );
end
fclose( fid );

figure; hold on; axis equal; axis vis3d; grid on;
%title( 'Polylines colored by Label No.' );
plot3( phi, theta, radius, 'k.' );
