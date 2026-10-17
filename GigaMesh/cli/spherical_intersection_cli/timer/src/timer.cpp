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

#include "timer.h"

using namespace timer;

std::unordered_map<std::string,
		   std::chrono::time_point<std::chrono::system_clock>>
    Timer::start_times;

std::unordered_map<std::string, double> Timer::run_times;

void Timer::start(const std::string &action) {
		Timer::start_times[action] = std::chrono::system_clock::now();
	}

void Timer::stop(const std::string &action) {
    auto now = std::chrono::system_clock::now();
    auto start_time_it = Timer::start_times.find(action);
    if (start_time_it != Timer::start_times.end()) {
        std::chrono::duration<double> elapsed_seconds =
        now - start_time_it->second;
        Timer::run_times[action] = elapsed_seconds.count();
    }
}

double Timer::get(const std::string &action) {
    return Timer::run_times[action];
}