#!/bin/bash

# PSVRAR GitHub Push Script
# Run this to push to https://github.com/Lithiumwow/PSVRAR

echo "=================================="
echo "  PSVRAR GitHub Push"
echo "=================================="
echo ""
echo "Repository: https://github.com/Lithiumwow/PSVRAR"
echo "Username: Lithiumwow"
echo ""
echo "This will push the PSVRAR repository to GitHub."
echo "You'll be prompted for authentication."
echo ""

cd "$(dirname "$0")"

# Check if git repo
if [ ! -d ".git" ]; then
    echo "Error: Not a git repository"
    exit 1
fi

# Show status
echo "Current status:"
git status --short
echo ""

# Push
echo "Pushing to GitHub..."
echo ""
git push -u origin main

# Check result
if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Success! Repository pushed to GitHub"
    echo ""
    echo "View at: https://github.com/Lithiumwow/PSVRAR"
    echo ""
    echo "Next steps:"
    echo "1. Visit your repository on GitHub"
    echo "2. Create a release with unrar_extract.elf"
    echo "3. Add topics: ps5, homebrew, jailbreak, rar"
else
    echo ""
    echo "❌ Push failed"
    echo ""
    echo "Authentication options:"
    echo ""
    echo "Option 1: GitHub CLI (easiest)"
    echo "  brew install gh"
    echo "  gh auth login"
    echo "  ./push_to_github.sh"
    echo ""
    echo "Option 2: Personal Access Token"
    echo "  1. Visit https://github.com/settings/tokens"
    echo "  2. Generate new token with 'repo' scope"
    echo "  3. Use token as password when prompted"
    echo ""
    echo "Option 3: SSH"
    echo "  git remote set-url origin git@github.com:Lithiumwow/PSVRAR.git"
    echo "  ./push_to_github.sh"
fi
