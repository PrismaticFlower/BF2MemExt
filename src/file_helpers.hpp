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

/// @brief Copy a file to next to another file.
/// @param src_file The file to copy.
/// @param dest_file_name The target file to place the copied file next to.
/// @return If copying the file succeeded or not.
[[nodiscard]] bool copy_next_to(const char* src_file, const char* dest_file_name);

/// @brief Checks if a file exists.
/// @param file The file.
/// @return If the file exists or not.
[[nodiscard]] bool file_exists(const char* file);

/// @brief Call AttachConsole and initialize the CRT's stdio.
void init_cstdio();