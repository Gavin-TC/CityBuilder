#include "map.h"

Tile** generate_tile_map(int rows, int cols) {
	Tile** arr = new Tile*[rows];

	for (int i = 0; i < rows; i++) {
		arr[i] = new Tile[cols];

		for (int j = 0; j < cols; j++) {
			arr[i][j] = Tile(GRASS, WHITE, -1, -1, '.');
		}
	}

	return arr;
}

Road** generate_road_map(int rows, int cols) {
	Road** arr = new Road*[rows];

	for (int i = 0; i < rows; i++) {
		arr[i] = new Road[cols];

		for (int j = 0; j < cols; j++) {
			arr[i][j] = Road(EMPTY_TILE, j, i);
		}
	}

	return arr;
}

Building** generate_building_map(int rows, int cols) {
	Building** arr = new Building*[rows];

	for (int i = 0; i < rows; i++) {
		arr[i] = new Building[cols];

		for (int j = 0; j < cols; j++) {
			arr[i][j] = Building(EMPTY, j, i);
		}
	}

	return arr;
}

void print_tile_map(Tile** map, int rows, int cols, int ox, int oy) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			int color = (int) map[i][j].color;
			char chr = map[i][j].chr;

			attron(COLOR_PAIR(color));
			mvprintw(i + oy, j + ox, "%c", chr);
			attroff(COLOR_PAIR(color));
		}
	}
}

void print_road_map(Road** map, int rows, int cols, int ox, int oy) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (map[i][j].type == EMPTY)
				continue;

			int color = (int) map[i][j].color;
			char chr = map[i][j].chr;

			attron(COLOR_PAIR(color - 1));
			mvprintw(i + oy, j + ox, "%c", chr);
			attroff(COLOR_PAIR(color - 1));
		}
	}
}

void print_building_map(Building** map, int rows, int cols, int ox, int oy) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (map[i][j].type == EMPTY_TILE)
				continue;

			int color = (int) map[i][j].color;
			char chr = map[i][j].chr;

			attron(COLOR_PAIR(color + 1));
			mvprintw(i + oy, j + ox, "%c", chr);
			attroff(COLOR_PAIR(color + 1));
		}
	}
}

void delete_tile_map(Tile** map, int rows, int cols) {
	for (int i = 0; i < rows; i++) {
		delete[] map[i];
	}

	delete[] map;
}

void delete_road_map(Road** map, int rows, int cols) {
	for (int i = 0; i < rows; i++) {
		delete[] map[i];
	}

	delete[] map;
}

void delete_building_map(Building** map, int rows, int cols) {
	for (int i = 0; i < rows; i++) {
		delete[] map[i];
	}

	delete[] map;
}
