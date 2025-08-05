
/************************************** Filesystem **********************************
*
* - Use std::filesystem for file and path operations
*
*
*   * exists(p): Checks if the file or directory at path `p` exists.
*   * file_size(p): Returns the size in bytes of the file at path `p`.
*   * create_directory(p): Creates a single directory at path `p`.
*   * create_directories(p): Creates nested directories as needed at path `p`.
*   * remove(p): Deletes the file or empty directory at path `p`.
*   * remove_all(p): Recursively deletes all files and directories under path `p`.
*   * rename(from, to): Renames or moves a file or directory.
*   * copy_file(from, to, options): Copies a file from one path to another with options.
*   * copy(from, to): Copies a file or directory (recursive for directories).
*   * is_regular_file(p): Returns true if path `p` refers to a regular file.
*   * is_directory(p): Returns true if path `p` is a directory.
*   * is_symlink(p): Returns true if path `p` is a symbolic link.
*   * is_empty(p): Checks if a file is empty or a directory is empty.
*   * directory_iterator(p): Iterates over entries in directory `p`.
*   * recursive_directory_iterator(p): Recursively iterates over directory entries.
*   * current_path(): Returns or sets the current working directory.
*   * absolute(p): Converts path `p` to an absolute path.
*   * canonical(p): Returns the absolute path after resolving symlinks.
*   * relative(p, base): Computes relative path from base to `p`.
*   * temp_directory_path(): Returns the system temporary directory path.
*   * equivalent(p1, p2): Checks if two paths refer to the same file.
*   * permissions(p): Gets or sets permissions of the path.
*   * status(p): Returns file status (type, permissions, etc.).
*   * symlink_status(p): Like status(), but does not follow symlinks.
*   * last_write_time(p): Gets or sets the last modification time.
*   * resize_file(p, size): Changes file size to specified number of bytes.
*   * create_symlink(from, to): Creates a symbolic link.
*   * create_hard_link(from, to): Creates a hard link.
*   * file_time_type, file_type, perms: Enum types for timestamps, types, and permission flags.
*
*
*--------------------------  Interview Questions and Answers -------------------------------
*
* 1. What is std::filesystem and when was it introduced in C++?
*    - It is a standard library module for file and path manipulation, introduced in C++17.
*
* 2. How do you check if a file or directory exists using std::filesystem?
*    - Use `fs::exists(path)`.
*
* 3. What is the difference between absolute() and canonical() paths?
*    - `absolute()` resolves to full path; `canonical()` also resolves symlinks.
*
* 4. How can you iterate over all files in a directory recursively?
*    - Use `fs::recursive_directory_iterator(path)`.
*
* 5. What is the purpose of directory_iterator and recursive_directory_iterator?
*    - To list files in a directory (non-recursive vs. recursive).
*
* 6. How do you copy a file and what are copy options available?
*    - Use `copy_file()` or `copy()` with `copy_options` (e.g., `overwrite_existing`).
*
* 7. How can you create nested directories with a single call?
*    - Use `create_directories()`.
*
* 8. What function would you use to get the file size?
*    - `fs::file_size(path)`.
*
* 9. How do you handle symbolic links in filesystem?
*    - Use functions like `is_symlink()`, `symlink_status()` and `create_symlink()`.
*
* 10. How do you remove a file versus removing an entire directory tree?
*     - Use `remove()` for file and `remove_all()` for directories recursively.
*
* 11. What enums does std::filesystem provide to represent file types and permissions?
*     - `file_type`, `perms`, and `copy_options`.
*
* 12. How can you check if two paths refer to the same physical file?
*     - Use `equivalent(path1, path2)`.
*
* 13. What function would you use to get or set a file’s last modification time?
*     - Use `last_write_time(path)`.
*
* 14. What is the difference between status() and symlink_status()?
*     - `status()` follows symlinks, `symlink_status()` does not.
*
* 15. How can you resize an existing file from code?
*     - Use `resize_file(path, new_size)`.
*
*
*********************************************************************************************************************/



#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

void filesystemDemo() {
	fs::path filePath = "example.txt";

	if (fs::exists(filePath)) {
		std::cout << "File exists: " << filePath << "\n";
		std::cout << "Absolute path: " << fs::absolute(filePath) << "\n";
		std::cout << "Canonical path: " << fs::canonical(filePath) << "\n";
		std::cout << "File size: " << fs::file_size(filePath) << " bytes\n";
	}

	std::cout << "Current Path: " << fs::current_path() << "\n";
	fs::create_directory("example_dir");
	fs::create_directories("example_dir/nested/structure");
	std::cout << "Created directories: example_dir/nested/structure\n";

	fs::copy_file(filePath, "example_dir/copied.txt", fs::copy_options::overwrite_existing);
	std::cout << "File copied to example_dir/copied.txt\n";

	if (fs::is_directory("example_dir")) {
		std::cout << "example_dir is a directory\n";
	}

	std::cout << "Contents of current directory:\n";
	for (const auto& entry : fs::directory_iterator(".")) {
		std::cout << " - " << entry.path() << "\n";
	}

	fs::remove("example_dir/copied.txt");
	fs::remove_all("example_dir");
	std::cout << "Cleaned up created files and directories\n";
}

int main() {
	filesystemDemo();
	return 0;
}

