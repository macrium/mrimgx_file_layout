#pragma once

// Validates the command-line arguments.
std::tuple<std::wstring, std::wstring, std::wstring, int, bool, bool, bool> handleCommandLineParameters(int argc, wchar_t* argv[]);

// Converts a wide string to a UTF-8 string.
std::string convertToUtf8(const std::wstring& wstr);
