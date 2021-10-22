#!/usr/bin/env pwsh
param ([string] $m)

$SOURCEDIR = ".\build\html\*"
$OUTPUTDIR = ".\public"

# Check if input argument with commit message is empty or null
if ([string]::IsNullOrEmpty($m)){
  $date = Get-Date -Format "dd/MM/yyyy HH:mm K"
  $m = "UPD: Rebuilding site: $date"
}

Write-Host "Deploying updates to GitHub with commit message: "
Write-Host $m

# Go To Public folder
Set-Location $OUTPUTDIR
git pull
git reset --hard origin/clean
git rebase origin/main
Set-Location ..\

# Build the website
make github
Copy-item $SOURCEDIR $OUTPUTDIR -Recurse -Force

Set-Location $OUTPUTDIR
# Add changes to git
# git status
git add .
# Commit changes
git commit -m $m
# Push source and build repos
git push origin main
# Come Back up to the Project Root
Set-Location ..\

Write-Host "Update successful!"
