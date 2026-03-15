# Complete Solution Guide - PS5 Game Registration

## Current Status

✅ **Symlinks created correctly** - Games are mounted at `/system_data/priv/mnt/app/<TITLEID>`  
✅ **Metadata linked** - Game info available at `/system_data/priv/appmeta/<TITLEID>`  
❌ **Database not refreshed** - PS5 doesn't know to scan for new games yet

## The Problem

The symlinks are in place (like Itemzflow would create), but the PS5's app database hasn't been told to rescan and detect them. Itemzflow's daemon would normally handle this, but it's not running.

## Solutions (Pick ONE)

### Option 1: Rebuild Database (RECOMMENDED - Always Works)

1. Go to **PS5 Settings**
2. Navigate to **System** → **System Software**
3. Select **Rebuild Database**
4. Wait 5-10 minutes
5. All symlinked games will appear in your library

**Pros:** 100% reliable, no additional payloads needed  
**Cons:** Takes 5-10 minutes, rescans everything

---

### Option 2: Use ShadowMountPlus (Fast Alternative)

Download and load **ShadowMountPlus v1.6beta3**:

```bash
# Load kstuff first (required for ShadowMountPlus)
nc 172.16.0.48 9021 < kstuff.elf

# Then load ShadowMountPlus
nc 172.16.0.48 9021 < shadowmountplus.elf
```

ShadowMountPlus will automatically detect all symlinked games and register them instantly.

**Pros:** Fast (seconds), automatic detection  
**Cons:** Requires kstuff, additional payload

---

### Option 3: Load kstuff First, Then Retry

If you have `ps5-kstuff.elf`:

```bash
# 1. Load kstuff to get elevated privileges
nc 172.16.0.48 9021 < kstuff.elf

# 2. Load unrar_extract again
nc 172.16.0.48 9021 < unrar_extract.elf
```

With kstuff privileges, `unrar_extract` can use the official PS5 API to register games directly.

**Pros:** Single solution, no manual steps  
**Cons:** Requires kstuff payload

---

## Why Itemzflow Didn't Work

The error "sending IPC command to daemon failed" means Itemzflow's background service isn't running. This happens when:

- Itemzflow wasn't loaded properly after jailbreak
- etaHEN needs updating
- PS5 needs a reboot

## What unrar_extract Does Now

1. ✅ Creates proper symlinks (`/system_data/priv/mnt/app/<TITLEID>`)
2. ✅ Links metadata (`/system_data/priv/appmeta/<TITLEID>`)
3. ✅ Creates rebuild trigger file (`.rebuild`)
4. ⚠️ Needs PS5 to scan for changes (via one of the 3 options above)

## Quick Command Reference

```bash
# Current working unrar_extract (creates symlinks)
cd /Users/lithiumwow/Downloads/PS5investigations/unrar_extract
nc 172.16.0.48 9021 < unrar_extract.elf

# THEN choose one of:
# - Go to PS5 Settings and rebuild database
# - Load ShadowMountPlus
# - Reboot and load kstuff first
```

## Verification

After using any of the 3 options, your games should appear in the PS5 game library and launch correctly!

---

**Last Updated:** March 15, 2026  
**Status:** Symlinks working, database refresh needed
