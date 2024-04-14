#ifndef INTERP_H
#define INTERP_H
/*
    Interpreter main loop and data structures
*/
#include <stdbool.h>
#include <stdint.h>

// TODO: Status line should be part of gamedata struct?
extern char status_line[40];

// todo: consider struct for optimisation
enum control {
    PROGRAM_CONTROL,
    PLAYER_CONTROL
};

struct logic_stack {
    uint8_t logic_no;
    uint16_t pc;
};

typedef struct {
    int x1, y1;
    int x2, y2;
} rect;

typedef struct {
    bool animate;
    bool draw;
    bool fix_loop;
    bool start_update;
    bool ignore_horizon;
    bool is_cycling;
    bool ignore_objects;
    bool ignore_blocks;
    bool has_fixed_priority;

    int x, y;
    int width, height; // todo: are these needed?
    uint8_t view_no;
    uint8_t loop_no;
    uint8_t cel_no;

    int old_x, old_y;
    int old_view_no;
    uint8_t old_loop_no;
    uint8_t old_cel_no;

    uint8_t fixed_priority;
    uint8_t allowed_on;

    uint8_t cycling_mode;
    uint8_t cycle_time;
    uint8_t cycles_to_next_update;

    uint8_t move_mode;
    uint8_t step_time;
    uint8_t steps_to_next_update;

    uint8_t step_size;

    int move_distance_x, move_distance_y;
    uint8_t move_step_size;
    uint8_t move_done_flag;

    uint8_t end_of_loop_flag;

    uint8_t direction;

    uint8_t wander_distance;
} animated_object;

typedef struct MenuItem {
    char* text;
    uint8_t controller; // or any other relevant data
    bool enabled;
    struct MenuItem* next;
} MenuItem;

typedef struct Menu {
    char* title;
    MenuItem* items;
    struct Menu* next;
} Menu;

typedef struct {
    int controller_number;
    char key_press_info;
} ControllerAssignment;

// todo: this struct should track *EVERYTHING*
struct gamedata {
    char id[6];
    uint16_t pc;
    // bool test;
    // bool and_result;
    // bool or_result;
    // bool or_test;
    // bool negate;
    uint8_t current_logic;

    struct logic_stack callstack[8]; // depth is 8
    uint8_t callstack_ptr;           // ?

    bool cycle_complete;

    enum control control_mode;

    uint8_t vars[256]; // 0-26 reserved by interpreter
    
    /*
    TODO: to squeeze some memory, flags could be stored in 32 bytes:
    uint8_t flags[32]; // 32 * 8 = 256 bits

    // Set a flag
    void set_flag(int index) {
        flags[index / 8] |= 1 << (index % 8);
    }

    // Clear a flag
    void clear_flag(int index) {
        flags[index / 8] &= ~(1 << (index % 8));
    }

    // Check a flag
    bool check_flag(int index) {
        return flags[index / 8] & (1 << (index % 8));
    }
    */
    bool flags[256];   // 0-15 reserved by interpreter
    char strings[12][40];

    bool enter_pressed;
    uint8_t horizon;
    bool input_enabled;
    char cursor;

    bool block_active;
    rect block;

    animated_object objects[32];

    // int soung_flag; //?
    bool status_line_enabled;

    bool controller_flags[50];
    ControllerAssignment controller_assignments[50];
    char controller_evaluation[50];

    uint8_t playtop, input_line, status_line;

    Menu* menus; 
};

extern struct gamedata game;

extern Menu* menu;

// inventory? objects(inventory items) strings?

// Initialise the interpreter
void interp_init(void);

// Run the interpreter loop
void interp_run(void);

#endif // INTERP_H