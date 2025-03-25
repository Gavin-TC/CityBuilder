#ifndef MAP_H
#define MAP_H

#include "defines.h"

Tile** generate_tile_map(int rows, int cols);
Road** generate_road_map(int rows, int cols);
Building** generate_building_map(int rows, int cols);

void print_tile_map(Tile** map, int rows, int cols, int ox, int oy);
void print_road_map(Road** map, int rows, int cols, int ox, int oy);
void print_building_map(Building** map, int rows, int cols, int ox, int oy);

void delete_tile_map(Tile** map, int rows, int cols);
void delete_road_map(Road** map, int rows, int cols);
void delete_building_map(Building** map, int rows, int cols);

#endif MAP_H