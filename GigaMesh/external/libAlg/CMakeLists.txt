cmake_minimum_required(VERSION 3.10)

project(libalg LANGUAGES CXX)
# reduces set of sources for GigaMesh
# add firther sources if needed, e.g. if there are linker errors for alglib
set(ALG_SOURCES
	src/ablas.cpp		include/alg/ablas.h
	src/ablasf.cpp		include/alg/ablasf.h
#	src/airyf.cpp
	src/ap.cpp			include/alg/ap.h
#	src/apserv.cpp
#	src/autogk.cpp
#	src/bdss.cpp
#	src/bdsvd.cpp
#	src/bessel.cpp
#	src/betaf.cpp
#	src/binomialdistr.cpp
#	src/blas.cpp
#	src/chebyshev.cpp
#	src/chisquaredistr.cpp
	src/conv.cpp		include/alg/conv.h
	src/corr.cpp		include/alg/corr.h
#	src/correlation.cpp
#	src/correlationtests.cpp
#	src/creflections.cpp
#	src/dawson.cpp
#	src/densesolver.cpp
#	src/descriptivestatistics.cpp
#	src/dforest.cpp
#	src/elliptic.cpp
#	src/estnorm.cpp
#	src/evd.cpp
#	src/expintegrals.cpp
#	src/fdistr.cpp
	src/fft.cpp			include/alg/fft.h
#	src/fht.cpp
#	src/fresnel.cpp
	src/ftbase.cpp		include/alg/ftbase.h
#	src/gammafunc.cpp
#	src/gkq.cpp
#	src/gq.cpp
#	src/hblas.cpp
#	src/hermite.cpp
#	src/hqrnd.cpp
#	src/hsschur.cpp
	src/ialglib.cpp		include/alg/ialglib.h
#	src/ibetaf.cpp
#	src/idwint.cpp
#	src/igammaf.cpp
#	src/inverseupdate.cpp
#	src/jacobianelliptic.cpp
#	src/jarquebera.cpp
#	src/kmeans.cpp
#	src/laguerre.cpp
#	src/lda.cpp
#	src/ldlt.cpp
#	src/legendre.cpp
#	src/linmin.cpp
#	src/linreg.cpp
#	src/logit.cpp
#	src/lsfit.cpp
#	src/mannwhitneyu.cpp
#	src/matdet.cpp
#	src/matgen.cpp
#	src/matinv.cpp
#	src/minasa.cpp
#	src/mincg.cpp
#	src/minlbfgs.cpp
#	src/minlm.cpp
#	src/mlpbase.cpp
#	src/mlpe.cpp
#	src/mlptrain.cpp
#	src/nearestneighbor.cpp
#	src/nearunityunit.cpp
#	src/normaldistr.cpp
#	src/odesolver.cpp
#	src/ortfac.cpp
#	src/pca.cpp
#	src/poissondistr.cpp
#	src/polint.cpp
#	src/psif.cpp
#	src/pspline.cpp
#	src/ratint.cpp
#	src/ratinterpolation.cpp
#	src/rcond.cpp
#	src/reflections.cpp
#	src/rotations.cpp
#	src/safesolve.cpp
#	src/sblas.cpp
#	src/schur.cpp
#	src/sdet.cpp
#	src/sinverse.cpp
#	src/spdgevd.cpp
#	src/spline1d.cpp
#	src/spline2d.cpp
#	src/spline3.cpp
#	src/srcond.cpp
#	src/ssolve.cpp
#	src/stest.cpp
#	src/studenttdistr.cpp
#	src/studentttests.cpp
#	src/svd.cpp
#	src/trfac.cpp
#	src/trigintegrals.cpp
#	src/trlinsolve.cpp
#	src/tsort.cpp
#	src/variancetests.cpp
#	src/wsr.cpp
#	src/xblas.cpp
)

add_library(alg STATIC ${ALG_SOURCES})

target_compile_features(alg PRIVATE cxx_std_17)

target_include_directories(alg PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
								$<INSTALL_INTERFACE:include>
							   PRIVATE
								${CMAKE_CURRENT_SOURCE_DIR}/src
								${CMAKE_CURRENT_SOURCE_DIR}/include/alg)

