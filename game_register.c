/*
 * PS5 Game Registration Implementation
 * 
 * Auto-registers extracted games so they appear in PS5 library
 * Uses same approach as ShadowMountPlus and Dump Runner
 */

#include "game_register.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

// JSON parsing (simple implementation for param.json)
static char* find_json_value(const char *json, const char *key) {
    char search[256];
    snprintf(search, sizeof(search), "\"%s\"", key);
    
    char *pos = strstr(json, search);
    if (!pos) return NULL;
    
    pos = strchr(pos + strlen(search), ':');
    if (!pos) return NULL;
    
    while (*pos && (*pos == ':' || *pos == ' ' || *pos == '\t')) pos++;
    
    if (*pos == '"') {
        pos++;
        char *end = strchr(pos, '"');
        if (!end) return NULL;
        
        size_t len = end - pos;
        char *value = malloc(len + 1);
        if (!value) return NULL;
        
        memcpy(value, pos, len);
        value[len] = '\0';
        return value;
    }
    
    // Numeric value
    char *end = pos;
    while (*end && *end != ',' && *end != '}' && *end != '\n') end++;
    
    size_t len = end - pos;
    char *value = malloc(len + 1);
    if (!value) return NULL;
    
    memcpy(value, pos, len);
    value[len] = '\0';
    return value;
}

int parse_param_json(const char *game_dir, ps5_param_t *param) {
    char param_path[PATH_MAX];
    snprintf(param_path, sizeof(param_path), "%s/sce_sys/param.json", game_dir);
    
    FILE *fp = fopen(param_path, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open %s\n", param_path);
        return -1;
    }
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char *json = malloc(size + 1);
    if (!json) {
        fclose(fp);
        return -1;
    }
    
    fread(json, 1, size, fp);
    json[size] = '\0';
    fclose(fp);
    
    memset(param, 0, sizeof(*param));
    
    // Extract fields
    char *title_id = find_json_value(json, "titleId");
    if (title_id) {
        strncpy(param->title_id, title_id, sizeof(param->title_id) - 1);
        free(title_id);
    }
    
    char *title = find_json_value(json, "localizedParameters");
    if (!title) {
        title = find_json_value(json, "defaultLanguage");
    }
    if (title) {
        // Look for title within localizedParameters
        char *title_val = find_json_value(title, "title");
        if (title_val) {
            strncpy(param->title, title_val, sizeof(param->title) - 1);
            free(title_val);
        }
        free(title);
    }
    
    char *content_id = find_json_value(json, "contentId");
    if (content_id) {
        strncpy(param->content_id, content_id, sizeof(param->content_id) - 1);
        free(content_id);
    }
    
    char *category = find_json_value(json, "category");
    if (category) {
        strncpy(param->category, category, sizeof(param->category) - 1);
        free(category);
    }
    
    char *version = find_json_value(json, "contentVersion");
    if (version) {
        param->app_version = atoi(version);
        free(version);
    }
    
    free(json);
    
    if (param->title_id[0] == '\0') {
        fprintf(stderr, "Failed to parse titleId from param.json\n");
        return -1;
    }
    
    return 0;
}

// Create symbolic link in /system_data/priv/appmeta for game registration
// This is how ShadowMountPlus makes games visible
int register_game(const char *game_path, const ps5_param_t *param) {
    char link_path[PATH_MAX];
    char target_path[PATH_MAX];
    
    // Create appmeta link (similar to ShadowMountPlus approach)
    snprintf(link_path, sizeof(link_path), "/system_data/priv/appmeta/%s", param->title_id);
    snprintf(target_path, sizeof(target_path), "%s/sce_sys", game_path);
    
    struct stat st;
    if (lstat(link_path, &st) == 0) {
        fprintf(stdout, "Game %s already registered\n", param->title_id);
        return 0;
    }
    
    // Create symbolic link
    if (symlink(target_path, link_path) != 0) {
        fprintf(stderr, "Failed to create registration link for %s\n", param->title_id);
        return -1;
    }
    
    fprintf(stdout, "Registered game: %s (%s)\n", param->title, param->title_id);
    return 0;
}

int unregister_game(const char *title_id) {
    char link_path[PATH_MAX];
    snprintf(link_path, sizeof(link_path), "/system_data/priv/appmeta/%s", title_id);
    
    if (unlink(link_path) != 0) {
        return -1;
    }
    
    return 0;
}

int is_game_registered(const char *title_id) {
    char link_path[PATH_MAX];
    struct stat st;
    
    snprintf(link_path, sizeof(link_path), "/system_data/priv/appmeta/%s", title_id);
    return (lstat(link_path, &st) == 0);
}

// Scan directory for param.json and auto-register
int auto_register_game(const char *dir_path) {
    char param_path[PATH_MAX];
    ps5_param_t param;
    
    snprintf(param_path, sizeof(param_path), "%s/sce_sys/param.json", dir_path);
    
    struct stat st;
    if (stat(param_path, &st) != 0) {
        // Not a game directory
        return -1;
    }
    
    if (parse_param_json(dir_path, &param) != 0) {
        fprintf(stderr, "Failed to parse param.json in %s\n", dir_path);
        return -1;
    }
    
    if (is_game_registered(param.title_id)) {
        fprintf(stdout, "Game %s already registered, skipping\n", param.title_id);
        return 0;
    }
    
    return register_game(dir_path, &param);
}
