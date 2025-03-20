#ifndef FILE_MAGIC_PLUGIN_HPP
#define FILE_MAGIC_PLUGIN_HPP

#include "xeus-cpp/xmagic_plugin.hpp"
#include "xeus-cpp/xmagics.hpp"

namespace xcpp_plugins
{
    class file_magic_line : public xcpp::xmagic_line
    {
    public:
        void operator()(const std::string& line) override;
    };

    class file_magic_plugin : public xcpp::xmagic_plugin
    {
    private:
        file_magic_line m_line_magic;

    public:
        std::string name() const override { return "file"; }
        
        bool supports_line_magic() const override { return true; }
        bool supports_cell_magic() const override { return false; }
        
        xcpp::xmagic_line* as_line_magic() override { return &m_line_magic; }
        xcpp::xmagic_cell* as_cell_magic() override { return nullptr; }
    };
}

#endif