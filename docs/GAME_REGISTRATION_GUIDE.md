# Game Registration Guide - unrar_extract

## Understanding Game Registration on PS5

After extracting RAR files, games need to be **registered** with the PS5 system to appear in your library. This guide explains all methods and when to use each.

---

## 🎯 Quick Reference

| Extraction Location | Registration Method | Auto-Appears? |
|---------------------|---------------------|---------------|
| `/data/etaHEN/games/` | Automatic | ✅ Yes |
| `/mnt/usb0-7/` | Itemzflow Refresh | ❌ Manual |
| `/mnt/ext0-1/` | Itemzflow Refresh | ❌ Manual |
| `/data/games/` | Itemzflow Refresh | ❌ Manual |
| `.exfat` images | ShadowMountPlus | ✅ Yes |

---

## Method 1: Extract to etaHEN Folder (Easiest)

### ✅ Automatic Registration

**Where to extract:** `/data/etaHEN/games/`

**Steps:**
1. Upload RAR to `/data/etaHEN/games/`
2. Run `unrar_extract.elf`
3. Games appear automatically! 🎮

**Pros:**
- Zero manual steps
- Always works
- No refresh needed

**Cons:**
- Uses internal storage
- Limited by PS5 storage capacity

---

## Method 2: Itemzflow Refresh (Recommended for USB)

### 🔄 Manual Refresh Required

**Where to extract:** USB drives (`/mnt/usb0-7/`) or custom locations

**Steps:**
1. Run `unrar_extract.elf` (creates symlinks)
2. Open **Itemzflow** app on PS5
3. Select **"Refresh Games"** option
4. Games appear in both Itemzflow and PS5 home menu! ✅

**What happens:**
- `unrar_extract.elf` creates symlinks at `/system_data/priv/mnt/app/<TITLEID>`
- Itemzflow scans this location when you click "Refresh"
- PS5 system database is updated
- Games appear everywhere

**Pros:**
- Fast (< 5 seconds)
- Works without privileges
- Games stay on USB (saves internal storage)
- Reliable

**Cons:**
- Requires one manual step
- Needs Itemzflow installed

---

## Method 3: Manual Path Registration

### 🛠️ For Custom/Non-Standard Locations

**When to use:** Games extracted to custom folders not in standard locations

**Steps:**
1. Run `unrar_extract.elf`
2. Open **Itemzflow**
3. Go to **Settings** → **Add Game Path**
4. Enter the path where games were extracted (e.g., `/mnt/usb0/my-games/`)
5. Click **Refresh**
6. Games from that path now appear

**Pros:**
- Works for any location
- Persistent (remembers path)

**Cons:**
- Most manual steps
- Need to add each new path

---

## Method 4: ShadowMountPlus (For Disk Images)

### 💿 Automatic Mounting of Game Images

**When to use:** Games in `.exfat` format (disk images, not extracted folders)

**Steps:**
1. Place `.exfat` game image on USB/storage
2. Load **kstuff.elf** first (required)
3. Load **ShadowMountPlus.elf**
4. Games mount and appear automatically! ✅

**What it does:**
- Auto-detects `.exfat` formatted game images
- Mounts them as if they were disc games
- Registers in PS5 library automatically

**Pros:**
- Fully automatic
- Best for disk images
- No manual refresh

**Cons:**
- Requires kstuff payload
- Only works with `.exfat` images (not extracted folders)

---

## Method 5: Rebuild Database (Slowest)

### 🐌 Full System Rescan

**When to use:** When nothing else works or for complete refresh

**Steps:**
1. Run `unrar_extract.elf` first
2. Go to PS5 **Settings**
3. **System** → **System Software** → **Rebuild Database**
4. Wait 5-10 minutes
5. All symlinked games appear

**Pros:**
- 100% reliable
- No additional tools needed
- Fixes database corruption

**Cons:**
- Slowest method (5-10 minutes)
- Rescans everything

---

## 🤔 Which Method Should I Use?

### Scenario 1: Small Games, Internal Storage Available
**Use:** Method 1 (Extract to `/data/etaHEN/games/`)
- Simplest, fully automatic

### Scenario 2: Large Games, Want to Save Internal Storage
**Use:** Method 2 (Itemzflow Refresh)
- Extract to USB
- One quick manual step
- Games stay on USB

### Scenario 3: Have Game Disk Images (.exfat)
**Use:** Method 4 (ShadowMountPlus)
- No extraction needed
- Fully automatic

### Scenario 4: Complex Setup, Custom Paths
**Use:** Method 3 (Manual Path Registration)
- Most flexible
- Works for any location

---

## ❓ Common Questions

### Q: Why don't games appear automatically after extraction?
**A:** The PS5 system needs to be told to scan for new apps. The symlinks are created, but the PS5 doesn't automatically detect them without one of the methods above.

### Q: Can't you make it automatic like Itemzflow does?
**A:** Itemzflow's "automatic" detection is actually its background daemon scanning periodically. When the daemon is running, it works. When it's not (e.g., after reboot), you need to manually refresh. Our tool creates the same symlinks Itemzflow would create.

### Q: What about the Triangle button refresh?
**A:** The Triangle (△) button in the home menu is meant to reload the app list, but it doesn't always trigger a full filesystem scan. Itemzflow's dedicated refresh function is more reliable.

### Q: Do I need to refresh every time?
**A:** No! Once a game is registered, it stays in your library. You only need to refresh when:
- First time adding new games
- After PS5 reboot (if daemon isn't running)
- After changing USB drives

### Q: What if Itemzflow daemon fails?
**A:** Itemzflow's built-in "Refresh Games" option works even when the daemon is down. It's actually more reliable than waiting for automatic detection.

---

## 🔍 Technical Details

### What unrar_extract Actually Does

1. **Extracts RAR** to target location
2. **Parses param.json** to get game metadata (titleId, title, etc.)
3. **Creates symlinks:**
   - `/system_data/priv/mnt/app/<TITLEID>` → game folder
   - `/system_data/priv/appmeta/<TITLEID>` → `sce_sys` folder
4. **Sends notification** reminder
5. **Waits for refresh** (via one of the methods above)

### Why Symlinks?

Symlinks make the PS5 think games are "installed" in `/system_data` while they actually live on USB:

```
USB Game:           /mnt/usb0/PPSA12345-app/
Symlink Created:    /system_data/priv/mnt/app/PPSA12345 → /mnt/usb0/PPSA12345-app/
PS5 Sees:           "Game PPSA12345 is installed at /system_data"
PS5 Loads From:     /mnt/usb0/PPSA12345-app/ (via symlink)
```

**Benefits:**
- ✅ Zero storage used on internal drive
- ✅ Games run directly from USB
- ✅ No copying needed
- ✅ Standard PS5 method (used by all homebrew tools)

---

## 📊 Comparison Matrix

| Feature | etaHEN Auto | Itemzflow Refresh | ShadowMountPlus | Rebuild DB |
|---------|-------------|-------------------|-----------------|------------|
| **Speed** | Instant | 5 seconds | Instant | 5-10 minutes |
| **Manual Steps** | 0 | 1 | 0 | 1 |
| **Storage Used** | Internal | None (USB) | None | None |
| **Reliability** | 100% | 95% | 100% | 100% |
| **Requirements** | None | Itemzflow | kstuff | None |
| **Best For** | Small games | Large games | Disk images | Troubleshooting |

---

## 🎯 Recommended Workflow

### Daily Use:
1. Extract to USB via `unrar_extract.elf`
2. Open Itemzflow → "Refresh Games"
3. Play! 🎮

### Bulk Setup:
1. Extract multiple games to `/data/etaHEN/games/`
2. Automatic registration
3. No manual steps

### Advanced Users:
1. Use ShadowMountPlus for `.exfat` images
2. Automatic everything
3. Most efficient

---

**Last Updated:** March 15, 2026

For more information, see the main [README.md](README.md)
