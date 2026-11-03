#!/bin/bash
# Renders icon.svg to icon.png (256x256) using headless Chrome
# (ImageMagick's built-in SVG renderer drops strokes)
set -e
cd "$(dirname "$0")"
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
printf '<html><body style="margin:0;background:transparent"><img src="file://%s" width="256" height="256" style="display:block"></body></html>' "$PWD/icon.svg" > "$TMP/page.html"
google-chrome --headless=new --disable-gpu --hide-scrollbars --default-background-color=00000000 \
    --force-device-scale-factor=1 --window-size=256,256 --screenshot="$PWD/icon.png" "file://$TMP/page.html" >/dev/null 2>&1
