#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <memory>
#ifdef _WIN32
#  include <objbase.h>   // GUID, CoCreateGuid
#  include <winioctl.h>  // disk IOCTLs (Windows-only)
#endif
#include <chrono>
#include <sstream>
#include <nlohmann/json.hpp>
#include <random>