# Quick Start Guide - unrar_extract

## How to Use (3 Easy Steps)

### Step 1: Load the Payload
```bash
cd /Users/lithiumwow/Downloads/PS5investigations/unrar_extract
nc 172.16.0.48 9021 < unrar_extract.elf
```

**What it does:**
- Scans all your drives for games and RARs
- Creates mount symlinks automatically
- Shows notifications on PS5

---

### Step 2: Press Triangle (△) in Game Library

On your PS5 controller, while in the game library screen:

```
     △
    ◁ ▷      ← Press this button (Triangle)
     ▽
```

This triggers "Reload Apps" and makes your games appear instantly!

---

### Step 3: Play Your Games! 🎮

All mounted games now appear in your library and can be launched normally.

---

## What You'll See

### On Your Mac:
```
╔════════════════════════════════════════╗
║   PS5 RAR Auto-Extractor v2.1 TURBO  ║
║   Scanning common locations...        ║
╚════════════════════════════════════════╝

[AUTO-REGISTER] Found game: /mnt/usb0/PPSA17221-app
[REGISTER] Creating mount point...
[REGISTER] ✅ Game mounted at /system_data/priv/mnt/app/PPSA17221
[REGISTER] ⚠️  Manual action required:
[REGISTER]    Press TRIANGLE (△) in game library to reload

Games registered: 8
```

### On Your PS5:
```
📱 Notification: "✅ God of War Ragnarök ready! Press Triangle to reload library"
```

Then press **Triangle** and your games appear!

---

## Troubleshooting

### Q: I pressed Triangle but nothing happened?
**A:** Make sure you're in the actual game library screen (not settings or another app).

### Q: Can I automate the Triangle press?
**A:** No - it requires kernel privileges. But it's instant (< 1 sec), so it's actually the fastest method!

### Q: Do I need to press Triangle every time?
**A:** Only when you first mount new games. Already mounted games stay in your library.

### Q: What if I don't want to press Triangle?
**A:** Use one of these alternatives:
- Go to Settings → Rebuild Database (takes 5-10 min)
- Load ShadowMountPlus (auto-detects)
- Load kstuff.elf first, then unrar_extract (auto-registers)

---

## Features

✅ Fast RAR extraction (optimized speed)  
✅ Auto-scans all USB drives and storage  
✅ Recursive game directory scanning  
✅ Itemzflow-compatible symlink mounting  
✅ Progress notifications on PS5  
✅ Automatic RAR cleanup after extraction  
✅ Zero internal storage used (games run from USB)  

---

## File Locations It Scans

- `/data/games`
- `/data/etaHEN/games`
- `/mnt/usb0-7` (all USB drives)
- `/mnt/ext0-1` (external drives)
- `/user/temp`

And all subdirectories recursively!

---

**That's it!** Three simple steps and your games are ready to play.

For more technical details, see `TRIANGLE_SOLUTION.md`
