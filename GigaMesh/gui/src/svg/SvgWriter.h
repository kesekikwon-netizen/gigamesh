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

#ifndef SVGWRITER_H
#define SVGWRITER_H

#include "SvgElement.h"
#include <string>
#include <list>
#include <memory>
#include <filesystem>

//!
//! \brief The SvgWriter class for writing simple SVG's
//!
//! Writer class to write Gigamesh SVG's. This is by no means a complete SVG implementation.
//!
class SvgWriter
{
	public:
		SvgWriter();
		~SvgWriter();

		SvgWriter(const SvgWriter& other) = delete;
		SvgWriter(SvgWriter&& other) = delete;

		SvgWriter& operator=(const SvgWriter& other) = delete;
		SvgWriter& operator=(SvgWriter&& other) = delete;

		//!
		//! \brief setSize sets the canvas size
		//! \param width width of the canvas
		//! \param height height of the canvas
		//!
		void setSize(double width, double height);
		//!
		//! \brief registers a new element to be added to the file. Elements are written to the file in the order they are registered
		//! \param element The SvgElement to be registered to the writer. The writer takes ownership of the element
		//!
		void addElement(std::unique_ptr<SvgElement>&& element);

		//!
		//! \brief writeToFile writes all registered elements to a file. The writer will not check if the SVG is actually valid, e.g. group tags my not be closed at the end
		//! \param filename The filename the SVG is written to
		//!
		void writeToFile(const std::filesystem::path& filename);

		//!
		//! \brief starts a new objectgroup. Call endGroup() once all Elements of the group were added
		//!
		void startGroup();

		//!
		//! \brief ends the current group started with startGroup()
		//!
		void endGroup();

		//!
		//! \brief startDefSection
		//!
		void startDefSection();

		//!
		//! \brief endDefSection
		//!
		void endDefSection();
	private:

		class DummySvgElement : public SvgElement {
			public:
			DummySvgElement(const std::string& content);

			// SvgElement interface
			virtual std::string getString() const override;

			private:
			std::string mContent;
		};


		std::list<std::unique_ptr<SvgElement> > mElements;

		double mWidth;
		double mHeight;
};

#endif // SVGWRITER_H
