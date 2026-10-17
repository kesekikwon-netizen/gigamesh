vertexNr1 = 156238; % aussenkante
vertexNr2 = 228725; % ridge
vertexNr3 = 235427; % surface
188026 % corner
%vertexNr = 1180331; % non cunei

featVecNr1 = find( indices == vertexNr1 );
featVecNr2 = find( indices == vertexNr2 );
featVecNr3 = find( indices == vertexNr3 );

NFFT=16;

step = 360/(size(features0,2));
asc  = 0:step:360-step;

figure;
hold on;
plot(asc,features0(featVecNr1,:),'ro-','Linewidth',2)
xlim([0 360])
xlabel('(Degrees)')
ylabel('Feaure Value')
title( [ '{\bf Vertex ' int2str( vertexNr1 ) '}' ] )
grid on;

d = fft(features0,NFFT,2);
%dec  = features0( featVecNr, : );
%d = fft(dec,NFFT,2);
m = NFFT; % was: length(dec);
M = floor((m+1)/2);

a0 = d(featVecNr1,1)/m; 
an = 2*real(d(featVecNr1,2:M))/m;
bn = -2*imag(d(featVecNr1,2:M))/m;
aM1 = d(featVecNr1,M+1)/m;

x = 0:0.01:360.*(size(features0,2)/NFFT);
n = 1:length(an);
y = a0 + an*cos(2*pi*n'*x/360) ...
       + bn*sin(2*pi*n'*x/360) ...
       + aM1*cos(2*pi*(max(n)+1)*x/360); 

xBase = 0:0.01.*(NFFT/size(features0,2)):360;
yMainSin = an(1) *cos(2*pi*1*x/360) +bn(1)*sin(2*pi*1*x/360);
yMainSin2 = an(2) *cos(2*pi*2*x/360) +bn(1)*sin(2*pi*2*x/360);
plot(xBase,y,'Linewidth',2)
plot(xBase,yMainSin,'Linewidth',1)
plot(xBase,yMainSin2,'Linewidth',1)
legend('Data','DFT Interpolant','Location','NW')

%mainAmps = abs(d(:,1:3));
%toWriteWithIdx = [ indices'; mainAmps' ]';

%toSub1 = repmat( d( featVecNr1, : ), length( indices ), 1 );
%toSub2 = repmat( d( featVecNr2, : ), length( indices ), 1 );
%dist1 = real( sum(( d - toSub ).^2,2) );
%dist2 = real( sum(( d - toSub ).^2,2) );

dn = zeros( length(indices), NFFT*2 );
dn( 1:end, 1:2:end ) = real( d );
dn( 1:end, 2:2:end ) = imag( d );

toSub1 = repmat( dn( featVecNr1, : ), length( indices ), 1 );
toSub2 = repmat( dn( featVecNr2, : ), length( indices ), 1 );
toSub3 = repmat( dn( featVecNr3, : ), length( indices ), 1 );
dist1  = sqrt(sum(( dn - toSub1 ).^2,2));
dist2  = sqrt(sum(( dn - toSub2 ).^2,2));
dist3  = sqrt(sum(( dn - toSub3 ).^2,2));

figure; hold on;
hist( dist1, 1000 );
hist( dist2, 1000 );
hist( dist3, 1000 );
dist=min(dist1,min(dist2,dist3));
break;
dist=min(dist1,dist2);

% make hsv by sort - not distance:
[y,is] = sort( dist );
cmap = hsv( length( indices ) );
hsvMap = cmap( is, : );
fid = fopen( '../mesh/Warka_W20219_1_CLEAN_WITHOUT_HOLES_r0.5_255_FFT_dist16_HSV_sort.tex', 'w');
fprintf( fid, '%i %f %f %f\n', [ indices(is)'; cmap' ] );
fclose( fid );

% make grayscale by distance:
cutOffVal = 0.32;
cutOffIdx=find(dist>cutOffVal);
dist( cutOffIdx ) = cutOffVal;
distNormalized=dist./max(dist);
grayMap = repmat( distNormalized, 1, 3 );
%grayMap = floor( repmat( distNormalized, 1, 3 ) .*255 );

fid = fopen( '../mesh/Warka_W20219_1_CLEAN_WITHOUT_HOLES_r0.5_255_FFT_dist12_gray.tex', 'w');
fprintf( fid, '%i %f %f %f\n', [ indices'; grayMap' ] )
%fprintf( fid, '%i %i %i %i\n', [ indices'; grayMap' ] )
fclose( fid );

hotVals=hot(1000);
hotMap = hotVals( int16( floor(1+distNormalized*999) ), : );

fid = fopen( '../mesh/Warka_W20219_1_CLEAN_WITHOUT_HOLES_r0.5_255_FFT_dist12_hot.tex', 'w');
fprintf( fid, '%i %f %f %f\n', [ indices'; hotMap' ] )
%fprintf( fid, '%i %i %i %i\n', [ indices'; grayMap' ] )
fclose( fid );

%dlmwrite( fileNameOut2, mainAmps, 'delimiter', ' ', 'precision', 10 );
%dlmwrite( fileNameOut3, toWriteWithIdx, 'delimiter', ' ', 'precision', 10 );