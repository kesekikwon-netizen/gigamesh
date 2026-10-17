/* * GigaMesh - The GigaMesh Software Framework is a modular software for display,
 * editing and visualization of 3D-data typically acquired with structured light or
 * structure from motion.
 * Copyright (C) 2009-2020 Hubert Mara
 *
 * This file is part of GigaMesh.
 *
 * GigaMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GigaMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sectionmanager.h"
#include "svg/SvgWriter.h"
#include "svg/SvgPath.h"
#include "svg/SvgText.h"
#include <GigaMesh/mesh/mesh.h>
#include <GigaMesh/mesh/vertex.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <iostream>

SectionManager::SectionManager(QObject* parent)
    : QObject(parent)
    , mActiveSection(nullptr)
{
}

SectionManager::~SectionManager()
{
	clearSections();
}

Section* SectionManager::getSection(int index) const
{
	if (index >= 0 && index < mSections.size()) {
		return mSections[index];
	}
	return nullptr;
}

int SectionManager::getActiveSectionIndex() const
{
	return mSections.indexOf(mActiveSection);
}

void SectionManager::addSection(Section* section)
{
	if (section != nullptr) {
		mSections.append(section);
		if (mActiveSection == nullptr) {
			mActiveSection = section;
		}
		emit sectionAdded(section);
	}
}

void SectionManager::removeSection(int index)
{
	if (index >= 0 && index < mSections.size()) {
		Section* section = mSections[index];
		mSections.removeAt(index);

		if (mActiveSection == section) {
			mActiveSection = mSections.isEmpty() ? nullptr : mSections.first();
			emit activeSectionChanged(mActiveSection);
		}

		delete section;
		emit sectionRemoved(index);
	}
}

void SectionManager::clearSections()
{
	for (Section* section : mSections) {
		delete section;
	}
	mSections.clear();
	mActiveSection = nullptr;
	emit sectionsCleared();
}

void SectionManager::setActiveSection(int index)
{
	if (index >= 0 && index < mSections.size()) {
		setActiveSection(mSections[index]);
	}
}

void SectionManager::setActiveSection(Section* section)
{
	if (section != mActiveSection && mSections.contains(section)) {
		mActiveSection = section;
		emit activeSectionChanged(section);
	}
}

Section* SectionManager::createSection(const QString& name, const Vector3D& planeHNF)
{
	Section* section = new Section(name, planeHNF);
	addSection(section);
	return section;
}

Section* SectionManager::createPresetSection(Section::SectionType type, const Vector3D& centerPoint)
{
	Section* section = Section::createPreset(type, centerPoint);
	addSection(section);
	return section;
}

//! Calculate intersection polyline for a section
//! @param mesh The mesh to intersect with
//! @param section The section to calculate intersection for
//! @returns true if intersection was calculated successfully
bool SectionManager::calculateIntersection(Mesh* mesh, Section* section)
{
	if (mesh == nullptr || section == nullptr) {
		return false;
	}

	// Use the existing calcIntersectionPolylineWithPlane function
	std::vector<Vector3D> intersectionPoints;
	if (!mesh->calcIntersectionPolylineWithPlane(section->getPlaneHNF(), &intersectionPoints)) {
		std::cerr << "[SectionManager::" << __FUNCTION__ << "] Failed to calculate intersection" << std::endl;
		return false;
	}

	if (intersectionPoints.size() < 2) {
		std::cerr << "[SectionManager::" << __FUNCTION__ << "] Not enough intersection points: "
		          << intersectionPoints.size() << std::endl;
		return false;
	}

	// Create a new PolyLine from the intersection points
	PolyLine* polyLine = new PolyLine();
	for (const auto& point : intersectionPoints) {
		Vector3D normal(0.0, 0.0, 1.0);  // Default normal
		polyLine->addBack(point, normal);
	}

	// Store the polyline in the section
	section->setPolyLine(polyLine);

	std::cout << "[SectionManager::" << __FUNCTION__ << "] Created polyline with "
	          << intersectionPoints.size() << " points for section: "
	          << section->getName().toStdString() << std::endl;

	return true;
}

void SectionManager::calculateAllIntersections(Mesh* mesh)
{
	for (Section* section : mSections) {
		calculateIntersection(mesh, section);
	}
}

//! Export a single section to SVG file
bool SectionManager::exportSectionToSVG(Section* section, const QString& filename)
{
	if (section == nullptr || !section->hasPolyLine()) {
		std::cerr << "[SectionManager::" << __FUNCTION__ << "] Section has no polyline to export" << std::endl;
		return false;
	}

	SvgWriter writer;

	// Calculate bounding box for canvas size
	PolyLine* polyLine = section->getPolyLine();
	double minX, minY, minZ, maxX, maxY, maxZ;
	polyLine->getBoundingBox(&minX, &minY, &minZ, &maxX, &maxY, &maxZ, false, false);

	double width = maxX - minX;
	double height = maxY - minY;
	double margin = 20.0;  // 20mm margin

	writer.setSize((width + 2 * margin), (height + 2 * margin));

	// Layer 1: Section intersection line
	writeSectionLayerToSVG(writer, section, -minX + margin, -minY + margin, 1.0);

	// Layer 2: Grid (if enabled)
	if (section->isGridEnabled()) {
		writeGridLayerToSVG(writer, section, -minX + margin, -minY + margin, 1.0);
	}

	// Layer 3: Scale bar (if enabled)
	if (section->isScaleBarEnabled()) {
		writeScaleBarToSVG(writer, section, margin, height + margin + 10, 1.0);
	}

	writer.writeToFile(filename.toStdWString());
	std::cout << "[SectionManager::" << __FUNCTION__ << "] Exported section to: "
	          << filename.toStdString() << std::endl;

	return true;
}

//! Export all sections to SVG (either one file with all layers, or separate files)
bool SectionManager::exportAllSectionsToSVG(const QString& filename, bool separateFiles)
{
	if (mSections.isEmpty()) {
		std::cerr << "[SectionManager::" << __FUNCTION__ << "] No sections to export" << std::endl;
		return false;
	}

	if (separateFiles) {
		// Export each section to a separate file
		QFileInfo fileInfo(filename);
		QString baseName = fileInfo.completeBaseName();
		QString dir = fileInfo.absolutePath();
		QString ext = fileInfo.suffix();

		for (int i = 0; i < mSections.size(); ++i) {
			Section* section = mSections[i];
			QString sectionFilename = QString("%1/%2_%3.%4")
			                          .arg(dir)
			                          .arg(baseName)
			                          .arg(section->getName())
			                          .arg(ext);
			if (!exportSectionToSVG(section, sectionFilename)) {
				std::cerr << "[SectionManager::" << __FUNCTION__ << "] Failed to export section "
				          << i << std::endl;
			}
		}
		return true;
	} else {
		// Export all sections to one file with multiple layers
		// TODO: Implement multi-section layout
		std::cerr << "[SectionManager::" << __FUNCTION__ << "] Multi-section single file export not yet implemented" << std::endl;
		return false;
	}
}

//! Export selected sections to SVG
bool SectionManager::exportSelectedSectionsToSVG(const QList<int>& indices, const QString& filename, bool separateFiles)
{
	// Create temporary section list with selected sections
	QList<Section*> selectedSections;
	for (int index : indices) {
		if (index >= 0 && index < mSections.size()) {
			selectedSections.append(mSections[index]);
		}
	}

	if (selectedSections.isEmpty()) {
		std::cerr << "[SectionManager::" << __FUNCTION__ << "] No valid sections selected" << std::endl;
		return false;
	}

	// For now, just export each selected section separately
	QFileInfo fileInfo(filename);
	QString baseName = fileInfo.completeBaseName();
	QString dir = fileInfo.absolutePath();
	QString ext = fileInfo.suffix();

	for (Section* section : selectedSections) {
		QString sectionFilename = QString("%1/%2_%3.%4")
		                          .arg(dir)
		                          .arg(baseName)
		                          .arg(section->getName())
		                          .arg(ext);
		if (!exportSectionToSVG(section, sectionFilename)) {
			std::cerr << "[SectionManager::" << __FUNCTION__ << "] Failed to export section" << std::endl;
		}
	}

	return true;
}

//! Write section intersection line to SVG
void SectionManager::writeSectionLayerToSVG(SvgWriter& writer, Section* section,
                                             double offsetX, double offsetY, double scale)
{
	writer.startGroup();  // Begin section layer

	auto path = std::make_unique<SvgPath>();
	path->setColor(section->getColor().redF(),
	               section->getColor().greenF(),
	               section->getColor().blueF());
	path->setLineWidth(static_cast<float>(section->getLineWidth()));
	path->setLineCap(SvgPath::LineCap::CAP_ROUND);
	path->setLineJoin(SvgPath::LineJoin::JOIN_ROUND);

	PolyLine* polyLine = section->getPolyLine();
	int vertexCount = polyLine->length();
	bool first = true;
	for (int i = 0; i < vertexCount; ++i) {
		Vertex* v = polyLine->getVertexRef(i);
		if (v == nullptr) continue;

		double x = (v->getX() + offsetX) * scale;
		double y = (v->getY() + offsetY) * scale;

		if (first) {
			path->moveTo(x, y);
			first = false;
		} else {
			path->lineTo(x, y);
		}
	}

	writer.addElement(std::move(path));
	writer.endGroup();  // End section layer
}

//! Write archaeological grid to SVG
void SectionManager::writeGridLayerToSVG(SvgWriter& writer, Section* section,
                                          double offsetX, double offsetY, double scale)
{
	writer.startGroup();  // Begin grid layer

	// TODO: Implement grid generation
	// This will be implemented in the next phase
	std::cout << "[SectionManager::" << __FUNCTION__ << "] Grid generation not yet implemented" << std::endl;

	writer.endGroup();  // End grid layer
}

//! Write scale bar to SVG
void SectionManager::writeScaleBarToSVG(SvgWriter& writer, Section* section,
                                         double offsetX, double offsetY, double scale)
{
	writer.startGroup();  // Begin scale bar layer

	double barLength = section->getScaleBarLength() * scale;
	double barHeight = 2.0;

	// Draw scale bar
	auto bar = std::make_unique<SvgPath>();
	bar->setColor(0.0, 0.0, 0.0);  // Black
	bar->setLineWidth(1.0);
	bar->moveTo(offsetX, offsetY);
	bar->lineTo(offsetX + barLength, offsetY);
	bar->moveTo(offsetX, offsetY - barHeight);
	bar->lineTo(offsetX, offsetY + barHeight);
	bar->moveTo(offsetX + barLength, offsetY - barHeight);
	bar->lineTo(offsetX + barLength, offsetY + barHeight);
	writer.addElement(std::move(bar));

	// Add scale bar label
	auto label = std::make_unique<SvgText>();
	label->setFont("Sans");
	label->setSize(10.0);
	label->setPosition(offsetX + barLength / 2, offsetY + 12);
	label->setText(QString("%1 %2").arg(section->getScaleBarLength()).arg(section->getRulerUnit()).toStdString());
	writer.addElement(std::move(label));

	writer.endGroup();  // End scale bar layer
}

//! Save sections to project file (JSON format)
bool SectionManager::saveToProject(const QString& filename)
{
	QJsonObject project;
	project["version"] = "1.0";

	QJsonArray sectionsArray;
	for (Section* section : mSections) {
		QJsonObject sectionObj;
		sectionObj["name"] = section->getName();
		sectionObj["type"] = static_cast<int>(section->getType());
		sectionObj["visible"] = section->isVisible();

		// Plane HNF
		QJsonArray planeArray;
		planeArray.append(section->getPlaneHNF().getX());
		planeArray.append(section->getPlaneHNF().getY());
		planeArray.append(section->getPlaneHNF().getZ());
		planeArray.append(section->getPlaneHNF().getH());
		sectionObj["planeHNF"] = planeArray;

		// Grid settings
		sectionObj["gridEnabled"] = section->isGridEnabled();
		sectionObj["gridSpacing"] = section->getGridSpacing();

		// Ruler settings
		sectionObj["rulerEnabled"] = section->isRulerEnabled();
		sectionObj["rulerUnit"] = section->getRulerUnit();

		// Scale bar settings
		sectionObj["scaleBarEnabled"] = section->isScaleBarEnabled();
		sectionObj["scaleBarLength"] = section->getScaleBarLength();

		sectionsArray.append(sectionObj);
	}
	project["sections"] = sectionsArray;

	QJsonDocument doc(project);
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly)) {
		std::cerr << "[SectionManager::" << __FUNCTION__ << "] Failed to open file for writing: "
		          << filename.toStdString() << std::endl;
		return false;
	}

	file.write(doc.toJson());
	file.close();

	std::cout << "[SectionManager::" << __FUNCTION__ << "] Saved project to: "
	          << filename.toStdString() << std::endl;
	return true;
}

//! Load sections from project file
bool SectionManager::loadFromProject(const QString& filename, Mesh* mesh)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		std::cerr << "[SectionManager::" << __FUNCTION__ << "] Failed to open file for reading: "
		          << filename.toStdString() << std::endl;
		return false;
	}

	QByteArray data = file.readAll();
	file.close();

	QJsonDocument doc = QJsonDocument::fromJson(data);
	if (doc.isNull()) {
		std::cerr << "[SectionManager::" << __FUNCTION__ << "] Failed to parse JSON" << std::endl;
		return false;
	}

	QJsonObject project = doc.object();
	QString version = project["version"].toString();

	clearSections();

	QJsonArray sectionsArray = project["sections"].toArray();
	for (const QJsonValue& value : sectionsArray) {
		QJsonObject sectionObj = value.toObject();

		QString name = sectionObj["name"].toString();
		Section::SectionType type = static_cast<Section::SectionType>(sectionObj["type"].toInt());

		QJsonArray planeArray = sectionObj["planeHNF"].toArray();
		Vector3D planeHNF(planeArray[0].toDouble(),
		                  planeArray[1].toDouble(),
		                  planeArray[2].toDouble(),
		                  planeArray[3].toDouble());

		Section* section = new Section(name, planeHNF, type);
		section->setVisible(sectionObj["visible"].toBool());
		section->setGridEnabled(sectionObj["gridEnabled"].toBool());
		section->setGridSpacing(sectionObj["gridSpacing"].toDouble());
		section->setRulerEnabled(sectionObj["rulerEnabled"].toBool());
		section->setRulerUnit(sectionObj["rulerUnit"].toString());
		section->setScaleBarEnabled(sectionObj["scaleBarEnabled"].toBool());
		section->setScaleBarLength(sectionObj["scaleBarLength"].toDouble());

		addSection(section);

		// Recalculate intersection if mesh is provided
		if (mesh != nullptr) {
			calculateIntersection(mesh, section);
		}
	}

	std::cout << "[SectionManager::" << __FUNCTION__ << "] Loaded " << mSections.size()
	          << " sections from: " << filename.toStdString() << std::endl;
	return true;
}
