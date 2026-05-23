# Publish to GitHub

This folder is already curated for a public-facing repository.

## Recommended Repository Names

- `enki-leak-detection`
- `esp32-leak-detection-tflite`
- `embedded-leak-detection-system`

## Option 1: GitHub Website + GitHub Desktop

1. Create a new repository on your GitHub profile.
2. Do **not** add a new README, `.gitignore`, or license on GitHub.
3. Open `github_upload_ready` in GitHub Desktop.
4. Publish the repository to your GitHub profile.

## Option 2: Command Line

Run these commands from inside `github_upload_ready` after you create an empty GitHub repository:

```powershell
git init
git add .
git commit -m "Initial portfolio release for ENKI leak detection project"
git branch -M main
git remote add origin https://github.com/YOUR_USERNAME/YOUR_REPOSITORY_NAME.git
git push -u origin main
```

## Before You Push

- replace `YOUR_USERNAME`
- replace `YOUR_REPOSITORY_NAME`
- optionally update the README title if you pick a different repo name
- optionally add a license if you want the code to be explicitly open source
