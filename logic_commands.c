#include <conio.h>
#include <cx16.h>
#include <stdlib.h>
#include <string.h>

#include "dprint.h"
#include "interp.h"
#include "logic.h"
#include "logic_commands.h"
#include "picture.h"
#include "resource.h"
#include "text.h"

void command_00_return(uint16_t* offset)
{
    dprintf("\nret");
    // set offset to -1 to indicate return
    *offset = -1;

    // asm("jsr popax");
    // asm("sta $80");
    // asm("stx $81");
    // asm("lda #$FF");
    // asm("sta ($80),y");
    // asm("lda #$FF");
    // asm("ldy #$01");
    // asm("sta ($80),y");
}

void command_01_increment(uint16_t* offset)
{
    // uint8_t var = read_bank_data(bank, (*offset)++);
    uint8_t var = read_heap((*offset)++);
    dprintf_args("\ninc %d", var);
    ++(game.vars[var]);
}
void command_02_decrement(uint16_t* offset)
{
    // uint8_t var = read_bank_data(bank, (*offset)++);
    uint8_t var = read_heap((*offset)++);
    dprintf_args("\ndec %d", var);
    --(game.vars[var]);
}

void command_03_assignn(uint16_t* offset)
{
    // uint8_t var = read_bank_data(bank, (*offset)++);
    // uint8_t value = read_bank_data(bank, (*offset)++);
    uint8_t var = read_heap((*offset)++);
    uint8_t value = read_heap((*offset)++);
    dprintf_args("\nassignn %d, %d", var, value);
    game.vars[var] = value;
}

void command_04_assignv(uint16_t* offset)
{
    // uint8_t var = read_bank_data(bank, (*offset)++);
    // uint8_t value = read_bank_data(bank, (*offset)++);
    uint8_t var = read_heap((*offset)++);
    uint8_t value = read_heap((*offset)++);
    dprintf_args("\nassignv %d, %d", var, value);
    game.vars[var] = game.vars[value];
}

void command_05_addn(uint16_t* offset)
{
    // uint8_t var = read_bank_data(bank, (*offset)++);
    // uint8_t value = read_bank_data(bank, (*offset)++);
    uint8_t var = read_heap((*offset)++);
    uint8_t value = read_heap((*offset)++);
    dprintf_args("\naddn %d, %d", var, value);
    game.vars[var] += value;
}
void command_06_addv(uint16_t* offset)
{
    // uint8_t var = read_bank_data(bank, (*offset)++);
    // uint8_t value = read_bank_data(bank, (*offset)++);
    uint8_t var = read_heap((*offset)++);
    uint8_t value = read_heap((*offset)++);
    dprintf_args("\naddv %d, %d", var, value);
    game.vars[var] += game.vars[value];
}
void command_07_subn(uint16_t* offset)
{
    // uint8_t var = read_bank_data(bank, (*offset)++);
    // uint8_t value = read_bank_data(bank, (*offset)++);
    uint8_t var = read_heap((*offset)++);
    uint8_t value = read_heap((*offset)++);
    dprintf_args("\nsubn %d, %d", var, value);
    game.vars[var] -= value;
}
void command_08_subv(uint16_t* offset)
{
    // uint8_t var = read_bank_data(bank, (*offset)++);
    // uint8_t value = read_bank_data(bank, (*offset)++);
    uint8_t var = read_heap((*offset)++);
    uint8_t value = read_heap((*offset)++);
    dprintf_args("\nsubv %d, %d", var, value);
    game.vars[var] -= game.vars[value];
}
void command_09_lindirectv(uint16_t* offset)
{
    // uint8_t var = read_bank_data(bank, (*offset)++);
    // uint8_t value = read_bank_data(bank, (*offset)++);
    uint8_t var = read_heap((*offset)++);
    uint8_t value = read_heap((*offset)++);
    dprintf_args("\nlindirectv %d, %d", var, value);
    game.vars[game.vars[var]] = game.vars[value];
}
void command_0a_rindirect(uint16_t* offset)
{
    // uint8_t var = read_bank_data(bank, (*offset)++);
    // uint8_t value = read_bank_data(bank, (*offset)++);
    uint8_t var = read_heap((*offset)++);
    uint8_t value = read_heap((*offset)++);
    dprintf_args("\nrindirect %d, %d", var, value);
    game.vars[var] = game.vars[game.vars[value]];
}
void command_0b_lindirectn(uint16_t* offset)
{
    // uint8_t var = read_bank_data(bank, (*offset)++);
    // uint8_t value = read_bank_data(bank, (*offset)++);
    uint8_t var = read_heap((*offset)++);
    uint8_t value = read_heap((*offset)++);
    dprintf_args("\nlindirectn %d, %d", var, value);
    game.vars[game.vars[var]] = value;
}

void command_0c_set(uint16_t* offset)
{
    // uint8_t flag = read_bank_data(bank, (*offset)++);
    uint8_t flag = read_heap((*offset)++);
    dprintf_args("\nset %d", flag);
    game.flags[flag] = true;
}

void command_0d_reset(uint16_t* offset)
{
    // uint8_t flag = read_bank_data(bank, (*offset)++);
    uint8_t flag = read_heap((*offset)++);
    dprintf_args("\nreset %d", flag);
    game.flags[flag] = false;
}

void command_0e_toggle(uint16_t* offset)
{
    // uint8_t flag = read_bank_data(bank, (*offset)++);
    uint8_t flag = read_heap((*offset)++);
    dprintf_args("\ntoggle %d", flag);
    game.flags[flag] = !game.flags[flag];
}

void command_0f_set_v(uint16_t* offset)
{
    // uint8_t flag = read_bank_data(bank, (*offset)++);
    uint8_t flag = read_heap((*offset)++);
    dprintf_args("\nset.v %d", flag);
    game.flags[game.vars[flag]] = true;
}
void command_10_reset_v(uint16_t* offset)
{
    // uint8_t flag = read_bank_data(bank, (*offset)++);
    uint8_t flag = read_heap((*offset)++);
    dprintf_args("\nreset.v %d", flag);
    game.flags[game.vars[flag]] = false;
}
void command_11_toggle_v(uint16_t* offset)
{
    // uint8_t flag = read_bank_data(bank, (*offset)++);
    uint8_t flag = read_heap((*offset)++);
    dprintf_args("\ntoggle.v %d", flag);
    game.flags[game.vars[flag]] = !game.flags[game.vars[flag]];
}

void command_12_new_room(uint16_t* offset)
{
    // uint8_t room = read_bank_data(bank, (*offset)++);
    uint8_t room = read_heap((*offset)++);
    dprintf_args("\nnew.room %d", room);

    game.vars[1] = game.vars[0]; // prev.room
    game.vars[0] = room;

    // todo: new.room

    game.flags[5] = true; // new_room

    // load the logic for the new room
    load_resource(logdir, room);

    // set the offset to 2
    *offset = 2;
}
void command_13_new_room_v(uint16_t* offset)
{
    // uint8_t var = read_bank_data(bank, (*offset)++);
    uint8_t var = read_heap((*offset)++);
    dprintf_args("\nnew.room.v %d", var);

    game.vars[1] = game.vars[0]; // prev.room
    game.vars[0] = game.vars[var];

    game.flags[5] = true; // new_room

    // load the logic for the new room
    load_resource(logdir, game.vars[var]);

    // set the offset to 2
    *offset = 2;
}

void command_14_load_logics(uint16_t* offset) {}

void command_15_load_logics_v(uint16_t* offset)
{
    // uint8_t num = read_bank_data(bank, (*offset)++);
    uint8_t num = read_heap((*offset)++);
    dprintf_args("\nload.logics.v %d", num);
    // TODO: load.logics.v
}

void command_16_call(uint16_t* offset)
{
    // uint8_t num = read_bank_data(bank, (*offset)++);
    uint8_t num = read_heap((*offset)++);
    dprintf_args("\ncall %d", num);
    load_resource(logdir, num);
    execute_logic(num);
    unload_resource(logdir, num);
    // TODO: script table (for save/load games)
}

void command_17_call_v(uint16_t* offset)
{
    // uint8_t var = read_bank_data(bank, (*offset)++);
    uint8_t var = read_heap((*offset)++);
    dprintf_args("\ncall.v %d", var);
    load_resource(logdir, game.vars[var]);
    execute_logic(game.vars[var]);
    unload_resource(logdir, game.vars[var]);
}
void command_18_load_pic(uint16_t* offset)
{
    // uint8_t pic_num = read_bank_data(bank, (*offset)++);
    uint8_t pic_num = read_heap((*offset)++);
    dprintf_args("\nload.pic %d", pic_num);

    // load the pic
    load_resource(picdir, pic_num);
}

void command_19_draw_pic(uint16_t* offset)
{
    // uint8_t pic_num = read_bank_data(bank, (*offset)++);
    uint8_t pic_num = read_heap((*offset)++);
    dprintf_args("\ndraw.pic %d", pic_num);

    // draw the pic
    draw_pic();
}

void command_1a_show_pic(uint16_t* offset) 
{
    (void)offset;
    dprintf_args("\nshow.pic");
    show_pic();
}

void command_1b_discard_pic(uint16_t* offset)
{
    // uint8_t pic_num = read_bank_data(bank, (*offset)++);
    uint8_t pic_num = read_heap((*offset)++);
    dprintf_args("\ndiscard.pic %d", pic_num);
    unload_resource(picdir, pic_num);
}

void command_1c_overlay_pic(uint16_t* offset) {}
void command_1d_show_pri_screen(uint16_t* offset) {}
void command_1e_load_view(uint16_t* offset)
{
    // uint8_t view_num = read_bank_data(bank, (*offset)++);
    uint8_t view_num = read_heap((*offset)++);
    dprintf_args("\nload.view %d", view_num);
    load_resource(viewdir, view_num);
}

void command_1f_load_view_v(uint16_t* offset)
{
    // uint8_t var = read_bank_data(bank, (*offset)++);
    uint8_t var = read_heap((*offset)++);
    dprintf_args("\nload.view.v %d", var);
}

void command_20_discard_view(uint16_t* offset)
{
    // uint8_t view_num = read_bank_data(bank, (*offset)++);
    uint8_t view_num = read_heap((*offset)++);
    dprintf_args("\ndiscard.view %d", view_num);
    unload_resource(viewdir, view_num);
}

void command_21_animate_obj(uint16_t* offset) {}
void command_22_unanimate_all(uint16_t* offset) {}
void command_23_draw(uint16_t* offset) {}
void command_24_erase(uint16_t* offset) {}
void command_25_position(uint16_t* offset) {}
void command_26_position_v(uint16_t* offset) {}
void command_27_get_posn(uint16_t* offset) {}
void command_28_reposition(uint16_t* offset) {}
void command_29_set_view(uint16_t* offset) {}
void command_2a_set_view_v(uint16_t* offset) {}
void command_2b_set_loop(uint16_t* offset) {}
void command_2c_set_loop_v(uint16_t* offset) {}
void command_2d_fix_loop(uint16_t* offset) {}
void command_2e_release_loop(uint16_t* offset) {}
void command_2f_set_cel(uint16_t* offset) {}
void command_30_set_cel_v(uint16_t* offset) {}
void command_31_last_cel(uint16_t* offset) {}
void command_32_current_cel(uint16_t* offset) {}
void command_33_current_loop(uint16_t* offset) {}
void command_34_current_view(uint16_t* offset) {}
void command_35_number_of_loops(uint16_t* offset) {}
void command_36_set_priority(uint16_t* offset) {}
void command_37_set_priority_v(uint16_t* offset) {}
void command_38_release_priority(uint16_t* offset) {}
void command_39_get_priority(uint16_t* offset) {}
void command_3a_stop_update(uint16_t* offset) {}
void command_3b_start_update(uint16_t* offset) {}
void command_3c_force_update(uint16_t* offset) {}
void command_3d_ignore_horizon(uint16_t* offset) {}
void command_3e_observe_horizon(uint16_t* offset) {}

void command_3f_set_horizon(uint16_t* offset)
{
    // uint8_t horizon = read_bank_data(bank, (*offset)++);
    uint8_t horizon = read_heap((*offset)++);
    dprintf_args("\nset.horizon %d", horizon);
    game.horizon = horizon;
}

void command_40_object_on_water(uint16_t* offset) {}
void command_41_object_on_land(uint16_t* offset) {}
void command_42_object_on_anything(uint16_t* offset) {}
void command_43_ignore_objs(uint16_t* offset) {}
void command_44_observe_objs(uint16_t* offset) {}
void command_45_distance(uint16_t* offset) {}
void command_46_stop_cycling(uint16_t* offset) {}
void command_47_start_cycling(uint16_t* offset) {}
void command_48_normal_cycle(uint16_t* offset) {}
void command_49_end_of_loop(uint16_t* offset) {}
void command_4a_reverse_cycle(uint16_t* offset) {}
void command_4b_reverse_loop(uint16_t* offset) {}
void command_4c_cycle_time(uint16_t* offset) {}
void command_4d_stop_motion(uint16_t* offset) {}
void command_4e_start_motion(uint16_t* offset) {}
void command_4f_step_size(uint16_t* offset) {}
void command_50_step_time(uint16_t* offset) {}
void command_51_move_obj(uint16_t* offset) {}
void command_52_move_obj_v(uint16_t* offset) {}
void command_53_follow_ego(uint16_t* offset) {}
void command_54_wander(uint16_t* offset) {}
void command_55_normal_motion(uint16_t* offset) {}
void command_56_set_dir(uint16_t* offset) {}
void command_57_get_dir(uint16_t* offset) {}
void command_58_ignore_blocks(uint16_t* offset) {}
void command_59_observe_blocks(uint16_t* offset) {}
void command_5a_block(uint16_t* offset) {}
void command_5b_unblock(uint16_t* offset) {}
void command_5c_get(uint16_t* offset) {}
void command_5d_get_v(uint16_t* offset) {}
void command_5e_drop(uint16_t* offset) {}
void command_5f_put(uint16_t* offset) {}
void command_60_put_v(uint16_t* offset) {}
void command_61_get_room_v(uint16_t* offset) {}
void command_62_load_sound(uint16_t* offset)
{
    // uint8_t sound_num = read_bank_data(bank, (*offset)++);
    uint8_t sound_num = read_heap((*offset)++);
    dprintf_args("\nload.sound %d", sound_num);
}

void command_63_sound(uint16_t* offset) {}
void command_64_stop_sound(uint16_t* offset) {}
void command_65_print(uint16_t* offset) {}
void command_66_print_v(uint16_t* offset) {}

void command_67_display(uint16_t* offset)
{
    // uint8_t row = read_bank_data(bank, (*offset)++);
    // uint8_t col = read_bank_data(bank, (*offset)++);
    // uint8_t message_num = read_bank_data(bank, (*offset)++);
    uint8_t row = read_heap((*offset)++);
    uint8_t col = read_heap((*offset)++);
    uint8_t message_num = read_heap((*offset)++);
    char* message = process_text(get_message(message_num));
    dprintf_args("\ndisplay row %d, col %d, message %d\n", row, col, message_num);
    dprintf_args("message address: %p\n", message);
    gotoxy(col, row);
    dprintf_args("%s", message);
    while (!kbhit())
        ;
    cgetc();

    dprintf("\n***END***");
    while (!kbhit())
        ;
    cgetc();
}
void command_68_display_v(uint16_t* offset) {}

void command_69_clear_lines(uint16_t* offset)
{
    uint8_t i, j;
    // uint8_t top = read_bank_data(bank, (*offset)++);
    // uint8_t bottom = read_bank_data(bank, (*offset)++);
    // uint8_t color = read_bank_data(bank, (*offset)++);
    uint8_t top = read_heap((*offset)++);
    uint8_t bottom = read_heap((*offset)++);
    uint8_t color = read_heap((*offset)++);
    dprintf_args("\nclear.lines %d, %d, %d", top, bottom, color);
    // if color is not 0 then clear to white
    if (color != 0) {
        bgcolor(15);
    }
    // clear lines from top to bottom
    for (i = top; i <= bottom; i++) {
        gotoxy(0, i);
        for (j = 0; j < 40; j++) {
            cputc(' ');
        }
    }
}

void command_6a_text_screen(uint16_t* offset) {}
void command_6b_graphics(uint16_t* offset) {}

void command_6c_set_cursor_char(uint16_t* offset)
{
    // uint8_t message_num = read_bank_data(bank, (*offset)++);
    uint8_t message_num = read_heap((*offset)++);
    dprintf_args("\nset.cursor.char %d", message_num);
    game.cursor = get_message(message_num)[0];
}

void command_6d_set_text_attribute(uint16_t* offset)
{
    // uint8_t fg = read_bank_data(bank, (*offset)++);
    // uint8_t bg = read_bank_data(bank, (*offset)++);
    uint8_t fg = read_heap((*offset)++);
    uint8_t bg = read_heap((*offset)++);
    dprintf_args("\nset.text.attribute %d, %d", fg, bg);

    bgcolor(bg);
    textcolor(fg);
}

void command_6e_shake_screen(uint16_t* offset)
{
    printf("\nshake.screen");
}

void command_6f_configure_screen(uint16_t* offset)
{
    // uint8_t playtop = read_bank_data(bank, (*offset)++);
    //  uint8_t inputline = read_bank_data(bank, (*offset)++);
    //  uint8_t statusline = read_bank_data(bank, (*offset)++);
    uint8_t playtop = read_heap((*offset)++);
    uint8_t inputline = read_heap((*offset)++);
    uint8_t statusline = read_heap((*offset)++);
    dprintf_args("\nconfigure.screen %d, %d, %d", playtop, inputline, statusline);
    game.playtop = playtop;
    game.input_line = inputline;
    game.status_line = statusline;
}

void command_70_status_line_on(uint16_t* offset)
{
    dprintf("\nstatus.line.on");
    game.status_line_enabled = true;
}

void command_71_status_line_off(uint16_t* offset)
{
    dprintf("\nstatus.line.off");
    game.status_line_enabled = false;
}

void command_72_set_string(uint16_t* offset)
{
    // uint8_t strnum = read_bank_data(bank, (*offset)++);
    // uint8_t message = read_bank_data(bank, (*offset)++);
    uint8_t strnum = read_heap((*offset)++);
    uint8_t message = read_heap((*offset)++);
    dprintf_args("\nset.string %d, %d", strnum, message);
    strcpy(game.strings[strnum], get_message(message));
}
void command_73_get_string(uint16_t* offset) {}
void command_74_word_to_string(uint16_t* offset) {}
void command_75_parse(uint16_t* offset) {}
void command_76_get_num(uint16_t* offset) {}

void command_77_prevent_input(uint16_t* offset)
{
    dprintf("\nprevent.input");
    game.input_enabled = false;
    // todo: hide input line
}

void command_78_accept_input(uint16_t* offset)
{
    dprintf("\naccept.input");
    game.input_enabled = true;
    ++(*offset);
}

void command_79_set_key(uint16_t* offset)
{
    // uint8_t ascii = read_bank_data(bank, (*offset)++);
    // uint8_t scan = read_bank_data(bank, (*offset)++);
    // uint8_t controller = read_bank_data(bank, (*offset)++);
    uint8_t ascii = read_heap((*offset)++);
    uint8_t scan = read_heap((*offset)++);
    uint8_t controller = read_heap((*offset)++);
    dprintf_args("\nset.key %d, %d, %d", ascii, scan, controller);
    game.controller_assignments[controller].controller_number = controller;
    game.controller_assignments[controller].key_press_info = ascii;
    // todo: set the key press info for scan code
}

void command_7a_add_to_pic(uint16_t* offset) {}
void command_7b_add_to_pic_v(uint16_t* offset) {}
void command_7c_status(uint16_t* offset) {}
void command_7d_save_game(uint16_t* offset) {}
void command_7e_restore_game(uint16_t* offset) {}
void command_7f_init_disk(uint16_t* offset) {}
void command_80_restart_game(uint16_t* offset) {}
void command_81_show_obj(uint16_t* offset) {}
void command_82_random(uint16_t* offset) {}

void command_83_program_control(uint16_t* offset)
{
    dprintf("\nprogram.control");
    game.control_mode = PROGRAM_CONTROL;
}
void command_84_player_control(uint16_t* offset)
{
    dprintf("\nplayer.control");
    game.control_mode = PLAYER_CONTROL;
}
void command_85_obj_status_v(uint16_t* offset) {}
void command_86_quit(uint16_t* offset) {}
void command_87_show_mem(uint16_t* offset) {}
void command_88_pause(uint16_t* offset) {}
void command_89_echo_line(uint16_t* offset) {}
void command_8a_cancel_line(uint16_t* offset) {}
void command_8b_init_joy(uint16_t* offset) {}
void command_8c_toggle_monitor(uint16_t* offset) {}
void command_8d_version(uint16_t* offset) {}

void command_8e_script_size(uint16_t* offset)
{
    // uint8_t size = read_bank_data(bank, (*offset)++);
    uint8_t size = read_heap((*offset)++);
    dprintf_args("\nscript.size %d", size); // TODO: setup script table (for load/save games)
}

void command_8f_set_game_id(uint16_t* offset)
{
    // used for save games (eg KQ3SG.1), and also interpreter versions
    // uint8_t message_num = read_bank_data(bank, (*offset)++);
    uint8_t message_num = read_heap((*offset)++);
    dprintf_args("\nset.game.id %d", message_num);
    strcpy(game.id, get_message(message_num));
}

void command_90_log(uint16_t* offset) {}
void command_91_set_scan_start(uint16_t* offset) {}
void command_92_reset_scan_start(uint16_t* offset) {}
void command_93_reposition_to(uint16_t* offset) {}
void command_94_reposition_to_v(uint16_t* offset) {}
void command_95_trace_on(uint16_t* offset) {}

void command_96_trace_info(uint16_t* offset)
{
    // uint8_t lognum = read_bank_data(bank, (*offset)++);
    // uint8_t top = read_bank_data(bank, (*offset)++);
    // uint8_t height = read_bank_data(bank, (*offset)++);
    uint8_t lognum = read_heap((*offset)++);
    uint8_t top = read_heap((*offset)++);
    uint8_t height = read_heap((*offset)++);
    dprintf_args("\ntrace.info %d, %d, %d", lognum, top, height);
    // TODO: trace.info
}

void command_97_print_at(uint16_t* offset) {}
void command_98_print_at_v(uint16_t* offset) {}

void command_99_discard_view_v(uint16_t* offset)
{
    // uint8_t var = read_bank_data(bank, (*offset)++);
    uint8_t var = read_heap((*offset)++);
    dprintf_args("\ndiscard.view.v %d", var);
    unload_resource(viewdir, game.vars[var]);
}

void command_9a_clear_text_rect(uint16_t* offset) {}
void command_9b_set_upper_left(uint16_t* offset) {}

void command_9c_set_menu(uint16_t* offset)
{
    // TODO: this is probably slow there's probably a better way to do it
    uint8_t message;
    Menu* new_menu;
    Menu* temp;

    // message = read_bank_data(bank, (*offset)++);
    message = read_heap((*offset)++);

    new_menu = (Menu*)malloc(sizeof(Menu));
    if (new_menu) {
        new_menu->title = get_message(message);
        new_menu->items = NULL;
        new_menu->next = NULL;

        if (game.menus == NULL) {
            game.menus = new_menu;
        } else {
            temp = game.menus;
            while (temp->next != NULL) {
                temp = temp->next;
            }
            temp->next = new_menu;
        }
        menu = new_menu;
    }
}

void command_9d_set_menu_item(uint16_t* offset)
{
    uint8_t message, controller;
    MenuItem* new_menu_item;
    MenuItem* temp;

    // message = read_bank_data(bank, (*offset)++);
    // controller = read_bank_data(bank, (*offset)++);
    message = read_heap((*offset)++);
    controller = read_heap((*offset)++);
    dprintf_args("\nmenu.item message %d, controller %d", message, controller);

    new_menu_item = (MenuItem*)malloc(sizeof(MenuItem));
    if (new_menu_item) {
        new_menu_item->text = get_message(message);
        new_menu_item->controller = controller;
        new_menu_item->next = NULL;

        if (menu && menu->items == NULL) {
            menu->items = new_menu_item;
        } else if (menu) {
            temp = menu->items;
            while (temp->next != NULL) {
                temp = temp->next;
            }
            temp->next = new_menu_item;
        }
    }
}

void command_9e_submit_menu(uint16_t* offset)
{
    dprintf("\nsubmit.menu");
    // TODO: setup menus
}

void command_9f_enable_item(uint16_t* offset)
{
    MenuItem* item;
    // uint8_t controller = read_bank_data(bank, (*offset)++);
    uint8_t controller = read_heap((*offset)++);
    dprintf_args("\nenable.item %d", controller);
    // go through the menu and enable any items with this controller set
    for (item = menu->items; item != NULL; item = item->next) {
        if (item->controller == controller) {
            item->enabled = true;
        }
    }
}

void command_a0_disable_item(uint16_t* offset)
{
    MenuItem* item;
    // uint8_t controller = read_bank_data(bank, (*offset)++);
    uint8_t controller = read_heap((*offset)++);
    dprintf_args("\ndisable.item %d", controller);
    // go through the menu and disable any items with this controller set
    for (item = menu->items; item != NULL; item = item->next) {
        if (item->controller == controller) {
            item->enabled = false;
        }
    }
}

void command_a1_menu_input(uint16_t* offset) {}
void command_a2_show_obj_v(uint16_t* offset) {}
void command_a3_open_dialogue(uint16_t* offset) {}
void command_a4_close_dialogue(uint16_t* offset) {}
void command_a5_mul_n(uint16_t* offset) {}
void command_a6_mul_v(uint16_t* offset) {}
void command_a7_div_n(uint16_t* offset) {}
void command_a8_div_v(uint16_t* offset) {}
void command_a9_close_window(uint16_t* offset) {}
void command_aa_set_simple(uint16_t* offset) {}
void command_ab_push_script(uint16_t* offset) {}
void command_ac_pop_script(uint16_t* offset) {}
void command_ad_hold_key(uint16_t* offset) {}
void command_ae_set_pri_base(uint16_t* offset) {}

void command_af_discard_sound(uint16_t* offset)
{
    // uint8_t sound_num = read_bank_data(bank, (*offset)++);
    uint8_t sound_num = read_heap((*offset)++);
    dprintf_args("\ndiscard.sound %d", sound_num);
    unload_resource(snddir, sound_num);
}

void command_b0_hide_mouse(uint16_t* offset) {}
void command_b1_allow_menu(uint16_t* offset) {}
void command_b2_show_mouse(uint16_t* offset) {}
void command_b3_fence_mouse(uint16_t* offset) {}
void command_b4_mouse_posn(uint16_t* offset) {}
void command_b5_release_key(uint16_t* offset) {}
void command_b6_adj_ego_move_to_x_y(uint16_t* offset) {}