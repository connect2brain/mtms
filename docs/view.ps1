#!/usr/bin/env pwsh

# Write-Host "Openning GitHub Page..."

# Go To Public folder
# Set-Location .\docsource

make html

# Go To Public folder
start .\build\html\index.html

# Come Back up to the Project Root
# Set-Location ..\
