/*
Allocore Example: Directory

Description:
This shows how to obtain the contents of a directory.

Author:
Lance Putnam, May 2019
*/
#include <stdio.h>
#include <functional>
#include "allocore/io/al_File.hpp"

int main(){

	al::Dir dir("../../");

	// Iterate over all files in the directory
	printf("\nFiles:\n------\n");
	while(dir.read()){
		const auto& entry = dir.entry();
		if(entry.type() == entry.REG){
			printf("%s\n", entry.name().c_str());
		}
	}

	// Rewind to first entry in directory
	dir.rewind();

	// Iterate over all subdirectories in the directory
	printf("\nDirectories:\n------------\n");
	while(dir.read()){
		const auto& entry = dir.entry();
		if(entry.type() == entry.DIR){
			printf("%s\n", entry.name().c_str());
		}
	}

	// Recurse subdirectories
	std::function<void(const std::string&)> onEnterDir = [&](const std::string& path){
		al::Dir dir(path);
		while(dir.read()){
			const auto& entry = dir.entry();
			if(entry.type() == entry.DIR){
				auto subdir = path + entry.name() + "/";
				printf("%s\n", subdir.c_str());
				onEnterDir(subdir);
			}
		}
	};

	onEnterDir("../../");
}
