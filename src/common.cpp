#include "common.h"
#include <fstream>
#include <sstream>

optional<string> LoadTextFile(const string &filename)
{
    ifstream fin(filename);
    if (!fin.is_open())
    {
        SPDLOG_ERROR("failed to openfile{}", filename);
        return {};
    }
    stringstream text;
    text << fin.rdbuf();
    return text.str();
}