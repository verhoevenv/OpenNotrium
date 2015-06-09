#include "virtualfs.h"
#include "physfs.h"
#include "physfsrwops.h"

#include <vector>
#include <string>
#include <sstream>

VirtualFS::VirtualFS(const char *argv0){
    PHYSFS_init(argv0);
    PHYSFS_setSaneConfig("monkkonen","notrium",nullptr,0,0); //Perhaps we should allow packages here. Not now.
}

bool VirtualFS::exists(const std::string& filename){
	return (PHYSFS_exists(filename.c_str()) != 0);
}

bool VirtualFS::isDirectory(const std::string& filename){
	return (PHYSFS_isDirectory(filename.c_str()) != 0);
}

std::vector<std::string> VirtualFS::listDirectory(const std::string& dir){
	char **rc = PHYSFS_enumerateFiles(dir.c_str());
	char **i;

	std::vector<std::string> vec;

	for (i = rc; *i != NULL; i++){
		vec.push_back(*i);
	}

	PHYSFS_freeList(rc);

	return vec;
}

std::vector<std::string> VirtualFS::readLines(const std::string& filename){
    std::vector<std::string> vec;

    PHYSFS_file *f = PHYSFS_openRead(filename.c_str());
    if(f == nullptr) {
        return vec;
    }
    char *myBuf = new char[PHYSFS_fileLength(f)];
    PHYSFS_read (f, myBuf, 1, PHYSFS_fileLength(f));

    std::stringstream ss(myBuf);
    std::string line;

    while(std::getline(ss,line,'\n')){
        vec.push_back(line);
    }

    PHYSFS_close(f);
    delete myBuf;

    return vec;
}

SDL_RWops* VirtualFS::openRWops_write(const std::string& filename){
	return PHYSFSRWOPS_openWrite(filename.c_str());
}

SDL_RWops* VirtualFS::openRWops_read(const std::string& filename){
	return PHYSFSRWOPS_openRead(filename.c_str());
}
