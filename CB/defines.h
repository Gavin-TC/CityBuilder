#ifndef DEFINES_H
#define DEFINES_H

#include <iostream>
#include <chrono>
#include <thread>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "curses.h"

enum GameState {
	MAP_VIEW,	// Print the map + cursor
	BUILD_VIEW,	// Print the build menu
	REMOVE_VIEW,// Print the remove menu
	INFO_VIEW,  // Print info of tile under cursor
	GAME_STATE_COUNT
};


enum CharColor {
	GREEN,	// Default residential color
	BLUE,	// Default commercial color
	YELLOW, // Default industrial color
	WHITE,	// Default tile color
	GRAY,	// Default road color
	CHAR_COLOR_COUNT
};

enum TileType {
	GRASS,
	BUILDING,
	ROAD,
	EMPTY_TILE,
	TILE_TYPE_COUNT
};

class Tile {
public:
	TileType type;
	CharColor color;
	int px, py; // Position of the entity
	char chr; // Long character (with color) that reprensents the entity

	virtual ~Tile() {}
	Tile() : type(GRASS), color(WHITE), px(-1), py(-1), chr('.') {}
	Tile(TileType type, CharColor color, int px, int py, char chr) {
		this->type = type;
		this->color = color;
		this->px = px;
		this->py = py;
		this->chr = chr;
	}
};

enum BuildingType {
	RESIDENTIAL,
	COMMERCIAL,
	INDUSTRIAL,
	EMPTY,
	BUILDING_TYPE_COUNT
};

class Building : public Tile {
public:
	BuildingType type = RESIDENTIAL;

	int residents = 0;
	int max_residents = 0;

	Building() {}
	Building(BuildingType type, int px, int py, int residents, int max_residents) 
		: Tile(BUILDING, (CharColor) type, px, py, '^')
	{
		this->type = type;
		this->residents = residents;
		this->max_residents = max_residents;

		if (type == RESIDENTIAL) this->chr = '^';
		if (type == COMMERCIAL)  this->chr = '*';
		if (type == INDUSTRIAL)  this->chr = 'L';
	}

	// <summary>
	// Returns true if the tile the player is attempting to place on is
	// grass and it has atleast 1 road tiles around it's cardinal directions.
	// Otherwise, return false.
	// </summary>
	//bool get_valid_placement(Road** road_map, Building** building_map, int px, int py) {
	//	if (building_map[py + 1][px].type == BUILDING) return false; // Up
	//	if (building_map[py - 1][px].type == BUILDING) return false; // Down
	//	if (building_map[py][px - 1].type == BUILDING) return false; // Left
	//	if (building_map[py][px + 1].type == BUILDING) return false; // Right

	//	if (road_map[py + 1][px].type == ROAD) return true; // Up
	//	if (road_map[py - 1][px].type == ROAD) return true; // Down
	//	if (road_map[py][px - 1].type == ROAD) return true; // Left
	//	if (road_map[py][px + 1].type == ROAD) return true; // Right

	//	return false;
	//}
};

enum RoadDirection {
	VERT,
	HORZ,
	CROSS,
	ROAD_DIRECTION_COUNT
};

class Road : public Tile {
public:
	RoadDirection dir = CROSS;

	Road() {}
	Road(TileType type, int px, int py) : Tile(type, GRAY, px, py, '.') {}

	void update_road(Road** map, int cols, int rows, bool update_others) {
		bool up = false;
		bool down = false;
		bool left = false;
		bool right = false;

		// Updates all other roads to attempt to change their direction after
		// This road is placed.
		if (py < rows - 1 && map[py + 1][px].type == ROAD) {
			if (update_others) dynamic_cast<Road*>(&map[py + 1][px])->update_road(map, cols, rows, false);
			up = true; // Up
		}
		if (py > 0 && map[py - 1][px].type == ROAD) {
			if (update_others) dynamic_cast<Road*>(&map[py - 1][px])->update_road(map, cols, rows, false);
			down = true; // Down
		}
		if (px > 0 && map[py][px - 1].type == ROAD) {
			if (update_others) dynamic_cast<Road*>(&map[py][px - 1])->update_road(map, cols, rows, false);
			left = true; // Left
		}
		if (px < cols - 1 && map[py][px + 1].type == ROAD) {
			if (update_others) dynamic_cast<Road*>(&map[py][px + 1])->update_road(map, cols, rows, false);
			right = true; // Right
		}

		if (up || down && !(left || right))			dir = VERT;
		else if (left || right && !(up || down))	dir = HORZ;
		else										dir = CROSS;

		// Change the char
		switch (dir) {
			case VERT:
				this->chr = '|';
				break;

			case HORZ:
				this->chr = '=';
				break;
			
			case CROSS:
				this->chr = '+';
				break;
		}
	}

	bool get_valid_placement(Road** road_map, Building** building_map, int px, int py) {
		return false;
	}
};

struct City {
	Road** road_map;
	Building** building_map;

	int total_residents = 0;

	int total_buildings = 0;
	int total_residential_buildings = 0;
	int total_commercial_buildings = 0;
	int total_industrial_buildings = 0;

	float happiness = 0.0; // average resident happiness; 0.0 -> 100.0

	City(Road** road_map, Building** building_map) : road_map(road_map), building_map(building_map) {}

	// Maybe for loading save?
	City(Road** rm, Building** bm, int tr, int tb, int trb, int tcb, int tib, float h) {
		road_map = rm;
		building_map = bm;
		
		total_residents = tr;

		total_buildings = tb;
		total_residential_buildings = trb;
		total_commercial_buildings = tcb;
		total_industrial_buildings = tib;

		happiness = h;
	}
};

#endif DEFINES_H
