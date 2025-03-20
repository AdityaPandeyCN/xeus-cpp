/************************************************************************************
 * Copyright (c) 2023, xeus-cpp contributors                                        *
 *                                                                                  *
 * Distributed under the terms of the BSD 3-Clause License.                         *
 *                                                                                  *
 * The full license is in the file LICENSE, distributed with this software.         *
 ************************************************************************************/

#ifndef XCPP_MAGIC_PLUGIN_HPP
#define XCPP_MAGIC_PLUGIN_HPP

#include "xmagics.hpp"
#include <memory>
#include <string>

namespace xcpp
{
    // Base class for magic plugins
    class xmagic_plugin
    {
    public:
        virtual ~xmagic_plugin() = default;
        
        // Get the command name (what follows % or %%)
        virtual std::string name() const = 0;
        
        // Check if this plugin supports line magic
        virtual bool supports_line_magic() const = 0;
        
        // Check if this plugin supports cell magic
        virtual bool supports_cell_magic() const = 0;
        
        // Cast to line magic if supported
        virtual xmagic_line* as_line_magic() = 0;
        
        // Cast to cell magic if supported
        virtual xmagic_cell* as_cell_magic() = 0;
    };
}

#endif