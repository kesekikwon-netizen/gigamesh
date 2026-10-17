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

#ifndef LOGGER_H
#define LOGGER_H

#include <string_view>
#include <ostream>


// Logger class wrapping an ostream object. Do not use directly, use the wrapping functions in Logging.h
// Also, this logging is not thread safe! This means, that logging with multiple threads could have overlapping outputs

class Logger
{
	public:
		Logger(bool isEnabled = true);
		~Logger();

		[[nodiscard]] bool isEnabled() const;
		void setIsEnabled(bool isEnabled);

		template<typename T>
		auto operator<<(const T& msg) -> Logger&
		{
			if(mIsEnabled && mOutputStream)
				(*mOutputStream) << msg;
			return *this;
		}

		void setOutputStream(std::ostream* outputStream);
	private:
		std::ostream* mOutputStream = nullptr;
		bool mIsEnabled = true;
};

#endif // LOGGER_H
