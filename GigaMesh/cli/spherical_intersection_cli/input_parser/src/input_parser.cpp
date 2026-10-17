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

#include "input_parser.h"

// Abstract_Value
Input_Parser::Abstract_Value::Abstract_Value(const std::string &description,
					     bool *is_set)
    : description{description}, is_set{is_set} {}

Input_Parser::Abstract_Value::~Abstract_Value() {}

const std::string Input_Parser::Abstract_Value::get_description() const {
	return this->description;
}

// Flag
Input_Parser::Flag::Flag(bool &value, const std::string &description)
    : value{value}, description{description} {}

const std::string Input_Parser::Flag::get_description() const {
	return this->description;
}

void Input_Parser::Flag::set() const {
	this->value = true;
}

// Input_Parser
void Input_Parser::add_flag(bool &value, const std::string &key,
			    const std::string &description) {
	this->flags[key] = std::make_unique<Flag>(value, description);
}

void Input_Parser::parse(int argc, char *argv[]) {
	std::vector<std::string> arguments;
	std::copy(argv, argv + argc, std::back_inserter(arguments));

	for (auto argument_it = arguments.begin();
	     argument_it != arguments.end();) {
		if (argument_it->front() == '-') {
			auto value_it = this->values.find(*argument_it);
			auto flag_it = this->flags.find(*argument_it);
			if (flag_it != this->flags.end()) {
				flag_it->second->set();
			}
			++argument_it;
			if (argument_it != arguments.end()) {
				if (value_it != this->values.end()) {
					value_it->second->set(*argument_it);
				}
			}
			while (argument_it != arguments.end() &&
			       argument_it->front() != '-') {
				++argument_it;
			}
		} else {
			++argument_it;
		}
	}
}

std::string Input_Parser::get_help() const {
	std::stringstream ss;
	ss << "--------------------\n";
	ss << "Arguments:\n";
	for (const auto &pair : this->values) {
		ss << pair.first
		   << " [value]: " << pair.second->get_description() << "\n";
	}
	for (const auto &pair : this->flags) {
		ss << pair.first << ": " << pair.second->get_description()
		   << "\n";
	}
	ss << "--------------------";
	return ss.str();
}
