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

#ifndef SECTIONMANAGER_H
#define SECTIONMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include "section.h"

class Mesh;

//!
//! \brief Manager for archaeological sections (cross-sections)
//!
//! Manages multiple Section objects for a mesh, handles section creation,
//! deletion, and SVG export with layers.
//!
class SectionManager : public QObject
{
	Q_OBJECT

public:
	explicit SectionManager(QObject* parent = nullptr);
	~SectionManager();

	// Section management
	int getSectionCount() const { return mSections.size(); }
	Section* getSection(int index) const;
	Section* getActiveSection() const { return mActiveSection; }
	int getActiveSectionIndex() const;

	void addSection(Section* section);
	void removeSection(int index);
	void clearSections();

	void setActiveSection(int index);
	void setActiveSection(Section* section);

	// Section creation helpers
	Section* createSection(const QString& name, const Vector3D& planeHNF);
	Section* createPresetSection(Section::SectionType type, const Vector3D& centerPoint);

	// Intersection calculation
	bool calculateIntersection(Mesh* mesh, Section* section);
	void calculateAllIntersections(Mesh* mesh);

	// Export functions
	bool exportSectionToSVG(Section* section, const QString& filename);
	bool exportAllSectionsToSVG(const QString& filename, bool separateFiles = false);
	bool exportSelectedSectionsToSVG(const QList<int>& indices, const QString& filename, bool separateFiles = false);

	// Project save/load (JSON format)
	bool saveToProject(const QString& filename);
	bool loadFromProject(const QString& filename, Mesh* mesh);

signals:
	void sectionAdded(Section* section);
	void sectionRemoved(int index);
	void activeSectionChanged(Section* section);
	void sectionsCleared();

private:
	QList<Section*> mSections;        //!< List of all sections
	Section* mActiveSection;          //!< Currently active section (for editing)

	// Helper functions for SVG export
	void writeSectionLayerToSVG(class SvgWriter& writer, Section* section, double offsetX, double offsetY, double scale);
	void writeGridLayerToSVG(class SvgWriter& writer, Section* section, double offsetX, double offsetY, double scale);
	void writeScaleBarToSVG(class SvgWriter& writer, Section* section, double offsetX, double offsetY, double scale);
};

#endif // SECTIONMANAGER_H
