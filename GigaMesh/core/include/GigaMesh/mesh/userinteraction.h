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

#ifndef USERINTERACTION_H
#define USERINTERACTION_H

#include<string>
#include<set>
#include<vector>
#include<cstdint>

class Matrix4D;

class UserInteraction
{
	public:
		UserInteraction();

	protected:
		virtual void showInformation( const std::string& rHead, const std::string& rMsg,
		                              const std::string& rToClipboard="" );
		virtual void showWarning( const std::string& rHead, const std::string& rMsg );

		virtual bool showEnterText( std::string&         rSomeStrg,  const char* rTitle );
		virtual bool showEnterText( uint64_t&            rULongInt,  const char* rTitle );
		virtual bool showEnterText( double&              rDoubleVal, const char* rTitle );
		virtual bool showEnterText( std::set<int64_t>&   rIntegers,  const char* rTitle );
		virtual bool showEnterText( std::vector<long>&   rIntegers,  const char* rTitle );
		virtual bool showEnterText( std::vector<double>& rDoubles,   const char* rTitle );
		virtual bool showEnterText( Matrix4D* rMatrix4x4, bool selectedVerticesOnly = false );

		virtual bool showSlider( double* rValueToChange, double rMin, double rMax, const char* rTitle );
		virtual bool showQuestion( bool* rUserChoice, const std::string& rHead, const std::string& rMsg );
};

#endif // USERINTERACTION_H
