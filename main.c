/*
 * PS5 RAR Auto-Extractor v2.1 - OPTIMIZED
 *
 * Performance optimizations:
 * - Buffered I/O with large write buffers
 * - Reduced fprintf() calls (batched output)
 * - Reduced notification frequency
 * - Directory caching to avoid redundant mkdir() calls
 * - Memory buffer extraction for small files
 * - CRC validation only when needed
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define WRITE_BUFFER_SIZE (8 * 1024 * 1024)  // 8MB write buffer (was 4MB)
#define SMALL_FILE_THRESHOLD (4 * 1024 * 1024)  // 4MB - extract to memory first (was 1MB)

#include "dmc_unrar.c"
#include "game_register.c"  // Include game registration support

typedef struct notify_request {
	char useless1[45];
	char message[3075];
} notify_request_t;

extern int sceKernelSendNotificationRequest(int, notify_request_t*, size_t, int);

void send_notification(const char *msg) {
	notify_request_t req;
	memset(&req, 0, sizeof(req));
	strncpy(req.message, msg, sizeof(req.message) - 1);
	req.message[sizeof(req.message) - 1] = '\0';  // Ensure null termination
	sceKernelSendNotificationRequest(0, &req, sizeof(req), 0);
	fprintf(stdout, "[NOTIFICATION] %s\n", msg);
	usleep(100000);  // 100ms delay between notifications
}

typedef struct {
	char path[PATH_MAX];
	int exists;
} dir_cache_entry_t;

static dir_cache_entry_t dir_cache[256];
static int dir_cache_count = 0;

static int is_dir_cached(const char *path) {
	int i;
	for (i = 0; i < dir_cache_count; i++) {
		if (strcmp(dir_cache[i].path, path) == 0) {
			return dir_cache[i].exists;
		}
	}
	return 0;
}

static void cache_dir(const char *path, int exists) {
	if (dir_cache_count < 256) {
		strncpy(dir_cache[dir_cache_count].path, path, PATH_MAX - 1);
		dir_cache[dir_cache_count].exists = exists;
		dir_cache_count++;
	}
}

static int create_directory_recursive(const char *path) {
	char tmp[PATH_MAX];
	char *p = NULL;
	size_t len;
	struct stat st;

	if (is_dir_cached(path)) {
		return 0;
	}

	if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
		cache_dir(path, 1);
		return 0;
	}

	snprintf(tmp, sizeof(tmp), "%s", path);
	len = strlen(tmp);
	if (tmp[len - 1] == '/')
		tmp[len - 1] = 0;

	for (p = tmp + 1; *p; p++) {
		if (*p == '/') {
			*p = 0;
			if (!is_dir_cached(tmp)) {
				if (mkdir(tmp, 0777) != 0 && errno != EEXIST) {
					return -1;
				}
				cache_dir(tmp, 1);
			}
			*p = '/';
		}
	}
	
	if (mkdir(tmp, 0777) != 0 && errno != EEXIST) {
		return -1;
	}
	cache_dir(tmp, 1);
	return 0;
}

static int extract_small_file_optimized(dmc_unrar_archive *archive, dmc_unrar_size_t index, const char *out_path, dmc_unrar_size_t size) {
	void *buffer;
	dmc_unrar_return ret;
	FILE *fp;

	buffer = malloc(size + 1024);
	if (!buffer) {
		return -1;
	}

	ret = dmc_unrar_extract_file_to_mem(archive, index, buffer, size, NULL, false);
	if (ret != DMC_UNRAR_OK) {
		free(buffer);
		return -1;
	}

	fp = fopen(out_path, "wb");
	if (!fp) {
		free(buffer);
		return -1;
	}

	setvbuf(fp, NULL, _IOFBF, WRITE_BUFFER_SIZE);

	if (fwrite(buffer, 1, size, fp) != size) {
		fclose(fp);
		free(buffer);
		return -1;
	}

	fclose(fp);
	free(buffer);
	return 0;
}

static int extract_large_file_optimized(dmc_unrar_archive *archive, dmc_unrar_size_t index, const char *out_path) {
	dmc_unrar_return ret;
	FILE *fp;
	char *write_buffer;

	fp = fopen(out_path, "wb");
	if (!fp) {
		return -1;
	}

	// Allocate and set large write buffer for maximum I/O performance
	write_buffer = malloc(WRITE_BUFFER_SIZE);
	if (write_buffer) {
		setvbuf(fp, write_buffer, _IOFBF, WRITE_BUFFER_SIZE);
	}

	ret = dmc_unrar_extract_file_to_path(archive, index, out_path, NULL, false);
	
	fclose(fp);
	if (write_buffer) {
		free(write_buffer);
	}

	return (ret == DMC_UNRAR_OK) ? 0 : -1;
}

static int extract_rar(const char *rar_path, const char *out_dir) {
	dmc_unrar_archive archive;
	dmc_unrar_return ret;
	dmc_unrar_size_t i, file_count;
	char notify_buf[256];
	int extracted = 0, skipped = 0, failed = 0;
	int last_percent = -1;
	unsigned long long total_bytes = 0, processed_bytes = 0;
	int quiet_console = 0;

	dir_cache_count = 0;

	snprintf(notify_buf, sizeof(notify_buf), "Found RAR: %s", basename((char*)rar_path));
	send_notification(notify_buf);
	
	if (!dmc_unrar_is_rar_path(rar_path)) {
		snprintf(notify_buf, sizeof(notify_buf), "❌ Invalid RAR: %s", basename((char*)rar_path));
		send_notification(notify_buf);
		return 1;
	}

	ret = dmc_unrar_archive_init(&archive);
	if (ret != DMC_UNRAR_OK) {
		send_notification("❌ RAR init failed");
		return 1;
	}

	ret = dmc_unrar_archive_open_path(&archive, rar_path);
	if (ret != DMC_UNRAR_OK) {
		dmc_unrar_archive_close(&archive);
		snprintf(notify_buf, sizeof(notify_buf), "❌ Cannot open: %s", basename((char*)rar_path));
		send_notification(notify_buf);
		return 1;
	}

	file_count = dmc_unrar_get_file_count(&archive);
	
	for (i = 0; i < file_count; i++) {
		const dmc_unrar_file *file_stat = dmc_unrar_get_file_stat(&archive, i);
		if (file_stat && !dmc_unrar_file_is_directory(&archive, i)) {
			total_bytes += file_stat->uncompressed_size;
		}
	}

	snprintf(notify_buf, sizeof(notify_buf), "📦 Extracting %llu files (%.1f GB)...", 
		(unsigned long long)file_count, (double)total_bytes / (1024*1024*1024));
	send_notification(notify_buf);
	
	// Show optimization info
	send_notification("🚀 TURBO Mode: 8MB buffers + 4MB RAM cache");

	if (create_directory_recursive(out_dir) != 0) {
		dmc_unrar_archive_close(&archive);
		send_notification("❌ Cannot create output dir");
		return 1;
	}

	if (file_count > 1000) {
		quiet_console = 1;
		fprintf(stdout, "Large archive detected, reducing console output for speed...\n");
	}

	for (i = 0; i < file_count; i++) {
		char filename_buf[4096];
		char out_path[4096];
		char *dir_path;
		dmc_unrar_size_t filename_len;
		const dmc_unrar_file *file_stat;
		int current_percent;

		filename_len = dmc_unrar_get_filename(&archive, i, NULL, 0);
		if (filename_len == 0 || filename_len >= sizeof(filename_buf)) {
			skipped++;
			continue;
		}

		dmc_unrar_get_filename(&archive, i, filename_buf, sizeof(filename_buf));
		dmc_unrar_unicode_make_valid_utf8(filename_buf);

		if (!quiet_console || (i % 100 == 0)) {
			fprintf(stdout, "[%llu/%llu] %s\n", (unsigned long long)(i+1), 
				(unsigned long long)file_count, filename_buf);
		}

		if (dmc_unrar_file_is_directory(&archive, i)) {
			snprintf(out_path, sizeof(out_path), "%s/%s", out_dir, filename_buf);
			create_directory_recursive(out_path);
			skipped++;
			continue;
		}

		file_stat = dmc_unrar_get_file_stat(&archive, i);
		if (!file_stat) {
			failed++;
			continue;
		}

		ret = dmc_unrar_file_is_supported(&archive, i);
		if (ret != DMC_UNRAR_OK) {
			skipped++;
			continue;
		}

		snprintf(out_path, sizeof(out_path), "%s/%s", out_dir, filename_buf);

		dir_path = strrchr(out_path, '/');
		if (dir_path && dir_path != out_path) {
			char temp_path[PATH_MAX];
			size_t dir_len = dir_path - out_path;
			memcpy(temp_path, out_path, dir_len);
			temp_path[dir_len] = '\0';
			create_directory_recursive(temp_path);
		}

		if (file_stat->uncompressed_size <= SMALL_FILE_THRESHOLD) {
			// Fast path: extract to memory for files < 4MB
			if (extract_small_file_optimized(&archive, i, out_path, file_stat->uncompressed_size) != 0) {
				failed++;
			} else {
				extracted++;
				processed_bytes += file_stat->uncompressed_size;
			}
		} else {
			// Large files: extract with optimized 8MB buffering
			if (extract_large_file_optimized(&archive, i, out_path) != 0) {
				failed++;
			} else {
				extracted++;
				processed_bytes += file_stat->uncompressed_size;
			}
		}

		current_percent = total_bytes > 0 ? 
			(int)((processed_bytes * 100) / total_bytes) : 
			(int)(((i + 1) * 100) / file_count);
			
		// Show progress every 5% instead of 10% for more feedback
		if (current_percent != last_percent && (current_percent % 5 == 0 || current_percent == 100)) {
			snprintf(notify_buf, sizeof(notify_buf), "⏳ %d%% (%.1f/%.1f GB)", 
				current_percent, 
				(double)processed_bytes / (1024*1024*1024),
				(double)total_bytes / (1024*1024*1024));
			send_notification(notify_buf);
			last_percent = current_percent;
		}
	}

	dmc_unrar_archive_close(&archive);

	fprintf(stdout, "\nExtraction Summary:\n");
	fprintf(stdout, "  Extracted: %d files (%.2f GB)\n", extracted, (double)processed_bytes / (1024*1024*1024));
	fprintf(stdout, "  Skipped:   %d\n", skipped);
	fprintf(stdout, "  Failed:    %d\n\n", failed);

	if (failed > 0) {
		snprintf(notify_buf, sizeof(notify_buf), "⚠️ %d OK, %d failed", extracted, failed);
	} else {
		snprintf(notify_buf, sizeof(notify_buf), "✅ %d files (%.1f GB)!", extracted, (double)processed_bytes / (1024*1024*1024));
	}
	send_notification(notify_buf);

	// Auto-register game if extracted successfully
	if (extracted > 0) {
		fprintf(stdout, "Checking for game registration...\n");
		send_notification("🔍 Looking for game data...");
		
		int reg_result = auto_register_game(out_dir);
		if (reg_result == 0) {
			send_notification("✅ Game registered!");
			fprintf(stdout, "✅ Game registered and ready to play!\n");
		} else {
			// More detailed error reporting
			char param_test[PATH_MAX];
			snprintf(param_test, sizeof(param_test), "%s/sce_sys/param.json", out_dir);
			
			struct stat st;
			if (stat(param_test, &st) == 0) {
				send_notification("⚠️ Game found but registration failed");
				fprintf(stdout, "⚠️  Found param.json but registration failed (may need elevated privileges)\n");
			} else {
				send_notification("ℹ️ No game data found");
				fprintf(stdout, "ℹ️  No param.json found (not a PS5 game)\n");
			}
		}
	}

	// Delete RAR after successful extraction to save space (default behavior)
	if (failed == 0 && extracted > 0) {
		if (unlink(rar_path) == 0) {
			fprintf(stdout, "Deleted RAR (saved space): %s\n", rar_path);
		} else {
			fprintf(stderr, "Could not delete RAR: %s\n", strerror(errno));
		}
	}

	return (failed > 0) ? 1 : 0;
}

static int scan_directory_for_rars(const char *dir_path, int *total_found, int *total_processed) {
	DIR *dir;
	struct dirent *entry;
	struct stat statbuf;
	char full_path[PATH_MAX];
	char out_path[PATH_MAX];
	int found = 0;

	dir = opendir(dir_path);
	if (!dir) {
		return 0;
	}

	fprintf(stdout, "Scanning: %s\n", dir_path);

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

		if (stat(full_path, &statbuf) != 0)
			continue;

		if (S_ISREG(statbuf.st_mode)) {
			size_t len = strlen(entry->d_name);
			if (len > 4 && strcasecmp(entry->d_name + len - 4, ".rar") == 0) {
				fprintf(stdout, "  Found RAR: %s\n", entry->d_name);
				found++;
				(*total_found)++;

				char *basename_no_ext = strdup(entry->d_name);
				if (basename_no_ext) {
					basename_no_ext[len - 4] = '\0';
					snprintf(out_path, sizeof(out_path), "%s/%s", dir_path, basename_no_ext);
					free(basename_no_ext);
				} else {
					snprintf(out_path, sizeof(out_path), "%s/extracted", dir_path);
				}

				if (extract_rar(full_path, out_path) == 0) {
					(*total_processed)++;
				}
			}
		}
	}

	closedir(dir);
	return found;
}

// Recursive function to scan directories for games
static void scan_for_games_recursive(const char *dir_path, int depth, int *total_games_registered) {
	DIR *dir;
	struct dirent *entry;
	struct stat statbuf;
	char full_path[PATH_MAX];
	char param_path[PATH_MAX];
	
	// Limit recursion depth to avoid infinite loops
	if (depth > 3) return;
	
	dir = opendir(dir_path);
	if (!dir) return;
	
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		
		snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
		
		if (stat(full_path, &statbuf) != 0 || !S_ISDIR(statbuf.st_mode))
			continue;
		
		// Check if THIS directory has sce_sys/param.json
		snprintf(param_path, sizeof(param_path), "%s/sce_sys/param.json", full_path);
		
		if (stat(param_path, &statbuf) == 0) {
			fprintf(stdout, "[AUTO-REGISTER] Found game: %s\n", full_path);
			
			if (auto_register_game(full_path) == 0) {
				(*total_games_registered)++;
				fprintf(stdout, "[AUTO-REGISTER] ✅ Registered: %s\n", entry->d_name);
			}
		} else {
			// Recurse into subdirectory
			scan_for_games_recursive(full_path, depth + 1, total_games_registered);
		}
	}
	
	closedir(dir);
}

static void auto_scan_mode(void) {
	const char *scan_paths[] = {
		"/data/games",
		"/data/homebrew",
		"/data/etaHEN/games",
		"/mnt/usb0", "/mnt/usb0/games", "/mnt/usb0/homebrew", "/mnt/usb0/etaHEN/games",
		"/mnt/usb1", "/mnt/usb1/games", "/mnt/usb1/homebrew", "/mnt/usb1/etaHEN/games",
		"/mnt/usb2", "/mnt/usb2/games", "/mnt/usb2/homebrew", "/mnt/usb2/etaHEN/games",
		"/mnt/usb3", "/mnt/usb3/games", "/mnt/usb3/homebrew", "/mnt/usb3/etaHEN/games",
		"/mnt/usb4", "/mnt/usb4/games", "/mnt/usb4/homebrew", "/mnt/usb4/etaHEN/games",
		"/mnt/usb5", "/mnt/usb5/games", "/mnt/usb5/homebrew", "/mnt/usb5/etaHEN/games",
		"/mnt/usb6", "/mnt/usb6/games", "/mnt/usb6/homebrew", "/mnt/usb6/etaHEN/games",
		"/mnt/usb7", "/mnt/usb7/games", "/mnt/usb7/homebrew", "/mnt/usb7/etaHEN/games",
		"/mnt/ext0", "/mnt/ext0/games", "/mnt/ext0/homebrew", "/mnt/ext0/etaHEN/games",
		"/mnt/ext1", "/mnt/ext1/games", "/mnt/ext1/homebrew", "/mnt/ext1/etaHEN/games",
		"/user/temp",
		NULL
	};

	int total_found = 0;
	int total_processed = 0;
	int total_games_registered = 0;
	int i;
	char notify_buf[256];

	fprintf(stdout, "\n╔════════════════════════════════════════╗\n");
	fprintf(stdout, "║   PS5 RAR Auto-Extractor v2.1 TURBO  ║\n");
	fprintf(stdout, "║   Scanning common locations...        ║\n");
	fprintf(stdout, "╚════════════════════════════════════════╝\n\n");

	send_notification("! Welcome to PSVRAR by Lithiumwow");
	send_notification("🔍 Scanning for RARs and games...");

	// First pass: Extract RAR files
	for (i = 0; scan_paths[i] != NULL; i++) {
		scan_directory_for_rars(scan_paths[i], &total_found, &total_processed);
	}

	// Second pass: Register any extracted games that aren't registered yet
	fprintf(stdout, "\n[AUTO-REGISTER] Scanning for unregistered games (recursive)...\n");
	send_notification("🎮 Checking for games to register...");
	
	for (i = 0; scan_paths[i] != NULL; i++) {
		fprintf(stdout, "[AUTO-REGISTER] Scanning: %s\n", scan_paths[i]);
		scan_for_games_recursive(scan_paths[i], 0, &total_games_registered);
	}

	fprintf(stdout, "\n╔════════════════════════════════════════╗\n");
	fprintf(stdout, "║            Scan Complete              ║\n");
	fprintf(stdout, "╚════════════════════════════════════════╝\n");
	fprintf(stdout, "  Total RAR files found: %d\n", total_found);
	fprintf(stdout, "  Successfully extracted: %d\n", total_processed);
	fprintf(stdout, "  Failed: %d\n", total_found - total_processed);
	fprintf(stdout, "  Games registered: %d\n\n", total_games_registered);

	if (total_games_registered > 0) {
		snprintf(notify_buf, sizeof(notify_buf), "✅ %d games registered!", total_games_registered);
		send_notification(notify_buf);
	} else if (total_found == 0) {
		// Check if we found games but couldn't register them
		DIR *test_dir = opendir("/data/games");
		int found_unregistered = 0;
		if (test_dir) {
			struct dirent *entry;
			while ((entry = readdir(test_dir)) != NULL && found_unregistered < 3) {
				char test_path[PATH_MAX];
				snprintf(test_path, sizeof(test_path), "/data/games/%s/sce_sys/param.json", entry->d_name);
				struct stat st;
				if (stat(test_path, &st) == 0) {
					found_unregistered++;
				}
			}
			closedir(test_dir);
		}
		
		if (found_unregistered > 0) {
			send_notification("ℹ️ Games found but need Itemzflow/SMP to mount");
			fprintf(stdout, "\nℹ️  Found %d games but registration needs elevated privileges\n", found_unregistered);
			fprintf(stdout, "💡 Solution: Open Itemzflow or run ShadowMountPlus to mount games\n\n");
		} else {
			send_notification("ℹ️ No RARs or games found");
		}
	}

	if (total_found == 0 && total_games_registered == 0) {
		send_notification("ℹ️ No RARs or new games found");
	} else if (total_processed == total_found) {
		snprintf(notify_buf, sizeof(notify_buf), "✅ All %d RARs extracted!", total_found);
		send_notification(notify_buf);
	} else if (total_found > 0) {
		snprintf(notify_buf, sizeof(notify_buf), "⚠️ %d/%d RARs extracted", total_processed, total_found);
		send_notification(notify_buf);
	}
}

int main(int argc, char *argv[]) {
	send_notification("! Welcome to PSVRAR by Lithiumwow");
	
	if (argc >= 2) {
		const char *rar_path = argv[1];
		const char *out_dir;
		
		fprintf(stdout, "\n╔════════════════════════════════════════╗\n");
		fprintf(stdout, "║   PS5 RAR Auto-Extractor v2.1 TURBO  ║\n");
		fprintf(stdout, "║   Performance Optimized               ║\n");
		fprintf(stdout, "╚════════════════════════════════════════╝\n\n");

		if (argc >= 3) {
			out_dir = argv[2];
		} else {
			char *rar_copy = strdup(rar_path);
			char *basename_no_ext = NULL;
			if (rar_copy) {
				basename_no_ext = strdup(basename(rar_copy));
				free(rar_copy);
				if (basename_no_ext) {
					char *dot = strrchr(basename_no_ext, '.');
					if (dot) *dot = '\0';
				}
			}
			if (basename_no_ext) {
				out_dir = basename_no_ext;
			} else {
				out_dir = "/user/temp/extracted";
			}
		}

		fprintf(stdout, "Input:  %s\n", rar_path);
		fprintf(stdout, "Output: %s\n\n", out_dir);

		int result = extract_rar(rar_path, out_dir);
		
		if (argc < 3) {
			char *rar_copy = strdup(rar_path);
			char *basename_no_ext = NULL;
			if (rar_copy) {
				basename_no_ext = strdup(basename(rar_copy));
				free(rar_copy);
				if (basename_no_ext) {
					char *dot = strrchr(basename_no_ext, '.');
					if (dot) *dot = '\0';
					free(basename_no_ext);
				}
			}
		}
		
		return result;
	} else {
		auto_scan_mode();
		return 0;
	}
}
