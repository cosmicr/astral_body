#include <cbm.h>
#include <conio.h>
#include <cx16.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"
#include "cx16api.h"
#include "interp.h"
#include "logic.h"
#include "resource.h"

char status_line[40];

struct gamedata game;

Menu* menu;

char keyboard_buffer[40];

/* 
    Initialise the game variables, flags, strings and load logic 0.
*/
void interp_init(void)
{
    uint8_t i;

    // set all vars and flags to 0
    for (i = 0; i != 255; i++) {
        game.vars[i] = 0;
        game.flags[i] = false;
    }

    for (i = 0; i < 12; i++) {
        game.strings[i][0] = '\0';
    }
    game.strings[0][0] = '>';
    game.strings[0][1] = '\0';

    game.pc = 0;
    // game.test = false;
    // game.and_result = true;
    // game.or_result = true;
    // game.or_test = false;
    // game.negate = false;
    game.current_logic = 0;

    game.callstack_ptr = 0;

    game.cycle_complete = false;

    game.control_mode = PLAYER_CONTROL;

    /*
    for (i = 0; i < MAX_NUM_OBJECTS; i++) {
        state.objects[i].animate = false;
        state.objects[i].draw = false;
        state.objects[i].start_update = true;
        state.objects[i].old_view_no = -1;
    }
    state.sound_flag = -1;
    */

    for (i = 0; i < 50; i++) {
        game.controller_flags[i] = false;
    }

    // need to set these vars:
    // var 20 computer type
    game.vars[20] = 0; // IBM-PC
    // var 26 monitor type
    game.vars[26] = 3; // EGA
    // var 24 input length?
    game.vars[24] = 40;
    // var 8 free memory?
    game.vars[8] = 0x10000; // 64k?

    // var 22 sound type (tandy?)
    game.vars[22] = 3; // Tandy

    // note: nagi keeps lists for logics, pictures, views, sounds, words, animated_objs, and inventory items for each new room

    // System Stuff
    keyboard_buffer[0] = '\0';

    game.cursor = '_';

    // load logic 0
    load_resource(LOGIC, 0); 

    game.flags[5] = true; // new_room
}

// todo: move this and other keyboard stuff into their own file
void clear_keybuf(char* keybuffer)
{
    uint8_t i;
    for (i = 0; i != 40; i++) {
        keybuffer[i] = ' ';
    }
}

// Run the main interpreter loop
void interp_run(void)
{
    static char key = 0;
    static uint8_t i;
    static char keybuffer[40];
    static uint8_t buffer_index = 0;
    // main loop:

    // interpreter work cycle (loop):
    // 1. delay time
    for (i=0;i<game.vars[10];i++) {
        // wait 1/20th of a second (refresh is 1/60th of a second multiplied by 3)
        asm_wait_for_refresh(); 
        asm_wait_for_refresh(); 
        asm_wait_for_refresh(); 
    }

    // 2. clear keyboard buffer
    // todo: does cx16 have a keyboard buffer?

    // set flag 2 to 0
    game.flags[2] = false; // has player issued a command?

    // 3. read keyboard (and joystick)
    // todo: use routines in keyboard.h
    // todo: it should only clear if input_received (flag 2)
    
    game.vars[19] = 0; // v19 = pressed key
    kbdbuf_peek(&i);
    if (i != 0) {
        key = cbm_k_getin();
        game.vars[19] = key; // v19 = pressed key
        // first check for backspace
        if (key == 20) {
            if (buffer_index > 0) {
                buffer_index--;
                keybuffer[buffer_index] = '\0';
                // move the cursor back one position, print a space, then move back again
                gotoxy(buffer_index, 22);
                cputc(' ');
                gotoxy(buffer_index, 22);
            }
        } else {
            keybuffer[buffer_index++] = key;
        }

        if (key == 13) { // enter/return key
            // todo: parse the input (check for words accepted in words.tok and put them into parsed words list)
            // if parsed words list is > 0, set flag 2
            game.flags[2] = true;

            clear_keybuf(keybuffer);
            buffer_index = 0;
        }
        // if key is an arrow key, react on it?
        // left = 157, right = 29, up = 145, down = 17
        else if (key == 157 || key == 29 || key == 145 || key == 17) {
            // set flag 2 to 1
            game.flags[2] = true;
            // set var 6 to the direction
            if (key == 157) {
                game.vars[6] = 7;
            }
            if (key == 29) {
                game.vars[6] = 1;
            }
            if (key == 145) {
                game.vars[6] = 3;
            }
            if (key == 17) {
                game.vars[6] = 5;
            }
        }
        // plot the keybuffer
        gotoxy(0, 22);
        bgcolor(0x00);
        textcolor(0x0f);
        cputs(keybuffer);
    }

    // if current mode is program_control:
    if (game.control_mode == PROGRAM_CONTROL) {
        // ego_motion_dir = var 6
        game.objects[0].direction = game.vars[6];
    }

    // if current mode is player_control:
    if (game.control_mode == PLAYER_CONTROL) {
        game.vars[6] = game.objects[0].direction;
    }

    // for all animated_objs which animate.obj, start_update and draw were called:

    for (i = 0; i != 255; i++) {
        if (game.objects[i].animate && game.objects[i].start_update && game.objects[i].draw) {
            // recalculate direction of movement
        }
    }

    // if score has changed (var 3) or sound toggled (flag 9), update status line
    // (just update it anyway)
    for (i = 0; i != 40; i++) {
        status_line[i] = '\0';
    }
    sprintf(status_line, "Score: %d", game.vars[3]);
    gotoxy(0, 0);
    bgcolor(0x0f);
    textcolor(0x90);
    if (game.status_line_enabled == true)
        printf("%-40s", status_line);

    // 4. execute logic 0 (including calls to other logics etc)
    execute_logic(0);

    // ego_motion_dir = var 6
    game.objects[0].direction = game.vars[6];
    // if score has changed (var 3) or sound toggled (flag 9), update status line
    // var 5 = 0 ; border code touched by any object
    // var 4 = 0 ; number of object touching border
    // flag 5 = 0 ; new_room?
    // flag 6 = 0 ; restart_game?
    // flag 12 = 0 ; restore_game?

    // update all controlled animated_objs

    // if new.room or new.room.v wasn't called goto step 1 (repeat this room)
    // else:
    // stop.update
    // unanimate.all
    // destroy logic resources (unload) except logic 0
    // player.control
    // unblock???
    // set horizon 36
    // var 1 = var 0 ; prev = current room
    // var 0 = new room number
    // var 4 = 0 ; number of object touching border
    // var 5 = 0 ; border code touched by any object
    // var 9 = 0 ; number of words in the user message not found in dictionary
    // var 16 = ego view number (set)
    // set ego coords based on var 2 (border touched)
    // var 2 = 0 ; border code touched by ego
    // flag 2 = 0 ; has player issued a command?
    // flag 5 = 1 ; new room?
    // score = var 3
}
