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
	MAIN_ROAD, // Forced road that all roads must connect to to grow the city.
	TILE_TYPE_COUNT
};

class Tile {
public:
	TileType type;
	CharColor color;
	int px, py; // Position of the entity
	char chr; // Long character (with color) that reprensents the entity

	virtual ~Tile() {}
	Tile() : type(GRASS), color(GREEN), px(-1), py(-1), chr('.') {}
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

	// Residents can mean workers in the context 
	// of a commercial/industrial building
	int residents = 0;
	int max_residents = 0;

	int provided_jobs = 0;

	int price = 0.0;
	static const int residential_price = 20000;
	static const int commercial_price = 45000;
	static const int industrial_price = 60000;
	static const int removal_price = 10000;

	Building() {}
	Building(BuildingType type, int px, int py) 
		: Tile(BUILDING, (CharColor) (((int)type)), px, py, '^')
	{
		this->type = type;

		if (type == RESIDENTIAL) {
			this->chr = '^';
			this->max_residents = 20;
			this->price = residential_price;
		}
		if (type == COMMERCIAL) {
			this->chr = 'o';
			this->provided_jobs = 10;
			this->price = commercial_price;
		}
		if (type == INDUSTRIAL) {
			this->chr = 'L';
			this->provided_jobs = 25;
			this->price = industrial_price;
		}
	}
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

	static const int price = 1000;

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

		// This probably doesn't need so much logic, but it works
		if (up || down && !(left || right)) dir = VERT;
		if (left || right && !(up || down))	dir = HORZ;

		if (left && up && !(right || down)) dir = CROSS;
		if (right && up && !(left || down)) dir = CROSS;
		if (left && down && !(right || up)) dir = CROSS;
		if (right && down && !(left || up)) dir = CROSS;
		if (right && left && (down || up))  dir = CROSS;
		if ((left && up && down) || (right && up && down)) dir = CROSS;

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

	int total_funds = 500000;

	int total_residents = 0;

	int total_buildings = 0;
	int total_residential_buildings = 0;
	int total_commercial_buildings = 0;
	int total_industrial_buildings = 0;
	int total_roads = 0;

	float happiness = 0.0; // average resident happiness; 0.0 -> 100.0

	City(Road** road_map, Building** building_map, int total_funds) 
		: road_map(road_map), building_map(building_map), total_funds(total_funds) {}

	// Maybe for loading save?
	City(Road** rm, Building** bm, int tf, int tr, int tb, int trb, int tcb, int tib, int tro, float h) {
		road_map = rm;
		building_map = bm;

		total_funds = tf;
		
		total_residents = tr;

		total_buildings = tb;
		total_residential_buildings = trb;
		total_commercial_buildings = tcb;
		total_industrial_buildings = tib;
		total_roads = tro;

		happiness = h;
	}

	int get_total_buildings() {
		return total_residential_buildings + total_commercial_buildings + total_industrial_buildings + total_roads;
	}
};

#endif DEFINES_H
