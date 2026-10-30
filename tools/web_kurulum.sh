#!/bin/bash
set -euo pipefail

if [ "$#" -ne 3 ]; then
  echo "Kullanim: bash tools/web_kurulum.sh <surum> <tam_imaj_klasoru> <cikis_klasoru>"
  echo "Ornek:    bash tools/web_kurulum.sh 1.3.0 indirilen _site"
  exit 1
fi

SURUM="$1"
KAYNAK="$2"
CIKIS="$3"
KOK="$(cd "$(dirname "$0")/.." && pwd)"

mkdir -p "$CIKIS/firmware"
cp "$KOK/web_kurulum/index.html" "$CIKIS/index.html"
cp "$KOK/docs/tanitim.png" "$CIKIS/tanitim.png"
cp "$KOK/web/icon.png" "$CIKIS/icon.png"
touch "$CIKIS/.nojekyll"

for PANEL in ILI9341 ST7789; do
  KUCUK="$(echo "$PANEL" | tr '[:upper:]' '[:lower:]')"
  IMAJ="$KAYNAK/Rumy-$SURUM-$PANEL-tam-0x0.bin"
  if [ ! -f "$IMAJ" ]; then
    echo "Tam imaj bulunamadi: $IMAJ"
    exit 1
  fi
  cp "$IMAJ" "$CIKIS/firmware/rumy-$SURUM-$KUCUK.bin"
  cat > "$CIKIS/manifest-$KUCUK.json" <<EOF
{
  "name": "Rumy ($PANEL)",
  "version": "$SURUM",
  "new_install_prompt_erase": true,
  "new_install_improv_wait_time": 0,
  "builds": [
    {
      "chipFamily": "ESP32",
      "parts": [
        { "path": "firmware/rumy-$SURUM-$KUCUK.bin", "offset": 0 }
      ]
    }
  ]
}
EOF
done

echo "web kurulum sitesi hazir: $CIKIS (surum $SURUM)"
