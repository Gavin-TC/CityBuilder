#ifndef DEFINES_H
#define DEFINES_H

#include <iostream>
#include <chrono>
#include <thread>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "curses.h"

enum Month {
	JANUARY,
	FEBUARY,
	MARCH,
	APRIL,
	MAY,
	JUNE,
	JULY,
	AUGUST,
	SEPTEMBER,
	OCTOBER,
	NOVEMBER,
	DECEMBER,
	MONTH_COUNT
};

enum GameState {
	MAP_VIEW,	// Print the map + cursor
	BUILD_VIEW,	// Print the build menu
	REMOVE_VIEW,// Print the remove menu
	INFO_VIEW,  // Print info of tile under cursor
	PAUSE_VIEW, // Prints pause menu and pauses game
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
	PARK, // Just adds happiness
	BUILDING_TYPE_COUNT
};

class Building : public Tile {
public:
	BuildingType type = RESIDENTIAL;

	int residents = 0;
	static const int max_residents = 20; // For a RESIDENTIAL building

	static const int residential_price = 20000;
	static const int commercial_price = 45000;
	static const int industrial_price = 60000;
	static const int park_price = 10000;
	static const int removal_price = 10000;

	// Income on a monthly basis
	static const int income_per_commercial_worker = 300; // $6,000/building max
	static const int income_per_industrial_worker = 300; // $10,500/building max

	int workers = 0;

	static const int max_jobs_commercial = 20;
	static const int max_jobs_industrial = 35;

	static const int happiness_per_commercial_building = 5;

	Building() {}
	Building(BuildingType type, int px, int py) 
		: Tile(BUILDING, (CharColor) (((int)type)), px, py, '^')
	{
		this->type = type;

		if (type == RESIDENTIAL) {
			this->chr = '^';
		}
		if (type == COMMERCIAL) {
			this->chr = 'o';
		}
		if (type == INDUSTRIAL) {
			this->chr = 'L';
		}
		if (type == PARK) {
			this->chr = '@';
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
	int cols, rows;

	int total_funds = 500000;

	int total_residents = 0;
	int max_residents = 0;

	int total_buildings = 0;
	int total_residential_buildings = 0;
	int total_commercial_buildings = 0;
	int total_industrial_buildings = 0;
	int total_roads = 0;

	float happiness = 0.0; // average resident happiness; 0.0 -> 100.0

	City(Road** road_map, Building** building_map, int total_funds, int cols, int rows) 
		: road_map(road_map), building_map(building_map), total_funds(total_funds), cols(cols), rows(rows) {}

	// Maybe for loading save?
	City(Road** rm, Building** bm, int tf, int tr, int tb, int trb, int tcb, int tib, int tro, float h, int c, int r) {
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

		cols = c;
		rows = r;
	}

	int get_total_buildings() {
		return total_residential_buildings + total_commercial_buildings + total_industrial_buildings + total_roads;
	}

	int get_current_residents() {
		int total = 0;

		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				if (building_map[i][j].type == RESIDENTIAL) {
					total += building_map[i][j].residents;
				}
			}
		}
		
		return total;
	}

	int get_max_residents() {
		return total_residential_buildings * Building::max_residents;
	}

	// Loop through every commercial/industrial building and do the mather
	int get_commercial_monthly_funds() {
		int total = 0;

		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				if (building_map[i][j].type == COMMERCIAL) {
					total += building_map[i][j].workers * Building::income_per_commercial_worker;
				}
			}
		}

		return total;
	}

	int get_industrial_monthly_funds() {
		int total = 0;

		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				if (building_map[i][j].type == INDUSTRIAL) {
					total += building_map[i][j].workers * Building::income_per_industrial_worker;
				}
			}
		}

		return total;
	}

	int get_total_monthly_funds() {
		return get_commercial_monthly_funds() + get_industrial_monthly_funds();
	}
};

#endif DEFINES_Hindustrial_price
