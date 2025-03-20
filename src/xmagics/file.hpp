/************************************************************************************
 * Copyright (c) 2023, xeus-cpp contributors                                        *
 *                                                                                  *
 * Distributed under the terms of the BSD 3-Clause License.                         *
 *                                                                                  *
 * The full license is in the file LICENSE, distributed with this software.         *
 ************************************************************************************/

#ifndef XEUS_CPP_FILE_MAGIC_HPP
#define XEUS_CPP_FILE_MAGIC_HPP

#include "xeus-cpp/xmagics.hpp"

namespace xcpp
{
    class file_magic : public xmagic_line
    {
    public:
        void operator()(const std::string& line) override;
    };

    // Declaration of the global instance
    extern file_magic file_magic_instance;
}

#endif