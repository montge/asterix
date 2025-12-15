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
 *
 *
 * AUTHORS: Damir Salantic, Croatia Control Ltd.
 *
 */

#include "Tracer.h"
#include <stdio.h>
#include <iostream>
#include <stdarg.h>
#include <string.h>

namespace {
    constexpr size_t kErrorBufferSize = 1024;
}

using namespace std;

Tracer *Tracer::g_TracerInstance = nullptr;

Tracer &Tracer::instance() {
    if (!Tracer::g_TracerInstance) {
        Tracer::g_TracerInstance = new Tracer();
    }
    return *Tracer::g_TracerInstance;
}

Tracer::Tracer()
        : pPrintFunc(nullptr), pPrintFunc2(nullptr), m_logLevel(1) {
    // Default log level is 1 (errors only)
}

void Tracer::Configure(ptExtPrintf pFunc) {
    Tracer::instance().pPrintFunc = pFunc;
}

void Tracer::Configure(ptExtVoidPrintf pFunc) {
    Tracer::instance().pPrintFunc2 = pFunc;
}

void Tracer::Delete() {
    if (Tracer::g_TracerInstance)
        delete Tracer::g_TracerInstance;
    Tracer::g_TracerInstance = nullptr;
}

void Tracer::Error(const char *format, ...) {
    Tracer &instance = Tracer::instance();

    // Check log level - if silent (0), don't output anything
    if (instance.m_logLevel <= 0) {
        return;
    }

    char buffer[kErrorBufferSize];
    va_list args;
    va_start (args, format);
    vsnprintf(buffer, kErrorBufferSize, format, args);
    va_end (args);

    if (instance.pPrintFunc) {
        instance.pPrintFunc(buffer);
    } else if (instance.pPrintFunc2) {
        instance.pPrintFunc2(buffer);
    } else {
        puts(buffer);
    }
}

void Tracer::SetLogLevel(int level) {
    Tracer &instance = Tracer::instance();
    instance.m_logLevel = level;
}

int Tracer::GetLogLevel() {
    Tracer &instance = Tracer::instance();
    return instance.m_logLevel;
}
