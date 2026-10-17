// GigaMesh main.cpp - Korean language forced version
// Modified to always load Korean translations

#include <iostream>
#include <QApplication>
#include <QtOpenGL/QGLFormat>
#include <QMessageBox>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>

#include "QGMMacros.h"
#include "QGMMainWindow.h"
#include <GigaMesh/printbuildinfo.h>

using namespace std;

#include <GigaMesh/logging/Logging.h>

int main( int argc, char *argv[] ) {
	LOG::initLogging();

	QApplication app( argc, argv );

	// Force Korean translation loading
	QTranslator translator;
	QTranslator qtTranslator;

	bool translationLoaded = false;

	// Try multiple paths to find Korean translation
	QStringList translationPaths;
	translationPaths << "./translations"
	                 << "."
	                 << QApplication::applicationDirPath() + "/translations"
	                 << QApplication::applicationDirPath()
	                 << ":/languages";

	// Load GigaMesh Korean translation
	for (const QString &path : translationPaths) {
		if (translator.load("GigaMesh_ko", path)) {
			app.installTranslator(&translator);
			translationLoaded = true;
			qDebug() << "Korean translation loaded from:" << path;
			break;
		}
	}

	// Load Qt Korean translation
	for (const QString &path : translationPaths) {
		if (qtTranslator.load("qt_ko", path)) {
			app.installTranslator(&qtTranslator);
			qDebug() << "Qt Korean translation loaded from:" << path;
			break;
		}
	}

	if (!translationLoaded) {
		qWarning() << "Failed to load Korean translation!";
		qWarning() << "Tried paths:" << translationPaths;
	}

	// Application settings
	app.setOrganizationName( "GigaMesh" );
	app.setApplicationName( "GigaMesh" );
	QCoreApplication::setApplicationVersion( VERSION_PACKAGE );

	// Command line parsing (rest of original main.cpp continues...)
	QCommandLineParser parser;
	parser.setApplicationDescription( "GigaMesh Software Framework GUI Interface" );
	parser.addHelpOption();
	parser.addVersionOption();

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

	parser.addPositionalArgument(  "filename",
	                               QCoreApplication::translate( "main", "File with 3D-data.") );

	parser.process( app );

	if(parser.isSet(setLogLevelOption))
	{
		bool conversionOk = false;
		int logLevel = parser.value(setLogLevelOption).toInt(&conversionOk);

		if(conversionOk)
			LOG::setMinSeverity(static_cast<LOG::Severity>(logLevel));
		else
			qWarning() << "Could not parse log level";
	}

	QGMMainWindow* mainWindow = new QGMMainWindow();
	mainWindow->show();

	if( parser.isSet( showLoadLastOption ) ) {
		mainWindow->loadLast();
	}

	if( parser.isSet( showHiDPI20Option ) ) {
		mainWindow->setDPIScaleFactor( 2.0 );
	}

	const QStringList argFileNames = parser.positionalArguments();
	if( argFileNames.size() == 1 ) {
		mainWindow->open( argFileNames.at(0) );
	} else if( argFileNames.size() > 1 ) {
		cerr << "[GigaMesh] ERROR: Multiple files given!" << endl;
	}

	return( app.exec() );
}
