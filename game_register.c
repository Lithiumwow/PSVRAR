/*
 * PS5 Game Registration Implementation
 * 
 * Auto-registers extracted games so they appear in PS5 library
 * Uses libSceAppInstUtil system library (PS5's official app installer)
 */

#include "game_register.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <dlfcn.h>

// PS5 libSceAppInstUtil function pointers - for directory-based game registration
typedef int (*sceAppInstUtilInitialize_t)(void);
typedef int (*sceAppInstUtilAppInstallTitleDir_t)(const char *title_dir, void *reserved);
typedef int (*sceAppInstUtilAppExists_t)(const char *title_id, int *exists);
typedef int (*sceAppInstUtilTerminate_t)(void);

static void *appinst_module = NULL;
static sceAppInstUtilInitialize_t sceAppInstUtilInitialize = NULL;
static sceAppInstUtilAppInstallTitleDir_t sceAppInstUtilAppInstallTitleDir = NULL;
static sceAppInstUtilAppExists_t sceAppInstUtilAppExists = NULL;
static sceAppInstUtilTerminate_t sceAppInstUtilTerminate = NULL;

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

// Initialize libSceAppInstUtil module - loads the system library for game registration
// Returns 0 on success, -1 on failure (non-critical - symlinks still work)
static int init_appinst_module(void) {
    if (appinst_module) {
        return 0; // Already initialized
    }
    
    fprintf(stdout, "[REGISTER] Attempting to load libSceAppInstUtil.sprx...\n");
    
    // Try to load the library - this may fail without privileges, which is OK
    appinst_module = dlopen("/system/common/lib/libSceAppInstUtil.sprx", RTLD_LAZY | RTLD_LOCAL);
    if (!appinst_module) {
        const char *err = dlerror();
        fprintf(stderr, "[REGISTER] ⚠️  Cannot load libSceAppInstUtil: %s\n", err ? err : "unknown error");
        fprintf(stdout, "[REGISTER] ℹ️  Continuing with symlink-only mode (Itemzflow-compatible)\n");
        return -1;
    }
    
    // Load function pointers
    sceAppInstUtilInitialize = (sceAppInstUtilInitialize_t)dlsym(appinst_module, "sceAppInstUtilInitialize");
    sceAppInstUtilAppInstallTitleDir = (sceAppInstUtilAppInstallTitleDir_t)dlsym(appinst_module, "sceAppInstUtilAppInstallTitleDir");
    sceAppInstUtilAppExists = (sceAppInstUtilAppExists_t)dlsym(appinst_module, "sceAppInstUtilAppExists");
    sceAppInstUtilTerminate = (sceAppInstUtilTerminate_t)dlsym(appinst_module, "sceAppInstUtilTerminate");
    
    if (!sceAppInstUtilInitialize || !sceAppInstUtilAppInstallTitleDir) {
        fprintf(stderr, "[REGISTER] ⚠️  Required functions not found in library\n");
        fprintf(stdout, "[REGISTER] ℹ️  Continuing with symlink-only mode\n");
        dlclose(appinst_module);
        appinst_module = NULL;
        return -1;
    }
    
    // Initialize the library
    fprintf(stdout, "[REGISTER] Initializing library...\n");
    int ret = sceAppInstUtilInitialize();
    if (ret != 0) {
        fprintf(stderr, "[REGISTER] ⚠️  Library initialization failed (0x%08X)\n", ret);
        fprintf(stdout, "[REGISTER] ℹ️  Continuing with symlink-only mode\n");
        dlclose(appinst_module);
        appinst_module = NULL;
        return -1;
    }
    
    fprintf(stdout, "[REGISTER] ✅ Library loaded successfully\n");
    return 0;
}

// Register game using HYBRID approach: symlink + optional API
// Works in SYMLINK-ONLY mode (like Itemzflow) if API is unavailable
int register_game(const char *game_path, const ps5_param_t *param) {
    char link_path[PATH_MAX];
    char target_path[PATH_MAX];
    char appmeta_path[PATH_MAX];
    
    fprintf(stdout, "\n[REGISTER] ═══════════════════════════════════════════════════\n");
    fprintf(stdout, "[REGISTER] Registering: %s (%s)\n", param->title, param->title_id);
    fprintf(stdout, "[REGISTER] Source: %s\n", game_path);
    fprintf(stdout, "[REGISTER] ═══════════════════════════════════════════════════\n");
    
    // STEP 1: Create symlink in /system_data (CRITICAL - this is what makes it work!)
    snprintf(link_path, sizeof(link_path), "/system_data/priv/mnt/app/%s", param->title_id);
    snprintf(target_path, sizeof(target_path), "%s", game_path);
    snprintf(appmeta_path, sizeof(appmeta_path), "/system_data/priv/appmeta/%s", param->title_id);
    
    fprintf(stdout, "[REGISTER] Creating mount point...\n");
    fprintf(stdout, "[REGISTER]   %s -> %s\n", link_path, target_path);
    
    // Create parent directories
    mkdir("/system_data", 0777);
    mkdir("/system_data/priv", 0777);
    mkdir("/system_data/priv/mnt", 0777);
    mkdir("/system_data/priv/mnt/app", 0777);
    mkdir("/system_data/priv/appmeta", 0777);
    
    // Check if symlink already exists
    struct stat st;
    int link_exists = (lstat(link_path, &st) == 0);
    
    if (!link_exists) {
        if (symlink(target_path, link_path) != 0) {
            fprintf(stderr, "[REGISTER] ❌ ERROR: Cannot create symlink (errno=%d: %s)\n", errno, strerror(errno));
            fprintf(stderr, "[REGISTER] 💡 SOLUTION: Load ps5-kstuff.elf first, then retry\n");
            fprintf(stdout, "[REGISTER] ═══════════════════════════════════════════════════\n\n");
            return -1;
        }
        fprintf(stdout, "[REGISTER] ✅ Game mounted at %s\n", link_path);
    } else {
        fprintf(stdout, "[REGISTER] ℹ️  Already mounted\n");
    }
    
    // STEP 2: Create appmeta symlink (for game metadata)
    char appmeta_target[PATH_MAX];
    snprintf(appmeta_target, sizeof(appmeta_target), "%s/sce_sys", game_path);
    
    fprintf(stdout, "[REGISTER] Creating metadata link...\n");
    fprintf(stdout, "[REGISTER]   %s -> %s\n", appmeta_path, appmeta_target);
    
    link_exists = (lstat(appmeta_path, &st) == 0);
    if (!link_exists) {
        if (symlink(appmeta_target, appmeta_path) != 0) {
            fprintf(stderr, "[REGISTER] ⚠️  Warning: Metadata symlink failed (errno=%d)\n", errno);
        } else {
            fprintf(stdout, "[REGISTER] ✅ Metadata linked\n");
        }
    } else {
        fprintf(stdout, "[REGISTER] ℹ️  Metadata already linked\n");
    }
    
    // STEP 3: Trigger library refresh using system restart
    fprintf(stdout, "[REGISTER] Triggering library refresh...\n");
    
    // Method 1: Send notification reminder
    char notify_msg[256];
    snprintf(notify_msg, sizeof(notify_msg), "✅ %s ready!", param->title);
    send_notification(notify_msg);
    
    // Method 2: Try to restart ShellUI process (forces complete refresh)
    void *system_lib = dlopen("/system/common/lib/libSceSystemService.sprx", RTLD_LAZY | RTLD_LOCAL);
    if (system_lib) {
        // Try to get the function that restarts ShellUI
        typedef int (*sceSystemServiceLaunchApp_t)(const char *title_id, char **args, void *event);
        sceSystemServiceLaunchApp_t sceSystemServiceLaunchApp = 
            (sceSystemServiceLaunchApp_t)dlsym(system_lib, "sceSystemServiceLaunchApp");
        
        if (sceSystemServiceLaunchApp) {
            // Launch the PS5 home menu app to force a refresh
            // This is equivalent to what the Triangle button does
            int ret = sceSystemServiceLaunchApp("NPXS20001", NULL, NULL);  // Home menu title ID
            if (ret == 0) {
                fprintf(stdout, "[REGISTER] ✅ Home menu refresh triggered\n");
                fprintf(stdout, "[REGISTER] 🎮 Game should appear in library now!\n");
            } else {
                fprintf(stdout, "[REGISTER] ⚠️  Auto-refresh failed (0x%08X)\n", ret);
                fprintf(stdout, "[REGISTER] 💡 Open Itemzflow and use its 'Refresh' option\n");
            }
        }
        dlclose(system_lib);
    }
    
    fprintf(stdout, "[REGISTER] ℹ️  If game doesn't appear:\n");
    fprintf(stdout, "[REGISTER]    1. Open Itemzflow → Use 'Refresh Games' option\n");
    fprintf(stdout, "[REGISTER]    2. Settings → Rebuild Database\n");
    
    // STEP 4: Try to register with PS5 database (optional fallback)
    fprintf(stdout, "[REGISTER] Attempting database registration...\n");
    
    int api_result = init_appinst_module();
    if (api_result == 0 && sceAppInstUtilAppExists) {
        // Check if already registered
        int exists = 0;
        int ret = sceAppInstUtilAppExists(param->title_id, &exists);
        if (ret == 0 && exists) {
            fprintf(stdout, "[REGISTER] ✅ Already in database\n");
            fprintf(stdout, "[REGISTER] 🎮 Game is ready to launch!\n");
            fprintf(stdout, "[REGISTER] ═══════════════════════════════════════════════════\n\n");
            return 0;
        }
        
        // Register using the SYMLINKED path
        ret = sceAppInstUtilAppInstallTitleDir(link_path, NULL);
        if (ret == 0) {
            fprintf(stdout, "[REGISTER] ✅ Registered in database\n");
            fprintf(stdout, "[REGISTER] 🎮 Game is ready to launch!\n");
        } else {
            fprintf(stdout, "[REGISTER] ⚠️  Database registration failed (0x%08X)\n", ret);
            fprintf(stdout, "[REGISTER] 💡 Use Itemzflow to scan and add to library\n");
        }
    } else {
        fprintf(stdout, "[REGISTER] ⚠️  API unavailable (needs privileges)\n");
        fprintf(stdout, "[REGISTER] ✅ Game symlinks created successfully!\n");
        fprintf(stdout, "[REGISTER] \n");
        fprintf(stdout, "[REGISTER] 🎮 TO MAKE GAME APPEAR:\n");
        fprintf(stdout, "[REGISTER]    Open Itemzflow → Select 'Refresh Games'\n");
        fprintf(stdout, "[REGISTER]    → Game appears in Itemzflow immediately!\n");
        fprintf(stdout, "[REGISTER]    → Also appears in PS5 home menu\n");
        fprintf(stdout, "[REGISTER] \n");
        fprintf(stdout, "[REGISTER] 💡 Alternative: Settings > Rebuild Database\n");
    }
    
    fprintf(stdout, "[REGISTER] ═══════════════════════════════════════════════════\n\n");
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
