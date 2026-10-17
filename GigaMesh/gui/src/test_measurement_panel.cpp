//
// Test program for QGMDockMeasurement
// Demonstrates the Measurement Panel functionality
//

#include "QGMDockMeasurement.h"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	// Create main window
	QMainWindow mainWindow;
	mainWindow.setWindowTitle("Measurement Panel Test");
	mainWindow.resize(800, 600);

	// Create the measurement dock
	QGMDockMeasurement* measurementDock = new QGMDockMeasurement(&mainWindow);
	mainWindow.addDockWidget(Qt::RightDockWidgetArea, measurementDock);

	// Create a central widget with test buttons
	QWidget* centralWidget = new QWidget(&mainWindow);
	QVBoxLayout* layout = new QVBoxLayout(centralWidget);

	QPushButton* btnAddDistance = new QPushButton("Add Distance Measurement", centralWidget);
	QPushButton* btnAddAngle = new QPushButton("Add Angle Measurement", centralWidget);
	QPushButton* btnAddArea = new QPushButton("Add Area Measurement", centralWidget);
	QPushButton* btnAddMultiple = new QPushButton("Add 10 Random Measurements", centralWidget);

	layout->addWidget(btnAddDistance);
	layout->addWidget(btnAddAngle);
	layout->addWidget(btnAddArea);
	layout->addWidget(btnAddMultiple);
	layout->addStretch();

	mainWindow.setCentralWidget(centralWidget);

	// Test: Add some sample measurements
	qDebug() << "Adding initial test measurements...";

	Measurement m1;
	m1.id = 1;
	m1.type = MeasurementType::Distance;
	m1.value = 123.45;
	m1.unit = "mm";
	m1.label = "Edge length";
	m1.visible = true;
	measurementDock->addMeasurement(m1);

	Measurement m2;
	m2.id = 2;
	m2.type = MeasurementType::Angle;
	m2.value = 45.0;
	m2.unit = "°";
	m2.label = "Corner angle";
	m2.visible = true;
	measurementDock->addMeasurement(m2);

	Measurement m3;
	m3.id = 3;
	m3.type = MeasurementType::Area;
	m3.value = 2500.75;
	m3.unit = "mm²";
	m3.label = "Surface area";
	m3.visible = false;
	measurementDock->addMeasurement(m3);

	// Connect test buttons
	int nextId = 4;

	QObject::connect(btnAddDistance, &QPushButton::clicked, [&]() {
		Measurement m;
		m.id = nextId++;
		m.type = MeasurementType::Distance;
		m.value = 100.0 + (rand() % 900) / 10.0;
		m.unit = "mm";
		m.label = QString("Distance %1").arg(m.id);
		m.visible = true;
		measurementDock->addMeasurement(m);
		qDebug() << "Added distance measurement" << m.id;
	});

	QObject::connect(btnAddAngle, &QPushButton::clicked, [&]() {
		Measurement m;
		m.id = nextId++;
		m.type = MeasurementType::Angle;
		m.value = (rand() % 180);
		m.unit = "°";
		m.label = QString("Angle %1").arg(m.id);
		m.visible = true;
		measurementDock->addMeasurement(m);
		qDebug() << "Added angle measurement" << m.id;
	});

	QObject::connect(btnAddArea, &QPushButton::clicked, [&]() {
		Measurement m;
		m.id = nextId++;
		m.type = MeasurementType::Area;
		m.value = 1000.0 + (rand() % 9000) / 10.0;
		m.unit = "mm²";
		m.label = QString("Area %1").arg(m.id);
		m.visible = true;
		measurementDock->addMeasurement(m);
		qDebug() << "Added area measurement" << m.id;
	});

	QObject::connect(btnAddMultiple, &QPushButton::clicked, [&]() {
		for (int i = 0; i < 10; ++i) {
			Measurement m;
			m.id = nextId++;

			int typeRand = rand() % 3;
			if (typeRand == 0) {
				m.type = MeasurementType::Distance;
				m.value = 50.0 + (rand() % 500) / 10.0;
				m.unit = "mm";
			} else if (typeRand == 1) {
				m.type = MeasurementType::Angle;
				m.value = (rand() % 180);
				m.unit = "°";
			} else {
				m.type = MeasurementType::Area;
				m.value = 500.0 + (rand() % 5000) / 10.0;
				m.unit = "mm²";
			}

			m.label = QString("Test %1").arg(m.id);
			m.visible = (rand() % 2) == 0;
			measurementDock->addMeasurement(m);
		}
		qDebug() << "Added 10 random measurements";
	});

	// Connect signals from measurement dock
	QObject::connect(measurementDock, &QGMDockMeasurement::measurementVisibilityChanged,
	                 [](int id, bool visible) {
		                 qDebug() << "Measurement" << id << "visibility changed to" << visible;
	                 });

	QObject::connect(measurementDock, &QGMDockMeasurement::measurementDeleted,
	                 [](int id) {
		                 qDebug() << "Measurement" << id << "was deleted";
	                 });

	mainWindow.show();

	qDebug() << "Measurement Panel Test Application started";
	qDebug() << "Use the buttons to add measurements, or right-click in the table for options";

	return app.exec();
}
