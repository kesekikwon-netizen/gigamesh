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

#include <GigaMesh/logging/Logging.h>

#include <iostream>

namespace LOG {
	Logger logFatal (true );
	Logger logError (true );
	Logger logWarn  (false);
	Logger logInfo  (false);
	Logger logDebug (false);

	void initLogging()
	{
		logFatal.setOutputStream (&std::cerr);
		logError.setOutputStream (&std::cerr);
		logWarn. setOutputStream (&std::cerr);
		logInfo. setOutputStream (&std::cout);
		logDebug.setOutputStream (&std::cerr);
	}

	Logger* getLogger(LogLevel level)
	{
		Logger* logger = nullptr;
		switch (level) {
			case LogLevel::eFatal:
				logger = &logFatal;
				break;
			case LogLevel::eError:
				logger = &logError;
				break;
			case LogLevel::eWarn:
				logger = &logWarn;
				break;
			case LogLevel::eInfo:
				logger = &logInfo;
				break;
			case LogLevel::eDebug:
				logger = &logDebug;
				break;
		}

		return logger;
	}

	void setLoggerEnabled(bool enable, LogLevel level)
	{
		if(auto logger = getLogger(level); logger != nullptr)
			logger->setIsEnabled(enable);
	}

	void setLogStream(std::ostream* streamObject, LogLevel level)
	{
		if(auto logger = getLogger(level); logger != nullptr)
			logger->setOutputStream(streamObject);
	}

	void setLogLevel(LogLevel level)
	{
		auto l = static_cast<unsigned char>(level);
		for(auto i = static_cast<unsigned char>(LogLevel::eFatal); i <= static_cast<unsigned char>(LogLevel::eDebug); ++i)
		{
			setLoggerEnabled(l >= i, static_cast<LogLevel>(i));
		}
	}

	Logger& fatal()
	{
		return logFatal << "[LOG FATAL  ] ";
	}

	Logger& error()
	{
		return logError << "[LOG ERROR  ] ";
	}

	Logger& warn()
	{
		return logWarn << "[LOG WARNING] ";
	}

	Logger& info()
	{
		return logInfo << "[LOG INFO   ] ";
	}

	Logger& debug()
	{
		return logDebug << "[LOG DEBUG  ] ";
	}
}
