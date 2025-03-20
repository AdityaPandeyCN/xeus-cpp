/************************************************************************************
 * Copyright (c) 2023, xeus-cpp contributors                                        *
 *                                                                                  *
 * Distributed under the terms of the BSD 3-Clause License.                         *
 *                                                                                  *
 * The full license is in the file LICENSE, distributed with this software.         *
 ************************************************************************************/

#ifndef XCPP_MAGIC_FACTORY_HPP
#define XCPP_MAGIC_FACTORY_HPP

#include "xmagic_plugin.hpp"
#include <xplugin/xfactory.hpp>

namespace xcpp
{
    // Define the factory base type for magic plugins
    using xmagic_factory_base = xp::xfactory_base<xmagic_plugin>;
}

#endif