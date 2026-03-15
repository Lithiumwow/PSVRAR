# How to Push PSVRAR to GitHub

The repository is ready to push! I've initialized git, created a commit, and added the remote, but you'll need to authenticate with your GitHub credentials to push.

## Quick Push (Recommended)

```bash
cd /Users/lithiumwow/Downloads/PS5investigations/unrar_extract

# Push to your GitHub
git push -u origin main
```

If you get an authentication error, follow the options below.

## Authentication Options

### Option 1: GitHub CLI (Easiest)
```bash
# Install gh if you don't have it
brew install gh

# Login
gh auth login

# Push
git push -u origin main
```

### Option 2: Personal Access Token
1. Go to https://github.com/settings/tokens
2. Generate new token (classic) with `repo` scope
3. Use token as password when prompted:
   ```bash
   git push -u origin main
   # Username: Lithiumwow
   # Password: <your-token-here>
   ```

### Option 3: SSH Key
```bash
# Change remote to SSH
git remote set-url origin git@github.com:Lithiumwow/PSVRAR.git

# Push
git push -u origin main
```

## What's Been Prepared

✅ Git repository initialized  
✅ All files added and committed  
✅ Remote set to https://github.com/Lithiumwow/PSVRAR.git  
✅ Branch renamed to `main`  
✅ Professional README.md created  
✅ .gitignore configured  

## Files in Repository

```
PSVRAR/
├── .gitignore           # Ignore build files
├── README.md            # Professional GitHub README
├── Makefile             # Build system
├── main.c               # Main extractor logic
├── dmc_unrar.c          # RAR library (GPL-2.0)
├── game_register.c      # Auto-registration
└── game_register.h      # Header file
```

## After Pushing

Your repository will be live at:
**https://github.com/Lithiumwow/PSVRAR**

### Recommended Next Steps

1. **Add Release**
   ```bash
   # Build the ELF
   export PS5_PAYLOAD_SDK=/Users/lithiumwow/ps5-payload-sdk
   make
   
   # Create GitHub release with unrar_extract.elf
   gh release create v1.0.0 unrar_extract.elf \
     --title "PSVRAR v1.0.0" \
     --notes "Initial release - PS5 RAR extractor with auto-registration"
   ```

2. **Add Topics**
   - Go to repository settings
   - Add topics: `ps5`, `homebrew`, `jailbreak`, `rar`, `extractor`

3. **Update Description**
   - Set description: "PS5 RAR extraction payload with automatic game registration"
   - Add website: link to documentation if desired

## Troubleshooting

### Still Getting 403 Error?
Make sure you're pushing to the correct account:
```bash
git config user.name "Lithiumwow"
git config user.email "your-email@example.com"
```

### Wrong Remote?
Check current remote:
```bash
git remote -v
```

Change if needed:
```bash
git remote set-url origin https://github.com/Lithiumwow/PSVRAR.git
```

---

**Ready to push!** Just run `git push -u origin main` with your GitHub credentials.
