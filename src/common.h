#ifndef __COMMON_H__
#define __COMMON_H__

#include <memory>
#include <string>
#include <optional>
#include <glad/glad.h>
#include <spdlog/spdlog.h>

using namespace std;

// typedef
#define CLASS_PTR(klassName)                       \
    class klassName;                               \
    using klassName##UPtr = unique_ptr<klassName>; \
    using klassName##Ptr = shared_ptr<klassName>;  \
    using klassName##WPtr = weak_ptr<klassName>;

optional<string> LoadTextFile(const string &filename);

#endif