#include "main.h"

#define ROWS 20
#define COLS 40

#define OFFSET_X 0
#define OFFSET_Y 0

#define TPS 60

void attempt_city_growth(City& city, Road** road_map, Building** building_map, int rows, int cols);
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
bool paused = false;

int year = 2000;
int day = 1;
int days_in_month[]  = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int month = 1;

int main(void) {
	WINDOW* win = init();
	Tile** tile_map = generate_tile_map(ROWS, COLS);
	Road** road_map = generate_road_map(ROWS, COLS);
	Building** building_map = generate_building_map(ROWS, COLS);
	Building* buildings = {};
	City city = City(road_map, building_map, 1000000, COLS, ROWS);

	bool game_running = true;

	int cx = 1;
	int cy = 1;

	int game_ticks = 0; // 60 TPS 
	int days_per_second = 10;
	int ticks_per_day = TPS / days_per_second;

	int cursor_ticks = 0;
	int cursor_on_ticks = 7; // How many ticks the cursor appears for
	int cursor_off_ticks = 7; // How many ticks the cursor disappears for
	bool cursor_visible = true;

	while (game_running) {
		print_tile_map(tile_map, ROWS, COLS, OFFSET_X, OFFSET_Y);
		print_road_map(road_map, ROWS, COLS, OFFSET_X, OFFSET_Y);
		print_building_map(building_map, ROWS, COLS, OFFSET_X, OFFSET_Y);
		print_controls(0, ROWS + OFFSET_Y);
		if (cursor_visible) print_cursor(cx, cy, OFFSET_X, OFFSET_Y);
		print_city_info(city, COLS, 0);

		if (!cursor_moving) {
			if (cursor_ticks == cursor_on_ticks) {
				cursor_visible = false;
			}
			else if (cursor_ticks == cursor_on_ticks + cursor_off_ticks) {
				cursor_ticks = 0;
				cursor_visible = true;
			}
			cursor_ticks++;
		}
		else cursor_visible = true;

		if (!paused) game_ticks++;
		if (game_ticks % ticks_per_day == 0) { // Every day
			attempt_city_growth(city, road_map, building_map, ROWS, COLS);

			day++;
			// Every month
			if (day > days_in_month[month - 1]) {
				day = 1;
				month += 1;

				city.total_funds += city.get_total_monthly_funds();
				if (month == 13) {
					year++;
					month = 1;
				}
			}
		}

		refresh(); // Show the screen
		handle_controls(city, road_map, building_map, cx, cy);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / TPS));
	}

	delete_tile_map(tile_map, ROWS, COLS);
	delete_road_map(road_map, ROWS, COLS);
	delete_building_map(building_map, ROWS, COLS);

	endwin();
	return 0;
}

Building* get_specific_buildings(City city, Building** building_map, BuildingType type, int rows, int cols) {
	Building* buildings = new Building [city.total_commercial_buildings];
	int index = 0;

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (building_map[i][j].type == type) {
				buildings[index] = building_map[i][j];
				index++;
			}
		}
	}

	return buildings;
}

void attempt_city_growth(City& city, Road** road_map, Building** building_map, int rows, int cols) {
	// How many people are moving in this growth tick
	int incoming_people = rand() % Building::max_residents;
	if (incoming_people == 0) return; // Don't bother if no one is coming

	// The house being chosen for growth
	//int house_index = rand() % city.total_residential_buildings;
	//Building* buildings = get_specific_buildings(city, building_map, RESIDENTIAL, rows, cols);
	//Building chosen_building = buildings[house_index];

}

const char* month_to_str(int month) {
	switch (month) {
		case 1:  return "Jan";
		case 2:  return "Feb";
		case 3:  return "Mar";
		case 4:  return "Apr";
		case 5:  return "May";
		case 6:  return "Jun";
		case 7:  return "Jul";
		case 8:  return "Aug";
		case 9:  return "Sep";
		case 10: return "Oct";
		case 11: return "Nov";
		case 12: return "Dec";
	}
}

void print_city_info(City city, int ox, int oy) {
	// Border
	for (int i = 0; i < ROWS + 1; i++) {
		if (i != ROWS) mvprintw(0 + i, ox, "|");
		else		   mvprintw(0 + i, ox, "/");
	}
	ox += 2;

	// Clear stuff first
	mvprintw(oy + 0, ox, "\t\t\t\t\t\t");
	mvprintw(oy + 1, ox, "\t\t\t\t\t\t");
	mvprintw(oy + 2, ox, "\t\t\t\t\t\t");
	mvprintw(oy + 5, ox, "\t\t\t\t\t\t");
	mvprintw(oy + 6, ox, "\t\t\t\t\t\t");
	mvprintw(oy + 7, ox, "\t\t\t\t\t\t");
	mvprintw(oy + 8, ox, "\t\t\t\t\t\t");
	mvprintw(oy + 9, ox, "\t\t\t\t\t\t");

	mvprintw(oy + 0, ox - 1, "\tCity Info [%s %d, %d]:", month_to_str(month), day, year);
	mvprintw(oy + 1, ox, "Funds: \t\t$%d", city.total_funds);
	mvprintw(oy + 2, ox, "Income (monthly): \t$%d (Commercial: %d | Industrial: %d)", city.get_total_monthly_funds(), city.get_commercial_monthly_funds(), city.get_industrial_monthly_funds());
	mvprintw(oy + 3, ox, "Residents: \t\t%d (Max: %d)", city.total_residents, city.get_max_residents());

	mvprintw(oy + 5, ox, "\tBuilding Info:");
	mvprintw(oy + 6, ox, "Total residential buildings: \t%d", city.total_residential_buildings);
	mvprintw(oy + 7, ox, "Total commercial buildings: \t%d", city.total_commercial_buildings);
	mvprintw(oy + 8, ox, "Total industrial buildings: \t%d", city.total_industrial_buildings);
	mvprintw(oy + 9, ox, "Total roads: \t\t\t%d", city.total_roads);
	mvprintw(oy + 10, ox, "Total: \t\t\t%d", city.get_total_buildings());

	mvprintw(oy + 12, ox, "\tIncome Info:");
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
			paused = false;
			mvprintw(oy + 0, ox, "b: build");
			mvprintw(oy + 1, ox, "r: remove");
			mvprintw(oy + 2, ox, "p: pause");
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
			mvprintw(oy + 2, ox, "r: [=] road \t\t($%d)", Road::price);
			mvprintw(oy + 3, ox, "h: [^] residential\t($%d)", Building::residential_price);
			mvprintw(oy + 4, ox, "m: [o] commercial \t($%d)", Building::commercial_price);
			mvprintw(oy + 5, ox, "i: [L] industrial \t($%d)", Building::industrial_price);
			mvprintw(oy + 7, ox, "c: cancel");
			break;

		case REMOVE_VIEW:
			mvprintw(oy + 0, ox, "e: confirm removal");
			mvprintw(oy + 1, ox, "c: cancel");
			break;

		case PAUSE_VIEW:
			mvprintw(oy + 0, ox, "[Game Paused]");
			mvprintw(oy + 1, ox, "c: cancel");
			paused = true;
			break;
	}
}

void handle_controls(City &city, Road**& road_map, Building**& building_map, int& cx, int& cy) {
	int ch = getch();
	bool no_vertical = false;
	bool no_horizontal = false;

	if (paused) {
		if (ch == 'c') {
			game_state = MAP_VIEW;
		}
		return;
	}

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
		if (ch == 'p') game_state = PAUSE_VIEW;
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
				if (road_map[cy][cx].type == EMPTY_TILE &&
					building_map[cy][cx].type == EMPTY &&
					city.total_funds >= Road::price
				) {
					road_map[cy][cx] = Road(ROAD, cx, cy);
					road_map[cy][cx].update_road(road_map, COLS, ROWS, true);
					city.total_roads++;
					city.total_funds -= Road::price;
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
					if (building_type == RESIDENTIAL && city.total_funds >= Building::residential_price) {
						city.total_residential_buildings++;
						building_map[cy][cx] = building;
						city.total_funds -= Building::residential_price;
					}
					if (building_type == COMMERCIAL && city.total_funds >= Building::commercial_price) {
						city.total_commercial_buildings++;
						building_map[cy][cx] = building;
						city.total_funds -= Building::commercial_price;
					}
					if (building_type == INDUSTRIAL && city.total_funds >= Building::industrial_price) {
						city.total_industrial_buildings++;
						building_map[cy][cx] = building;
						city.total_funds -= Building::industrial_price;
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
	attron(COLOR_PAIR(6));
	mvprintw(cy + oy, cx + ox, " ");
	attroff(COLOR_PAIR(6));
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
