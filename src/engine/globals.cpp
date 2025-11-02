/*
 *  Copyright (c) 2013 Croatia Control Ltd. (www.crocontrol.hr)
 *
 *  This file is part of Asterix.
 *
 *  Asterix is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Asterix is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Asterix.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file globals.cpp
 * @brief Global variables for the ASTERIX library
 *
 * This file defines global variables that are used by both the library
 * and the executable. Previously these were only in src/main/asterix.cpp,
 * which caused undefined symbol errors when building the shared library on
 * macOS (due to stricter linker requirements).
 *
 * These globals are externally declared in src/main/asterix.h
 */

// Global verbose flag - controls debug output
bool gVerbose = false;

// Global trace flag - controls detailed tracing
bool gTrace = false;

// Force routing flag - override normal routing behavior
bool gForceRouting = false;

// Heartbeat interval in seconds (0 = disabled)
int gHeartbeat = 0;

// Path to ASTERIX definitions file
const char* gAsterixDefinitionsFile = nullptr;
