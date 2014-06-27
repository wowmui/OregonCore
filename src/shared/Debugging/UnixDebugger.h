/*
 * Copyright (C) 2014 OregonCore <http://www.oregoncore.com/>
 * Copyright (C) 2014 Zaffy <zaffy@lunar.sk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#if PLATFORM == PLATFORM_UNIX
#ifndef _UNIX_DEBUGGER_H_
#define _UNIX_DEBUGGER_H_

#include "Common.h"

// This is fix for libiberty because ACE has already declared it.
#define HAVE_DECL_BASENAME 1

#include <libiberty/libiberty.h>
#include <libiberty/demangle.h>
#include <bfd.h>

namespace UnixDebugger
{
    void RegisterDeadlySignalHandler();
    void DumpDebugInfo(const char* sig, const char* reason);

    class Resolver
    {
        public:

            Resolver(const char* executable);
            ~Resolver();

            bool Resolve(unsigned long addr);

            const std::string& GetFunction() const { return function; }
            const std::string& GetFile() const { return filename; }
            unsigned int GetLine() const { return line; }
        private:
            bfd* abfd = 0;
            asymbol **syms = 0;
            asection *text = 0;

            std::string function;
            std::string filename;
            unsigned int line;

            static bool initialized;
    };
};

#endif // _UNIX_DEBUGGER_H_
#endif // PLATFORM_UNIX
