#ifndef VIRTUAL_FS_H
#define VIRTUAL_FS_H

#include <string>
#include <vector>
#include <SDL.h>

class VirtualFS
{
public:
	VirtualFS(const char *argv0);

	bool exists(const std::string& filename);
	bool isDirectory(const std::string& filename);
	std::vector<std::string> listDirectory(const std::string& dir);
	std::string readAll(const std::string& filename) const;
	std::vector<std::string> readLines(const std::string& filename) const;

	SDL_RWops* openRWops_write(const std::string& filename);
	SDL_RWops* openRWops_read(const std::string& filename);
};

#endif
