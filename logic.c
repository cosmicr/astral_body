#include <conio.h>
#include <string.h>
#include <cx16.h>

#include "asm.h"
#include "bank.h"
#include "interp.h"
#include "logic.h"
#include "logic_commands.h"
#include "logic_tests.h"
#include "resource.h"
#include "dprint.h"

// Keep track of the current RAM bank for logic data
uint8_t bank;

static uint16_t messages_offset;

// Test functions pointer table
bool (*test_functions[0x12])(uint16_t* offset) = {test_equaln, test_equalv,
                                                  test_lessn, test_lessv,
                                                  test_greatern, test_greaterv,
                                                  test_isset, test_issetv,
                                                  test_has,
                                                  test_obj_in_room,
                                                  test_posn,
                                                  test_controller,
                                                  test_have_key,
                                                  test_said,
                                                  test_compare_strings,
                                                  test_obj_in_box,
                                                  test_center_posn, test_right_posn};

// Logic commands pointer table
void (*command_functions[0xB7])(uint16_t* offset) = {command_00_return,
                                                     command_01_increment, command_02_decrement,
                                                     command_03_assignn, command_04_assignv,
                                                     command_05_addn, command_06_addv,
                                                     command_07_subn, command_08_subv,
                                                     command_09_lindirectv, command_0a_rindirect, command_0b_lindirectn,
                                                     command_0c_set, command_0d_reset, command_0e_toggle,
                                                     command_0f_set_v, command_10_reset_v, command_11_toggle_v,
                                                     command_12_new_room, command_13_new_room_v,
                                                     command_14_load_logics, command_15_load_logics_v,
                                                     command_16_call, command_17_call_v,
                                                     command_18_load_pic, command_19_draw_pic, command_1a_show_pic, command_1b_discard_pic, command_1c_overlay_pic,
                                                     command_1d_show_pri_screen,
                                                     command_1e_load_view, command_1f_load_view_v, command_20_discard_view,
                                                     command_21_animate_obj, command_22_unanimate_all,
                                                     command_23_draw, command_24_erase,
                                                     command_25_position, command_26_position_v,
                                                     command_27_get_posn, command_28_reposition,
                                                     command_29_set_view, command_2a_set_view_v,
                                                     command_2b_set_loop, command_2c_set_loop_v,
                                                     command_2d_fix_loop, command_2e_release_loop,
                                                     command_2f_set_cel, command_30_set_cel_v,
                                                     command_31_last_cel, command_32_current_cel, command_33_current_loop, command_34_current_view,
                                                     command_35_number_of_loops,
                                                     command_36_set_priority, command_37_set_priority_v,
                                                     command_38_release_priority, command_39_get_priority,
                                                     command_3a_stop_update, command_3b_start_update, command_3c_force_update,
                                                     command_3d_ignore_horizon, command_3e_observe_horizon, command_3f_set_horizon,
                                                     command_40_object_on_water, command_41_object_on_land, command_42_object_on_anything,
                                                     command_43_ignore_objs, command_44_observe_objs,
                                                     command_45_distance,
                                                     command_46_stop_cycling, command_47_start_cycling,
                                                     command_48_normal_cycle,
                                                     command_49_end_of_loop,
                                                     command_4a_reverse_cycle,
                                                     command_4b_reverse_loop,
                                                     command_4c_cycle_time,
                                                     command_4d_stop_motion, command_4e_start_motion,
                                                     command_4f_step_size, command_50_step_time,
                                                     command_51_move_obj, command_52_move_obj_v,
                                                     command_53_follow_ego, command_54_wander, command_55_normal_motion,
                                                     command_56_set_dir, command_57_get_dir,
                                                     command_58_ignore_blocks, command_59_observe_blocks,
                                                     command_5a_block, command_5b_unblock,
                                                     command_5c_get, command_5d_get_v,
                                                     command_5e_drop,
                                                     command_5f_put, command_60_put_v,
                                                     command_61_get_room_v,
                                                     command_62_load_sound, command_63_sound, command_64_stop_sound,
                                                     command_65_print, command_66_print_v,
                                                     command_67_display, command_68_display_v,
                                                     command_69_clear_lines,
                                                     command_6a_text_screen,
                                                     command_6b_graphics,
                                                     command_6c_set_cursor_char,
                                                     command_6d_set_text_attribute,
                                                     command_6e_shake_screen,
                                                     command_6f_configure_screen,
                                                     command_70_status_line_on, command_71_status_line_off,
                                                     command_72_set_string, command_73_get_string,
                                                     command_74_word_to_string,
                                                     command_75_parse,
                                                     command_76_get_num,
                                                     command_77_prevent_input, command_78_accept_input,
                                                     command_79_set_key,
                                                     command_7a_add_to_pic, command_7b_add_to_pic_v,
                                                     command_7c_status,
                                                     command_7d_save_game, command_7e_restore_game,
                                                     command_7f_init_disk,
                                                     command_80_restart_game,
                                                     command_81_show_obj,
                                                     command_82_random,
                                                     command_83_program_control, command_84_player_control,
                                                     command_85_obj_status_v,
                                                     command_86_quit,
                                                     command_87_show_mem,
                                                     command_88_pause,
                                                     command_89_echo_line, command_8a_cancel_line,
                                                     command_8b_init_joy,
                                                     command_8c_toggle_monitor,
                                                     command_8d_version,
                                                     command_8e_script_size,
                                                     command_8f_set_game_id,
                                                     command_90_log,
                                                     command_91_set_scan_start, command_92_reset_scan_start,
                                                     command_93_reposition_to, command_94_reposition_to_v,
                                                     command_95_trace_on, command_96_trace_info,
                                                     command_97_print_at, command_98_print_at_v,
                                                     command_99_discard_view_v,
                                                     command_9a_clear_text_rect,
                                                     command_9b_set_upper_left,
                                                     command_9c_set_menu, command_9d_set_menu_item,
                                                     command_9e_submit_menu,
                                                     command_9f_enable_item, command_a0_disable_item,
                                                     command_a1_menu_input,
                                                     command_a2_show_obj_v,
                                                     command_a3_open_dialogue, command_a4_close_dialogue,
                                                     command_a5_mul_n, command_a6_mul_v,
                                                     command_a7_div_n, command_a8_div_v,
                                                     command_a9_close_window,
                                                     command_aa_set_simple,
                                                     command_ab_push_script, command_ac_pop_script,
                                                     command_ad_hold_key,
                                                     command_ae_set_pri_base,
                                                     command_af_discard_sound,
                                                     command_b0_hide_mouse,
                                                     command_b1_allow_menu,
                                                     command_b2_show_mouse, command_b3_fence_mouse, command_b4_mouse_posn,
                                                     command_b5_release_key,
                                                     command_b6_adj_ego_move_to_x_y};

// gets the next two bytes and then moves offset forward by that amount
void advance(uint16_t* offset)
{
    uint16_t value = read_bank_data(bank, *offset) | (read_bank_data(bank, *offset + 1) << 8);
    dprintf_args("\nadvance from offset: %X", *offset);
    *offset += value + 2;
    dprintf_args("\nskipping %d", value);
    dprintf_args(" new offset: %X", *offset);
}

void print_opcode(uint8_t code, uint16_t offset, char* str)
{
    dprintf_args("\n%X: %X %s", offset, code, str);
}

void execute_logic(uint8_t num)
{
    static uint8_t code;
    static uint16_t offset = 0;

    if (num == 0) {
        bank = LOGIC0_BANK;
    } else {
        bank = LOGIC_BANK;
    }

    RAM_BANK = bank;

    messages_offset = read_bank_data(bank, offset) | (read_bank_data(bank, offset + 1) << 8);
    messages_offset += 2;
    offset += 2;

    decrypt_messages(); // TODO: this should happen when logic is loaded

    // note: logic data does not have header or size - it starts straight away
    while (offset < messages_offset) {
        code = read_bank_data(bank, offset);
        printf("\n(%d)%X: %X ", num, offset, code);
        if (code == 0xff) { // if
            execute_if(&offset);
        } else if (code == 0xfe) { // else/goto
            print_opcode(code, offset, "else");
            offset++;
            advance(&offset);
        } else { // command
            print_opcode(code, offset, "command");
            execute_command(code, &offset);
        }
        // while (!kbhit())
        //     ;
        // cgetc();
        asm_clear_overlay();
        gotoxy(0, 1);
    }
}

bool execute_test(uint8_t code, uint16_t* offset)
{
    bool result;

    (*offset)++;
    result = test_functions[code - 1](offset);
    dprintf_args(" = %s\n", result ? "true" : "false");
    return result;
}

void execute_if(uint16_t* offset)
{
    uint8_t code;
    bool not_mode = 0;
    bool or_mode = 0;
    uint8_t result;

    print_opcode(0xFF, *offset, "if");

    // advance the offset
    (*offset)++;

    while (true) {
        code = read_bank_data(bank, *offset); // get test
        dprintf_args("\ncode is %X", code);
        if (code < 0xFC) // not a conditional
        {
            bool next_result;
            print_opcode(code, *offset, "(test)");
            next_result = execute_test(code, offset);
            if (not_mode) {
                next_result = !next_result;
                not_mode = false;
            }
            if (or_mode) {
                result = result || next_result;
            } else {
                result = result && next_result;
            }
        } else if (code == 0xFD) // not
        {
            print_opcode(code, *offset, "(not)");
            not_mode = !not_mode;
            (*offset)++;
        } else if (code == 0xFC) // or
        {
            print_opcode(code, *offset, "(or)");
            or_mode = true;
            (*offset)++;
        } else if (code == 0xFE) // else/goto?
        {
            print_opcode(code, *offset, "(else)");
            result = false;
            (*offset)++;
            break;
        } else if (code == 0xFF) // end of if
        {
            print_opcode(code, *offset, "endif");
            (*offset)++;
            break;
        }
    }
    if (result) {
        dprintf("\nif result is true");
        //  add 2 bytes to offset to skip the goto
        (*offset) += 2;
    } else {
        dprintf("\nif result is false");
        advance(offset);
    }
}

void execute_command(uint8_t code, uint16_t* offset)
{
    // TODO: a lot of commands and tests could be done in assembly?
    dprintf_args("\n%X command 0x%X ", *offset, code);
    (*offset)++;

    command_functions[code](offset);
}

char* get_message(uint8_t message_num)
{
    uint8_t num_messages;
    uint16_t message_offset;
    size_t message_length = 0;
    char* message = NULL;
    int i = 0;
    uint8_t c;

    // Read and validate the number of messages
    // num_messages = read_bank_data(bank, messages_offset);
    // if (message_num >= num_messages) {
    //     return NULL; // Message number out of range
    // }

    message_num -= 1; // Decrement message number to account for 1-based indexing

    message_offset = (read_bank_data(bank, message_num * 2 + messages_offset + 3) | (read_bank_data(bank, message_num * 2 + messages_offset + 4) << 8)) + messages_offset + 1;

    // read characters into message until we hit a null terminator
    while ((c = read_bank_data(bank, message_offset + i)) != 0) {
        char* temp = realloc(message, i + 2);
        if (temp == NULL) {
            free(message);
            return NULL; // Memory allocation failed
        }
        message = temp;
        message[i] = c;
        ++i;
    }

    if (message != NULL) {
        message[i] = '\0'; // Null terminate the string
        printf("\nMessage: %s", message);
    }
    else
    {
        dprintf("\nMessage: NULL");
    }

    return message;
}

void decrypt_messages()
{
    // Hard-coded array of ASCII hex values for "Avis Durgan"
    const char key[] = {0x41, 0x76, 0x69, 0x73, 0x20, 0x44, 0x75, 0x72, 0x67, 0x61, 0x6E, 0x00};
    size_t key_len = strlen(key);
    static uint16_t messages_size;
    static uint16_t text_start;
    int i = 0;

    messages_size = read_bank_data(bank, messages_offset + 1) | (read_bank_data(bank, messages_offset + 2) << 8);
    // printf("\nMessages size is %X", messages_size);

    text_start = read_bank_data(bank, messages_offset + 3) | (read_bank_data(bank, messages_offset + 4) << 8);
    // printf("\nText start is %X", text_start);
    text_start += messages_offset + 1;
    // printf("\nText start is %X", text_start);

    // Decrypt the text
    for (i = 0; i < messages_size; i++) {
        uint8_t c = read_bank_data(bank, text_start + i);
        c ^= key[i % key_len];

        // invert the case of the petscii character
        if (c >= 0x41 && c <= 0x5A) {
            c += 0x20;
        } else if (c >= 0x61 && c <= 0x7A) {
            c -= 0x20;
        }

        // set the new memory value to the decrypted value
        write_bank_data(bank, text_start + i, c);
    }
}