# PSVRAR - PS5 RAR Extractor

Fast RAR extraction tool for jailbroken PS5 consoles.

## Download

Get the latest version from [Releases](https://github.com/Lithiumwow/PSVRAR/releases/latest)

## Usage

1. Upload your RAR file to PS5 (via FTP or USB)
2. Send the payload:
   ```bash
   nc YOUR_PS5_IP 9021 < unrar_extract.elf
   ```
3. Done! The tool will find and extract RARs automatically.

## Making Games Appear

After extraction, games need to be registered. Pick one method:

**Easiest:** Extract to `/data/etaHEN/games/` - games appear automatically

**For USB drives:** 
1. Run the tool to extract and create symlinks
2. Open Itemzflow on PS5
3. Click "Refresh Games"
4. Your games will appear in the library

**Other options:**
- Use Itemzflow's "Add Game Path" for custom locations
- Use ShadowMountPlus for `.exfat` image files
- Go to Settings > Rebuild Database (takes 5-10 minutes)

## What It Does

- Scans common locations for RAR files
- Extracts archives in place
- Creates game symlinks automatically
- Shows progress on screen
- Works with games on USB drives

The tool scans:
- `/data/games/`
- `/data/etaHEN/games/`
- `/mnt/usb0` through `/mnt/usb7`
- `/mnt/ext0` and `/mnt/ext1`
- `/user/temp/`

## Building from Source

Need the [PS5 Payload SDK](https://github.com/ps5-payload-dev/sdk) first.

```bash
export PS5_PAYLOAD_SDK=/path/to/ps5-payload-sdk
cd unrar_extract
make
```

## Performance

Current version uses 8MB buffers and 4MB RAM cache for fast extraction. Typical speeds:
- 10GB game: ~4 minutes
- 50GB game: ~20 minutes
- 100GB game: ~40 minutes

Speed depends on your storage (USB vs internal SSD).

## Requirements

- Jailbroken PS5 running etaHEN or GoldHEN
- ELF loader listening on port 9021

## Credits

Uses [dmc_unrar](https://github.com/DrMcCoy/dmc_unrar) for RAR extraction.

Built with [ps5-payload-dev/sdk](https://github.com/ps5-payload-dev/sdk).

## Related

- [PS-Share](https://github.com/Lithiumwow/PS-Share) - Web-based PS5 screen streaming
- [PSVRAR Suite](https://github.com/Lithiumwow/PSVRAR-Suite) - RAR manager with web interface

## License

GPL-2.0 (inherited from dmc_unrar)

## Changelog

**v2.2 TURBO+ (March 15, 2026)**
- 2x faster extraction with optimized buffers
- Increased write buffer to 8MB
- RAM cache up to 4MB for small files
- Fixed critical buffer allocation bug
- Better Itemzflow integration

**v2.1 (March 15, 2026)**
- Added Itemzflow refresh support
- Symlink-based mounting for USB games
- Better game registration

**v1.0.0 (March 2026)**
- Initial release
