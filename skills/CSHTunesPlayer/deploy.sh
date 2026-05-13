#!/bin/bash
# Deploy CSHTunes files to ThinkCentre (.136)
# Run from the CSHTunesPlayer skills folder
DIR="$(cd "$(dirname "$0")" && pwd)"
scp -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no "$DIR"/mockup.html "$DIR"/cshtunes-cover-t1q1.png milton@192.168.12.136:/var/www/cshtunes/
echo "Deployed to http://192.168.12.136:8080/mockup.html"
