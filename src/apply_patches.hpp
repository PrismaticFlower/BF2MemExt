#pragma once

[[nodiscard]] bool apply(const char* file_path, int (*print)(const char* format, ...)) noexcept;
