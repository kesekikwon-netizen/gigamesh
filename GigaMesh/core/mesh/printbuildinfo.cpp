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

#include <GigaMesh/printbuildinfo.h>

#include <iostream>

//! Show build information.
void printBuildInfo() {
	std::cout << "[GigaMesh] ----------------------------------------------------------------------------------" << std::endl;
#ifdef VERSION_PACKAGE
	std::cout << "[GigaMesh] Package version: " << VERSION_PACKAGE << std::endl;
#else
	std::cerr << "[GigaMesh] ERROR: Package version missing!" << std::endl;
#endif
	std::cout << "[GigaMesh] .................................................................................." << std::endl;
#ifdef COMP_USER
	std::cout << "[GigaMesh] Built by: " << COMP_USER << std::endl;
#else
	std::cerr << "[GigaMesh] ERROR: Build user information missing!" << std::endl;
#endif
#ifdef COMP_DATE
	std::cout << "[GigaMesh] ..... on: " << COMP_DATE << std::endl;
#else
	std::cerr << "[GigaMesh] ERROR: Build date information missing!" << std::endl;
#endif
#ifdef COMP_EDIT
	std::cout << "[GigaMesh] .... for: " << COMP_EDIT << std::endl;
#else
	std::cerr << "[GigaMesh] ERROR: Build edition information missing!" << std::endl;
#endif
#ifdef COMP_GITHEAD
	std::cout << "[GigaMesh] Git SHA1: " << COMP_GITHEAD << std::endl;
#else
	std::cerr << "[GigaMesh] Build  not based on git commit!" << std::endl;
#endif
	std::cout << "[GigaMesh] ----------------------------------------------------------------------------------" << std::endl;
}
