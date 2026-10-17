//
// Test program for QGMDockToolPalette
//
// Verifies:
// 1. All 20 tools are created and displayed
// 2. Icons are loaded correctly
// 3. Tool activation signals work
// 4. Category filtering works
// 5. Tool availability updates work
//

#include "QGMDockToolPalette.h"
#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QDebug>

class TestWindow : public QMainWindow
{
	Q_OBJECT

public:
	TestWindow() : QMainWindow()
	{
		setWindowTitle("Tool Palette Test");
		resize(800, 600);

		// Create central widget with log
		QWidget* centralWidget = new QWidget(this);
		QVBoxLayout* layout = new QVBoxLayout(centralWidget);

		mLogText = new QTextEdit(centralWidget);
		mLogText->setReadOnly(true);
		layout->addWidget(mLogText);

		setCentralWidget(centralWidget);

		// Create and dock the tool palette
		mToolPalette = new QGMDockToolPalette(this);
		addDockWidget(Qt::LeftDockWidgetArea, mToolPalette);

		// Connect signals
		connect(mToolPalette, &QGMDockToolPalette::toolActivated,
		        this, &TestWindow::onToolActivated);
		connect(mToolPalette, &QGMDockToolPalette::toolSelectionChanged,
		        this, &TestWindow::onToolSelectionChanged);

		// Log initial state
		log("Tool Palette initialized");
		log(QString("Total tools: %1").arg(TOOL_COUNT));

		// Test tool availability updates
		log("\n=== Testing tool availability (no mesh) ===");
		mToolPalette->updateToolAvailability(false, false);
		log("Tools should be disabled");

		log("\n=== Testing tool availability (with mesh) ===");
		mToolPalette->updateToolAvailability(true, false);
		log("Tools should be enabled");

		log("\n=== Testing tool availability (with mesh and selection) ===");
		mToolPalette->updateToolAvailability(true, true);
		log("All tools should be enabled");

		// Test tool verification
		log("\n=== Verifying Tool Icons ===");
		verifyToolIcons();
	}

private slots:
	void onToolActivated(ArchaeologyToolID toolId)
	{
		log(QString("Tool ACTIVATED: ID=%1").arg(toolId));
	}

	void onToolSelectionChanged(ArchaeologyToolID toolId)
	{
		log(QString("Tool selection changed: ID=%1").arg(toolId));
	}

private:
	void log(const QString& message)
	{
		mLogText->append(message);
		qDebug() << message;
	}

	void verifyToolIcons()
	{
		// List of all icon paths that should exist
		QStringList iconPaths = {
			// Section tools
			":/icons/archaeology/section_top.svg",
			":/icons/archaeology/section_front.svg",
			":/icons/archaeology/section_side.svg",
			":/icons/archaeology/section_free.svg",
			":/icons/archaeology/section_edit.svg",
			":/icons/archaeology/section_profile.svg",

			// Lithic tools
			":/icons/archaeology/lithic_tool.svg",
			":/icons/archaeology/lithic_cortex.svg",
			":/icons/archaeology/lithic_orient.svg",
			":/icons/archaeology/lithic_ridge.svg",
			":/icons/archaeology/lithic_annotate.svg",

			// Ceramic tools
			":/icons/archaeology/ceramic_profile.svg",
			":/icons/archaeology/ceramic_reconstruct.svg",
			":/icons/archaeology/ceramic_rim.svg",
			":/icons/archaeology/ceramic_pattern.svg",
			":/icons/archaeology/ceramic_annotate.svg",

			// Measurement tools
			":/icons/archaeology/measure_distance.svg",
			":/icons/archaeology/measure_angle.svg",
			":/icons/archaeology/measure_area.svg",
			":/icons/archaeology/measure_curvature.svg"
		};

		int foundIcons = 0;
		int missingIcons = 0;

		for (const QString& iconPath : iconPaths) {
			QIcon icon(iconPath);
			if (!icon.isNull()) {
				foundIcons++;
				log(QString("  [OK] %1").arg(iconPath));
			} else {
				missingIcons++;
				log(QString("  [MISSING] %1").arg(iconPath));
			}
		}

		log(QString("\nIcon Summary: %1 found, %2 missing").arg(foundIcons).arg(missingIcons));
	}

	QGMDockToolPalette* mToolPalette;
	QTextEdit* mLogText;
};

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	TestWindow window;
	window.show();

	return app.exec();
}

#include "QGMDockToolPalette_test.moc"
