// framedriver.cpp
//
// This is a development driver for the Envy Engine frame queue to test the queue's performance and
// functionality without needing networked render nodes.

#define FRAME_DIR L"..\\3DEngineCpp\\res\\testframes"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include "framedriver.h"
#include "windows.h" // For windows file I/O mainly

FrameDriver::FrameDriver(MasterController *mc) {
	this->mc = mc; // MasterController to feed frames to



}

// For sorting wstrings
bool _wstringptr_sort(std::wstring *i, std::wstring *j) {
	return (*i < *j); // This is strcmp for dummies
}

void FrameDriver::loadFrames() {
	
	HANDLE hFind;
	WIN32_FIND_DATA findData;
	SDL_Surface *curSurf;
	TCHAR imgPath[MAX_PATH];
	char aImgPath[MAX_PATH];
	TCHAR pathBuf[MAX_PATH];
	std::vector<std::wstring*> dirList;
	std::wstring str;
	size_t pos;

	// Path of executable
	GetModuleFileNameW(NULL, pathBuf, MAX_PATH);
	
	// Snip executable name.
	str = pathBuf;
	pos = str.find_last_of(L"\\", std::wstring::npos);
	if (pos < MAX_PATH - 1) {
		pathBuf[pos + 1] = 0;
	}
	lstrcatW(pathBuf, FRAME_DIR);
	mFrameDir = pathBuf; // Set frame directory

	lstrcpy(imgPath, pathBuf);
	lstrcat(imgPath, L"\\*.png"); // Search for PNG graphics

	// Load all test frames out of frame directory
	dirList.clear();

	// Anything that has a .png extension. Sort in alpha order.
	hFind = FindFirstFileW(imgPath, &findData);
	dirList.push_back(new std::wstring(findData.cFileName));

	while (FindNextFileW(hFind, &findData))
		dirList.push_back(new std::wstring(findData.cFileName));

	std::sort(dirList.begin(), dirList.end(), _wstringptr_sort);

	// Load textures
	fprintf(stderr, "FrameDriver loading %d surfaces...\n", dirList.size());
	mSurfaceHolder.clear();
	for (std::vector<std::wstring*>::iterator it = dirList.begin(); it != dirList.end(); it++) {

		lstrcpy(imgPath, pathBuf);
		lstrcat(imgPath, L"\\");
		lstrcat(imgPath, (*it)->c_str());

		wcstombs(aImgPath, imgPath, MAX_PATH);

		curSurf = IMG_Load(aImgPath);

		if (!curSurf) {
			fprintf(stderr, "Error loading surface: %s\n", SDL_GetError());
		}
		else {
			mSurfaceHolder.push_back(curSurf);
		}
	}
}

void FrameDriver::_tick() {

	// Note: Currently unused
	size_t bufSize = 4 * 1366 * 720;
	GLubyte *frameBufData = (GLubyte*)malloc(bufSize);
	memset(frameBufData, 0, bufSize);


	// Create a surface
	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(frameBufData, 1366, 720, 32, 1366*4, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	if (!surface) {
	fprintf(stderr, "Error creating surface: %s\n", SDL_GetError());
	}

	if (frameBufData && surface) {
	glReadPixels(0, 0, 1366, 720, GL_RGBA, GL_UNSIGNED_BYTE, frameBufData);
	}

	if (SDL_SaveBMP(surface, "D:\\test.bmp")) {
	fprintf(stderr, "Error saving BMP: %s\n", SDL_GetError());
	}
	free(frameBufData);
}

void FrameDriver::tick() {
	// Check time since last tick
	// Over target rate?
		// Add frame to master controller's queue.
}

FrameDriver::~FrameDriver(){}
