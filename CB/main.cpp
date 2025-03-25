#include "main.h"

#define ROWS 15
#define COLS 25

#define OFFSET_X 0
#define OFFSET_Y 0

#define FPS 60

void print_city_info(int ox, int oy);
void print_controls(int ox, int oy);
void print_cursor(int cx, int cy, int ox, int oy);
void handle_controls(Road** &road_map, Building** &building_map, int &cx, int &cy);
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

	bool game_running = true;

	int cx = 1;
	int cy = 1;

	int ticks = 0;
	int on_ticks = 7; // How many ticks the cursor appears for
	int off_ticks = 7; // How many ticks the cursor disappears for
	bool cursor_visible = true;

	while (game_running) {
		switch (game_state) {
			case MAP_VIEW:
			case BUILD_VIEW:
			case REMOVE_VIEW:
			default:
				print_tile_map(tile_map, ROWS, COLS, OFFSET_X, OFFSET_Y);
				print_road_map(road_map, ROWS, COLS, OFFSET_X, OFFSET_Y);
				print_building_map(building_map, ROWS, COLS, OFFSET_X, OFFSET_Y);
				print_controls(0, ROWS + OFFSET_Y);
				if (cursor_visible) {
					print_cursor(cx, cy, OFFSET_X, OFFSET_Y);
				}
				print_city_info(OFFSET_X, 0);
				break;
		}

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
		else {
			cursor_visible = true;
			ticks = 0;
		}

		refresh(); // Show the screen

		handle_controls(road_map, building_map, cx, cy);

		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS));
	}

	delete_tile_map(tile_map, ROWS, COLS);
	delete_road_map(road_map, ROWS, COLS);
	delete_building_map(building_map, ROWS, COLS);

	endwin();
	return 0;
}

void print_city_info(int ox, int oy) {
	// TO BE IMPLEMENTED
	return;
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
		mvprintw(oy + 0, ox, "Enter: confirm placement");
		if (tile_type == ROAD) {
			mvprintw(oy + 1, ox, "Current: Road");
		}
		else {
			const char* type_str = building_type_to_str(building_type);
			mvprintw(oy + 1, ox, "Current: %s", type_str);
		}
		mvprintw(oy + 2, ox, "r: road");
		mvprintw(oy + 3, ox, "h: residential");
		mvprintw(oy + 4, ox, "m: commercial");
		mvprintw(oy + 5, ox, "i: industrial");
		mvprintw(oy + 7, ox, "c: cancel");
		break;

	case REMOVE_VIEW:
		mvprintw(oy + 0, ox, "Enter: confirm removal");
		mvprintw(oy + 1, ox, "c: cancel");
		break;
	}
}

void handle_controls(Road**& road_map, Building**& building_map, int& cx, int& cy) {
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
		no_vertical = false;
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
		no_horizontal = false;
	}

	if (no_vertical && no_horizontal)
		cursor_moving = false;

	if (game_state == MAP_VIEW) {
		if (ch == 'b') game_state = BUILD_VIEW;
		if (ch == 'r') game_state = REMOVE_VIEW;
	}

	if (game_state == BUILD_VIEW) {
		if (ch == 'r') {
			tile_type = ROAD;
			building_type = EMPTY;
			cur_tile = new Road(ROAD, cx, cy);
		}
		else if (ch == 'h') {
			tile_type = BUILDING;
			building_type = RESIDENTIAL;
			cur_tile = new Building(building_type, cx, cy, 0, 0);
		}
		else if (ch == 'm') {
			tile_type = BUILDING;
			building_type = COMMERCIAL;
			cur_tile = new Building(building_type, cx, cy, 0, 0);
		}
		else if (ch == 'i') {
			tile_type = BUILDING;
			building_type = INDUSTRIAL;
			cur_tile = new Building(building_type, cx, cy, 0, 0);
		}

		if (ch == 'e' && cur_tile != nullptr) {
			if (building_type == EMPTY) {
				road_map[cy][cx] = Road(ROAD, cx, cy);
				road_map[cy][cx].update_road(road_map, COLS, ROWS, true);
			}
			else if (building_map[cy][cx].type == EMPTY) {
				Building building(building_type, cx, cy, 0, 0);
				building_map[cy][cx] = building;
			}
		}
	}
	else if (game_state == REMOVE_VIEW) {
	}

	// Universal cancel button thing
	if (ch == 'c') game_state = MAP_VIEW;
}

void print_cursor(int cx, int cy, int ox, int oy) {
	mvprintw(cy + oy, cx + ox, "X");
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
	init_pair(0, COLOR_GREEN, COLOR_BLACK);
	init_pair(1, COLOR_BLUE, COLOR_BLACK);
	init_pair(2, COLOR_YELLOW, COLOR_BLACK);
	init_pair(3, COLOR_WHITE, COLOR_BLACK);
	init_pair(5, COLOR_BLACK, COLOR_WHITE);

	// Game initialization
	game_state = MAP_VIEW;

	return win;
}
