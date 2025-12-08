#pragma once

#include "Module.h"
#include <string>
#include <vector>

using namespace std;


class Texture : public Module
{
public:

	Texture();

	
	virtual ~Texture();

	bool Awake();

	bool Start();

	bool CleanUp();

	uint TextureFromFile(std::string directory, const char* filename);

	


public:
	
	uint id;
	std::string mapType;
	std::string path;
	int texW, texH;

	std::vector<Texture> textures_loaded;

	std::string defaultTexDir = "../Assets/Textures/checkers.jpg";
	
	
};
