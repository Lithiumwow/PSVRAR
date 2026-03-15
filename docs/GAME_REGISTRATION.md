# Game Registration Troubleshooting Guide

## 🎮 How Game Registration Works

The extractor tries to automatically register games similar to **ShadowMountPlus** and **Itemzflow**.

### Registration Process

1. **After extraction completes:**
   - Looks for `sce_sys/param.json` in extracted folder
   - Parses game info (titleId, title, contentId)
   - Creates symlink in `/system_data/priv/appmeta/`

2. **What you should see:**
   ```
   🔍 Looking for game data...
   [REGISTER] Attempting to register: GameName (CUSA12345)
   [REGISTER] Link: /system_data/priv/appmeta/CUSA12345 -> /path/to/game/sce_sys
   [REGISTER] Creating symlink...
   ✅ Game registered!
   ```

---

## ⚠️ Common Issues

### Issue 1: Permission Denied

**Symptom:**
```
[REGISTER] ERROR: symlink() failed, errno=1 (Permission denied)
[REGISTER] This may require running ps5-kstuff or having root access
⚠️ Game found but registration failed
```

**Why:** PS5 restricts symlink creation in `/system_data/` for security

**Solutions:**

**Option A: Use ps5-kstuff (Recommended)**
```bash
# 1. Load ps5-kstuff first (elevates privileges)
nc PS5_IP 9021 < ps5-kstuff.elf

# 2. Then run PSVRAR
nc PS5_IP 9021 < unrar_extract.elf
```

**Option B: Manual Registration**
After extraction, use **Itemzflow** or **ShadowMountPlus** to scan and register the game.

**Option C: Alternative Paths**
Extract to locations that auto-scan:
- `/data/games/` - Scanned by most homebrew
- `/mnt/usb0/games/` - USB drive location

---

### Issue 2: No Game Data Found

**Symptom:**
```
ℹ️ No game data found
ℹ️ No param.json found (not a PS5 game)
```

**Why:** The RAR doesn't contain a PS5 game (might be homebrew, updates, DLC, or other files)

**Solution:** This is normal for non-game RARs. The extraction still worked!

---

### Issue 3: Already Registered

**Symptom:**
```
[REGISTER] Game CUSA12345 already registered
```

**Why:** Game was previously registered (this is good!)

**Solution:** Nothing needed - game should already appear in library

---

## 📊 Testing Your Build

Run the improved version and check the console output:

```bash
nc 172.16.0.48 9021 < unrar_extract.elf
```

**Look for these new detailed messages:**
- `[REGISTER] Attempting to register: ...`
- `[REGISTER] Link: ... -> ...`
- `[REGISTER] Creating symlink...`
- `[REGISTER] SUCCESS: Registered game: ...`

Or if it fails:
- `[REGISTER] ERROR: symlink() failed, errno=X`

---

## 🔧 Current Build Status

**Version:** v2.1 TURBO+ (Enhanced Logging)  
**File:** `unrar_extract.elf` (356KB)

**New Features:**
- ✅ Detailed registration logging
- ✅ Shows exact symlink paths
- ✅ Reports errno on failure
- ✅ Better error notifications
- ✅ Directory creation attempts

---

## 🚀 Recommended Workflow

### For Best Results:

**1. Run ps5-kstuff first** (if available):
```bash
# Elevate privileges
nc 172.16.0.48 9021 < ps5-kstuff.elf

# Then extract
nc 172.16.0.48 9021 < unrar_extract.elf
```

**2. Alternative: Use Itemzflow/ShadowMountPlus**
```bash
# Extract game
nc 172.16.0.48 9021 < unrar_extract.elf

# Then run Itemzflow to register games
# (Opens and scans /data/games automatically)
```

**3. Manual Registration:**
```bash
# After extraction, use etaHEN's ShadowMount feature
# or Itemzflow's "Refresh Games" option
```

---

## 🎯 What This Update Does

The new build will tell you **exactly why** registration failed:

**Before:**
```
ℹ️ No param.json found or already registered
```

**After:**
```
[REGISTER] Attempting to register: Minecraft (PPSA17221)
[REGISTER] Link: /system_data/priv/appmeta/PPSA17221 -> /data/games/Minecraft/sce_sys
[REGISTER] Creating directory /system_data/priv/appmeta...
[REGISTER] Creating symlink...
[REGISTER] ERROR: symlink() failed, errno=1 (Operation not permitted)
[REGISTER] This may require running ps5-kstuff or having root access
⚠️ Game found but registration failed
```

Now you know:
- ✅ Game was found (has param.json)
- ✅ Extraction worked
- ❌ Just needs elevated privileges to register

---

## 📝 Next Steps

### Test the new build:
```bash
cd /Users/lithiumwow/Downloads/PS5investigations/unrar_extract
nc 172.16.0.48 9021 < unrar_extract.elf
```

### Check console output for:
1. Did it find `param.json`?
2. What error code (errno) did you get?
3. Does `/system_data/priv/appmeta` exist?

This will help diagnose the exact issue!

---

**Built:** March 15, 2026  
**Version:** v2.1 TURBO+ Enhanced Logging
