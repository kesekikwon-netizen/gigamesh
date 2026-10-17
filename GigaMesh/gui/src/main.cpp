//
// GigaMesh - The GigaMesh Software Framework is a modular software for display,
// editing and visualization of 3D-data typically acquired with structured light or
// structure from motion.
// Copyright (C) 2009-2020 Hubert Mara
//
// This file is part of GigaMesh.
//
// GigaMesh is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GigaMesh is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
//

//! \mainpage GigaMesh
//!
//! \author Hubert Mara

//! \todo check back later for debuging using http://www.cplusplus.com/reference/std/typeinfo/type_info/ AND find a nice way to demangle the output of .name()

// generic C++ includes:
#include <iostream>

// generic Qt includes:
#include <QApplication>
#include <QtOpenGL/QGLFormat>
#include <QMessageBox>
#include <QTranslator>

// Qt Interface includes:
#include "QGMMacros.h"
#include "QGMMainWindow.h"

// Qt data structure:
// none.

// C++ includes:
#include <GigaMesh/printbuildinfo.h>

using namespace std;

#include <GigaMesh/logging/Logging.h>

int main( int argc, char *argv[] ) {
	LOG::initLogging();

	QApplication app( argc, argv );

	//Initialize translation:
	QTranslator translator;
	QString localeName = QLocale::system().name();
	bool translationLoaded = false;

	// Try loading from translations/ folder first (for external translations)
	if(translator.load("GigaMesh_" + localeName, "./translations")) {
		app.installTranslator(&translator);
		translationLoaded = true;
		qDebug() << "Translation loaded from ./translations for locale:" << localeName;
	}
	// Fallback to embedded resources
	else if(translator.load("GigaMesh_" + localeName, ":/languages")) {
		app.installTranslator(&translator);
		translationLoaded = true;
		qDebug() << "Translation loaded from resources for locale:" << localeName;
	}

	if(!translationLoaded)
		qWarning() << "Failed to load translation for locale:" << localeName;

	// Application settings:
	app.setOrganizationName( "동국문화재연구원" );
	app.setApplicationName( "동국 실측 프로그램" );
	QCoreApplication::setApplicationVersion( VERSION_PACKAGE );

	// Command line parsing
	QCommandLineParser parser;
	parser.setApplicationDescription( "한국 고고학 유물 3D 실측 전용 프로그램" );
	parser.addHelpOption();
	parser.addVersionOption();
	// from example: parser.addPositionalArgument("source", QCoreApplication::translate("main", "Source file to copy."));
	// from example: parser.addPositionalArgument("destination", QCoreApplication::translate("main", "Destination directory."));

	// A boolean option with a single name
	QCommandLineOption showLoadLastOption( "load-last", 
	                                       QCoreApplication::translate( "main", "Load the last file used." ) );
	parser.addOption( showLoadLastOption );

	QCommandLineOption showHiDPI20Option( "hidpi20", 
	                                       QCoreApplication::translate( "main", "Assume HiDPI Display scale by a factor of 2." ) );
	parser.addOption( showHiDPI20Option );

	QCommandLineOption setLogLevelOption( "log-level",
	                                      QCoreApplication::translate( "main", "Sets the applications logging level [0-4]."),
	                                      "level", "1");
	parser.addOption( setLogLevelOption);

	// File name(s)
	parser.addPositionalArgument(  "filename", 
	                               QCoreApplication::translate( "main", "File with 3D-data.") );

	// Process the actual command line arguments given by the user
	parser.process( app );

	if(parser.isSet(setLogLevelOption))
	{
		QString param = parser.value(setLogLevelOption);
		if(param.size() != 1 || param[0] < '0' || param[0] > '4')
		{
			std::cerr << "The logging level has to be in the range of [0-4]." << std::endl;
		}
		else
		{
			LOG::setLogLevel(static_cast<LOG::LogLevel>(param.toInt()));
		}
	}

	const QStringList args = parser.positionalArguments();

	QString targetFile;
	if( args.size() > 1 ) {
		std::cout << "[Main] WARINING: More than one filename specified. Additional files will be ignored!" << std::endl;
	}
	if( args.size() >= 1 ) {
		targetFile = args.at( 0 );
		std::cout << "[Main] Loading: " << targetFile.toStdString() << std::endl;
	}

	bool loadLast      = parser.isSet( showLoadLastOption );
	bool hidpi20       = parser.isSet( showHiDPI20Option );

	if( ( targetFile.size() > 0 ) && loadLast ) {
		cerr << "ERROR: Exclusive options: --file and --load-last were given." << endl;
		exit( EXIT_FAILURE );
	}

	// Progress with starting GigaMesh
	cout << "[GigaMesh] using Qt Version: " << qVersion() << endl; // see: http://doc.trolltech.com/4.6/qtglobal.html
	printBuildInfo();

	// System checks:
	if( !QGLFormat::hasOpenGL() ) {
		LOG::fatal() << "[Main] ERROR: This system has no OpenGL support!\n";
		SHOW_MSGBOX_WARN( "No OpenGL", "ERROR: This system has no OpenGL support!" );
		exit( EXIT_FAILURE );
	}
	// Specify an OpenGL 3.2 format using the Core profile.
	// That is, no old-school fixed pipeline functionality
	QGLFormat glFormat; //QGLFormat is deprecated...
	//! \todo QSurfaceFormat is the replacment for the deprecated QGLFormat
	if(QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_4_3)
		glFormat.setVersion( 4, 3 );        //Higher Version needed to handle transparency in shader. Revert to 3.3 if it breaks core functionalities...
	else
		glFormat.setVersion( 3, 3 );                   // The version has to be larger than 3.3 due to geometry shaders and other usefull stuff.

	glFormat.setProfile( QGLFormat::CoreProfile ); // Do not even think to change the Profile to CompatibilityProfile !!!
	glFormat.setSampleBuffers( true );

	// The main window:
	QGMMainWindow mainWindow;
	mainWindow.show();
	mainWindow.setupMeshWidget( glFormat );
	if( hidpi20 ) {
		mainWindow.setupHighDPI20();
	}

	// Pass arguments to the main window
	if( targetFile.size() > 0 ) {
		mainWindow.load( targetFile );
	} else if( loadLast ) {
		if( !mainWindow.loadLast() ) {
			exit( EXIT_FAILURE );
		}
	} else {
		mainWindow.load();
	}

	return app.exec();
}
