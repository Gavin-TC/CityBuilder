#include "main.h"

#define ROWS 20
#define COLS 40

#define OFFSET_X 0
#define OFFSET_Y 0

#define FPS 60

void print_city_info(City city, int ox, int oy);
void print_controls(int ox, int oy);
void print_cursor(int cx, int cy, int ox, int oy);
void handle_controls(City &city, Road** &road_map, Building** &building_map, int &cx, int &cy);
const char* building_type_to_str(BuildingType bt);
WINDOW* init();

GameState game_state = MAP_VIEW;
TileType tile_type = ROAD;
BuildingType building_type = RESIDENTIAL;
Tile* cur_tile = nullptr;
bool cursor_moving = false;

int main(void) {
	WINDOW* win = init();
	Tile** tile_map = generate_tile_map(ROWS, COLS);
	Road** road_map = generate_road_map(ROWS, COLS);
	Building** building_map = generate_building_map(ROWS, COLS);
	City city = City(road_map, building_map, 500000);

	bool game_running = true;

	int cx = 1;
	int cy = 1;

	int ticks = 0;
	int on_ticks = 7; // How many ticks the cursor appears for
	int off_ticks = 7; // How many ticks the cursor disappears for
	bool cursor_visible = true;

	while (game_running) {
		print_tile_map(tile_map, ROWS, COLS, OFFSET_X, OFFSET_Y);
		print_road_map(road_map, ROWS, COLS, OFFSET_X, OFFSET_Y);
		print_building_map(building_map, ROWS, COLS, OFFSET_X, OFFSET_Y);
		print_controls(0, ROWS + OFFSET_Y);
		if (cursor_visible) {
			print_cursor(cx, cy, OFFSET_X, OFFSET_Y);
		}
		print_city_info(city, COLS, 0);

		if (!cursor_moving) {
			if (ticks == on_ticks) {
				cursor_visible = false;
			}
			else if (ticks == on_ticks + off_ticks) {
				ticks = 0;
				cursor_visible = true;
			}
			ticks++;
		}
		else if (cursor_moving) {
			cursor_visible = true;
		}

		refresh(); // Show the screen

		handle_controls(city, road_map, building_map, cx, cy);

		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS));
	}

	delete_tile_map(tile_map, ROWS, COLS);
	delete_road_map(road_map, ROWS, COLS);
	delete_building_map(building_map, ROWS, COLS);

	endwin();
	return 0;
}

void print_city_info(City city, int ox, int oy) {
	// Border
	for (int i = 0; i < ROWS + 1; i++) {
		if (i != ROWS) mvprintw(0 + i, ox, "|");
		else		   mvprintw(0 + i, ox, "/");
	}
	ox += 2;

	// Clear stuff first
	mvprintw(oy + 1, ox, "\t\t\t\t\t\t");
	mvprintw(oy + 2, ox, "\t\t\t\t\t\t");
	mvprintw(oy + 5, ox, "\t\t\t\t\t\t");
	mvprintw(oy + 6, ox, "\t\t\t\t\t\t");
	mvprintw(oy + 7, ox, "\t\t\t\t\t\t");
	mvprintw(oy + 8, ox, "\t\t\t\t\t\t");
	mvprintw(oy + 9, ox, "\t\t\t\t\t\t");

	mvprintw(oy + 0, ox - 1, "\tCity Info:");
	mvprintw(oy + 1, ox, "Funds: \t$%d", city.total_funds);
	mvprintw(oy + 2, ox, "Residents: \t%d", city.total_residents);

	mvprintw(oy + 4, ox, "\tBuilding Info:");
	mvprintw(oy + 5, ox, "Total residential buildings: \t%d", city.total_residential_buildings);
	mvprintw(oy + 6, ox, "Total commercial buildings: \t%d", city.total_commercial_buildings);
	mvprintw(oy + 7, ox, "Total industrial buildings: \t%d", city.total_industrial_buildings);
	mvprintw(oy + 8, ox, "Total roads: \t\t\t%d", city.total_roads);
	mvprintw(oy + 9, ox, "Total: \t\t\t%d", city.get_total_buildings());

	mvprintw(oy + 11, ox, "\tIncome Info:");
}

void print_controls(int ox, int oy) {
	int offset = 0;

	for (int i = 0; i < 10; i++) {
		mvprintw(oy + i, ox, "\t\t\t\t\t\t");
	}

	for (int i = 0; i < COLS; i++) {
		mvprintw(oy, ox + i, "-");
	}
	oy++;

	switch (game_state) {
	case MAP_VIEW:
		mvprintw(oy + 0, ox, "b: build");
		mvprintw(oy + 1, ox, "r: remove");
		break;

	case BUILD_VIEW:
		mvprintw(oy + 0, ox, "e: confirm placement");
		if (tile_type == ROAD) {
			mvprintw(oy + 1, ox, "Current: Road");
		}
		else {
			const char* type_str = building_type_to_str(building_type);
			mvprintw(oy + 1, ox, "Current: %s", type_str);
		}
		mvprintw(oy + 2, ox, "r: road ($%d)", Road::price);
		mvprintw(oy + 3, ox, "h: residential ($%d)", Building::residential_price);
		mvprintw(oy + 4, ox, "m: commercial ($%d)", Building::commercial_price);
		mvprintw(oy + 5, ox, "i: industrial ($%d)", Building::industrial_price);
		mvprintw(oy + 7, ox, "c: cancel");
		break;

	case REMOVE_VIEW:
		mvprintw(oy + 0, ox, "e: confirm removal");
		mvprintw(oy + 1, ox, "c: cancel");
		break;
	}
}

void handle_controls(City &city, Road**& road_map, Building**& building_map, int& cx, int& cy) {
	int ch = getch();
	bool no_vertical = false;
	bool no_horizontal = false;

	// Handle vertical movement
	if (ch == KEY_UP && cy > 0) {
		cy--;
		cursor_moving = true;
	}
	else if (ch == KEY_DOWN && cy < ROWS - 1) {
		cy++;
		cursor_moving = true;
	}
	else {
		no_vertical = true;
	}

	// Handle horizontal movement
	if (ch == KEY_LEFT && cx > 0) {
		cx--;
		cursor_moving = true;
	}
	else if (ch == KEY_RIGHT && cx < COLS - 1) {
		cx++;
		cursor_moving = true;
	}
	else {
		no_horizontal = true;
	}

	if (no_vertical && no_horizontal) {
		cursor_moving = false;
	}

	if (game_state == MAP_VIEW) {
		if (ch == 'b') game_state = BUILD_VIEW;
		if (ch == 'r') game_state = REMOVE_VIEW;
	}

	if (game_state == BUILD_VIEW) {
		if (ch == 'r') {
			tile_type = ROAD;
			building_type = EMPTY;
		}
		else if (ch == 'h') {
			tile_type = BUILDING;
			building_type = RESIDENTIAL;
		}
		else if (ch == 'm') {
			tile_type = BUILDING;
			building_type = COMMERCIAL;
		}
		else if (ch == 'i') {
			tile_type = BUILDING;
			building_type = INDUSTRIAL;
		}

		// Placing stuff
		if (ch == 'e') {
			bool valid_placement = false;

			if (tile_type == ROAD) {
				if (road_map[cy][cx].type == EMPTY_TILE && building_map[cy][cx].type == EMPTY) {
					road_map[cy][cx] = Road(ROAD, cx, cy);
					road_map[cy][cx].update_road(road_map, COLS, ROWS, true);
					city.total_roads++;
				}
			}
			else if (tile_type == BUILDING) {
				Building building(building_type, cx, cy);
				
				// Atleast 1 road in the cardinal directions of the building
				if (cy < ROWS - 1 && road_map[cy + 1][cx].type == ROAD) valid_placement = true;
				if (cy > 0 && road_map[cy - 1][cx].type == ROAD) valid_placement = true;
				if (cx < COLS - 1 && road_map[cy][cx - 1].type == ROAD) valid_placement = true;
				if (cx > 0 && road_map[cy][cx + 1].type == ROAD) valid_placement = true;

				// Industrial buildings must be spaced by 1 tile.
				if (building_type == INDUSTRIAL) {
					if (cy < ROWS - 1 && building_map[cy + 1][cx].type == INDUSTRIAL) valid_placement = false; // Up
					if (cy > 0 && building_map[cy - 1][cx].type == INDUSTRIAL) valid_placement = false; // Down
					if (cx < COLS - 1 && building_map[cy][cx - 1].type == INDUSTRIAL) valid_placement = false; // Left
					if (cx > 0 && building_map[cy][cx + 1].type == INDUSTRIAL) valid_placement = false; // Right
				}

				if (building_map[cy][cx].type == BUILDING) valid_placement = false; // Not placing on top of a building
				if (road_map[cy][cx].type == ROAD) valid_placement = false; // Not placing on top of a road

				if (valid_placement) {
					building_map[cy][cx] = building;
					if (building_type == RESIDENTIAL) {
						city.total_residential_buildings++;
					}
					if (building_type == COMMERCIAL) {
						city.total_commercial_buildings++;
					}
					if (building_type == INDUSTRIAL) {
						city.total_industrial_buildings++;
					}
				}
			}
		}
	}
	else if (game_state == REMOVE_VIEW) {
		if (ch == 'e') {
			if (road_map[cy][cx].type != EMPTY_TILE) {
				road_map[cy][cx] = Road(EMPTY_TILE, cx, cy);
				city.total_roads--;
			}
			else if (building_map[cy][cx].type != EMPTY) {
				BuildingType b_type = building_map[cy][cx].type;

				building_map[cy][cx] = Building(EMPTY, cx, cy);

				if (b_type == RESIDENTIAL) {
					city.total_residential_buildings--;
				}
				if (b_type == COMMERCIAL) {
					city.total_commercial_buildings--;
				}
				if (b_type == INDUSTRIAL) {
					city.total_industrial_buildings--;
				}
			}
		}
	}

	// Universal cancel button thing
	if (ch == 'c') game_state = MAP_VIEW;
}

void print_cursor(int cx, int cy, int ox, int oy) {
	attron(COLOR_PAIR(1));
	mvprintw(cy + oy, cx + ox, "X");
	attroff(COLOR_PAIR(1));
}

const char* building_type_to_str(BuildingType bt) {
	switch (bt) {
		case RESIDENTIAL: return "Residential";
		case COMMERCIAL:  return "Commercial";
		case INDUSTRIAL:  return "Industrial";
		default: return "Invalid building type";
	}
}

WINDOW* init() {
	// Curses initialization
	WINDOW* win = initscr();
	curs_set(0);
	nodelay(win, true);
	keypad(win, true);
	noecho();

	start_color();
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	init_pair(4, COLOR_WHITE, COLOR_BLACK);
	init_pair(6, COLOR_BLACK, COLOR_WHITE);

	// Game initialization
	game_state = MAP_VIEW;

	return win;
}
