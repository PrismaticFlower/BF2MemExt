#pragma once

/// @brief Aquire a temporary file using a base file path.
/// @param base_file_path The base file path.
/// @return The temporary file path. Must be passed to free if not null.
[[nodiscard]] char* aquire_temp_file(const char* base_file_path, const char* prefix);

/// @brief Move a file, replacing any already existing file at the destination.
/// @param from The file to move.
/// @param to The path to move the file to.
/// @return If moving the file succeeded or not.
[[nodiscard]] bool move_file(const char* from, const char* to);