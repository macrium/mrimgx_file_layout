#pragma once

#include "stdint.h"
#include <io.h>
#include <array> 
#include <map>
#include <memory>   
#include <stdexcept>
#include <functional>
#include <vector>
#include "encryption.h"

#include "openssl\ssl.h"
#include "openssl\crypto.h"
#include "openssl\err.h"
#include "openssl\evp.h"

#include <sstream> // for std::wstringstream
#include <iomanip> // for std::setw and std::setfill

