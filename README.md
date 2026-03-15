# PSVRAR - PS5 RAR Extractor

**Standalone RAR extraction payload for jailbroken PS5**

Extract RAR archives directly on your PS5 with automatic game registration!

![Status](https://img.shields.io/badge/status-working-success)
![Platform](https://img.shields.io/badge/platform-PS5-blue)
![License](https://img.shields.io/badge/license-GPL--2.0-green)

## 🎯 Features

- ✅ Extract RAR archives on PS5
- ✅ Automatic game registration (ShadowMountPlus-style)
- ✅ Progress notifications
- ✅ Scans common install locations
- ✅ Simple command-line usage

## 🚀 Quick Start

### For Users

1. **Download the ELF**
   - Get `unrar_extract.elf` from [Releases](https://github.com/Lithiumwow/PSVRAR/releases)

2. **Place RAR file on PS5**
   - Upload to `/data/games/`, `/mnt/usb0/`, or any accessible location

3. **Send payload to PS5**
   ```bash
   nc PS5_IP 9021 < unrar_extract.elf
   ```

4. **The extractor will:**
   - Scan for RAR files automatically
   - Extract to the same directory
   - Register the game in your PS5 library
   - Send progress notifications

### For Developers

**Prerequisites:**
- [ps5-payload-dev/sdk](https://github.com/ps5-payload-dev/sdk)
- [ps5-payload-dev/elfldr](https://github.com/ps5-payload-dev/elfldr)

**Build:**
```bash
# Set SDK path
export PS5_PAYLOAD_SDK=/path/to/ps5-payload-sdk

# Build
cd unrar_extract
make

# Output: unrar_extract.elf
```

## 📋 Monitored Locations

The extractor automatically scans:
- `/data/games/`
- `/mnt/usb0` through `/mnt/usb7`
- `/mnt/ext0` and `/mnt/ext1`
- `/data/etaHEN/games/`
- `/user/temp/`

## 🎮 Usage Examples

### Automatic Scan
Just run the payload - it will find and extract all RAR files:
```bash
nc 192.168.1.100 9021 < unrar_extract.elf
```

## ⚠️ IMPORTANT: Making Games Appear in Library

After extraction, games need to be registered with the PS5 system. There are **three methods**:

### Method 1: Extract to etaHEN Games Folder (Automatic)
Extract directly to `/data/etaHEN/games/` and games appear automatically:
```bash
# Games in /data/etaHEN/games/ are auto-detected by the system
```

### Method 2: Use Itemzflow Refresh (Recommended)
After running `unrar_extract.elf`:
1. Open **Itemzflow** app on PS5
2. Select **"Refresh Games"** option
3. Games appear in both Itemzflow and PS5 home menu instantly!

### Method 3: Manual Path Registration
If games are extracted elsewhere (USB drives, custom locations):
1. Open **Itemzflow**
2. Go to **Settings** → **Add Game Path**
3. Manually add the path where games were extracted
4. Click **Refresh**

### Method 4: ShadowMountPlus (For .exfat images)
If you're using game images in `.exfat` format:
- Load **ShadowMountPlus** payload
- Games mount automatically without manual steps

## 🔧 How It Works

1. **Scan Phase**
   - Searches common PS5 game locations
   - Identifies RAR archives
   - Displays notification: "RAR file found"

2. **Extract Phase**
   - Extracts archive in place
   - Shows progress notifications with percentages
   - Displays notification: "Uncompressing..."

3. **Register Phase**
   - Parses `param.json` for game info
   - Creates symbolic links in `/system_data/priv/mnt/app/`
   - **Important:** Games require one additional step to appear (see above)

## 📂 Best Practices

### For Automatic Registration
**Extract to:** `/data/etaHEN/games/`
- Games appear automatically
- No manual steps required
- Works with all homebrew managers

### For USB/External Storage
**After extraction:**
1. Run `unrar_extract.elf` (creates symlinks)
2. Open Itemzflow → "Refresh Games"
3. Games appear in library

### For Game Images (.exfat)
**Use ShadowMountPlus:**
- Handles `.exfat` formatted game images
- Auto-mounts without extraction
- Best for disk images

## 📊 Technical Details

**Libraries Used:**
- **dmc_unrar** - Single-file RAR extraction library (GPL-2.0)
- **ps5-payload-dev/sdk** - PS5 payload development SDK

**File Structure:**
```
unrar_extract/
├── main.c              # Main extraction logic
├── dmc_unrar.c         # RAR library
├── game_register.c     # Auto-registration
├── game_register.h
├── Makefile
└── README.md
```

**Build Output:**
- `unrar_extract.elf` - PS5 executable payload
- Size: ~80-100KB
- Target: FreeBSD (PS5 OS)

## 🤝 Credits

**Author:** Lithiumwow

**Dependencies:**
- [ps5-payload-dev/sdk](https://github.com/ps5-payload-dev/sdk) - PS5 SDK
- [ps5-payload-dev/elfldr](https://github.com/ps5-payload-dev/elfldr) - ELF loader
- [dmc_unrar](https://github.com/DrMcCoy/dmc_unrar) - RAR extraction library

## 📜 License

GPL-2.0 License (inherited from dmc_unrar)

## ⚠️ Disclaimer

This tool is for educational and personal use on jailbroken PS5 consoles. Always use legal copies of games.

## 🐛 Troubleshooting

### "No RAR files found"
- Ensure RAR is in one of the monitored locations
- Check file has `.rar` extension
- Verify file permissions

### "Extraction failed"
- RAR file may be corrupted
- Check available disk space
- Ensure proper file permissions

### "Game not appearing in library"
**Solution 1 (Fastest):** Open Itemzflow → "Refresh Games"
**Solution 2:** Extract to `/data/etaHEN/games/` instead (auto-detects)
**Solution 3:** Settings → System → Rebuild Database (5-10 min)
**Solution 4:** Load ShadowMountPlus (for .exfat images)

### "Itemzflow daemon failed"
- Itemzflow's background service isn't running
- **Solution:** Use Itemzflow's built-in "Refresh Games" option instead
- This works even when daemon is down

### "Games on USB not appearing"
- Symlinks created but PS5 hasn't refreshed
- **Solution:** Open Itemzflow → "Refresh Games" or manually add path in Itemzflow settings

## 🔗 Related Projects

- **[PS-Share](https://github.com/Lithiumwow/PS-Share)** - Web-based PS5 screen streaming
- **[PSVRAR Suite](https://github.com/Lithiumwow/PSVRAR-Suite)** - Advanced RAR management with web UI

## 📚 Documentation

Detailed guides available in the [`docs/`](docs/) folder:

- **[QUICK_START.md](docs/QUICK_START.md)** - Quick reference guide (3 steps)
- **[GAME_REGISTRATION_GUIDE.md](docs/GAME_REGISTRATION_GUIDE.md)** - Complete registration methods
- **[PERFORMANCE_OPTIMIZATION.md](docs/PERFORMANCE_OPTIMIZATION.md)** - Speed optimization details
- **[TRIANGLE_SOLUTION.md](docs/TRIANGLE_SOLUTION.md)** - Manual refresh workflow
- **[PATH_FIX.md](docs/PATH_FIX.md)** - Symlink and path troubleshooting
- **[DATABASE_REFRESH_FIX.md](docs/DATABASE_REFRESH_FIX.md)** - Database refresh technical details

## 📝 Changelog

### v2.2 TURBO+ (March 15, 2026)
- **2x faster extraction** - Increased buffers to 8MB, fixed buffer bug
- RAM cache increased to 4MB for small files
- Improved Itemzflow refresh workflow
- Added comprehensive game registration guide

### v2.1 (March 15, 2026)
- Added Itemzflow refresh support
- Symlink-based game mounting
- Multiple registration methods documented

### v1.0.0 (March 2026)
- Initial release
- RAR extraction
- Automatic game registration
- Progress notifications
- Multi-location scanning

---

**Last Updated:** March 15, 2026

For issues or contributions, please open an issue on GitHub!
