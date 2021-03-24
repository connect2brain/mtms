#!/usr/bin/env pwsh
param ([string] $m)

# Check if input argument with commit message is empty or null
if ([string]::IsNullOrEmpty($m)){
  $date = Get-Date -Format "dd/MM/yyyy HH:mm K"
  $m = "UPD: Rebuilding site: $date"
}

Write-Host "Deploying updates to GitHub with commit message: "
Write-Host $m

# Build the website
make github

# Go To Public folder
Set-Location .\public

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
