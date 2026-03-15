/*
 * PS5 Game Registration Helper
 * 
 * Reads param.json and registers games so they appear in PS5 library
 * Compatible with ShadowMountPlus workflow
 */

#ifndef GAME_REGISTER_H
#define GAME_REGISTER_H

#include <stdint.h>

// PS5 param.json structure (subset of fields we need)
typedef struct {
    char title_id[16];           // e.g., "PPSA12345"
    char title[256];             // Game title
    char content_id[64];         // Content ID
    char category[16];           // "gd" = game disc, "gp" = game patch, etc.
    uint32_t app_version;        // Application version
} ps5_param_t;

// Parse param.json from extracted game directory
int parse_param_json(const char *game_dir, ps5_param_t *param);

// Register game with PS5 system (makes it appear in library)
int register_game(const char *game_path, const ps5_param_t *param);

// Unregister game (cleanup)
int unregister_game(const char *title_id);

// Check if game is already registered
int is_game_registered(const char *title_id);

// Send notification to PS5 (from main.c)
void send_notification(const char *message);

#endif // GAME_REGISTER_H
