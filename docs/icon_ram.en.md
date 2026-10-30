# Icon RAM cost

**English** · [Türkçe](icon_ram.md)

Arena column: bytes the icon took in Rumy's 20 KB icon cache (excluding the file header).
LVGL default: bytes needed if the same icon were decoded to full ARGB8888 by LVGL's built-in decoder.
Accessories are drawn line by line straight from flash; they cost no RAM.

Since v1.1.0 the weather icons are embedded in flash as well and the RAM cache is gone; this table is kept as the measurement that led to that change.

| Size | Icon | Format | File (B) | Arena (B) | LVGL default (B) |
|---:|---|---|---:|---:|---:|
| 64 | clear-day | I8 | 5132 | 5120 | 16384 |
| 64 | clear-night | I8 | 5132 | 5120 | 16384 |
| 64 | partly-cloudy-day | I8 | 5132 | 5120 | 16384 |
| 64 | partly-cloudy-night | I8 | 5132 | 5120 | 16384 |
| 64 | overcast | I8 | 5132 | 5120 | 16384 |
| 64 | fog | I8 | 5132 | 5120 | 16384 |
| 64 | drizzle | I8 | 5132 | 5120 | 16384 |
| 64 | rain | I8 | 5132 | 5120 | 16384 |
| 64 | sleet | I8 | 5132 | 5120 | 16384 |
| 64 | snow | I8 | 5132 | 5120 | 16384 |
| 64 | showers-day | I8 | 5132 | 5120 | 16384 |
| 64 | showers-night | I8 | 5132 | 5120 | 16384 |
| 64 | thunderstorm | I8 | 5132 | 5120 | 16384 |
| 64 | hail | I8 | 5132 | 5120 | 16384 |
| 64 | sunrise | I8 | 5132 | 5120 | 16384 |
| 64 | sunset | I8 | 5132 | 5120 | 16384 |
| 48 | clear-day | I8 | 3340 | 3328 | 9216 |
| 48 | clear-night | I8 | 3340 | 3328 | 9216 |
| 48 | partly-cloudy-day | I8 | 3340 | 3328 | 9216 |
| 48 | partly-cloudy-night | I8 | 3340 | 3328 | 9216 |
| 48 | overcast | I8 | 3340 | 3328 | 9216 |
| 48 | fog | I8 | 3340 | 3328 | 9216 |
| 48 | drizzle | I8 | 3340 | 3328 | 9216 |
| 48 | rain | I8 | 3340 | 3328 | 9216 |
| 48 | sleet | I8 | 3340 | 3328 | 9216 |
| 48 | snow | I8 | 3340 | 3328 | 9216 |
| 48 | showers-day | I8 | 3340 | 3328 | 9216 |
| 48 | showers-night | I8 | 3340 | 3328 | 9216 |
| 48 | thunderstorm | I8 | 3340 | 3328 | 9216 |
| 48 | hail | I8 | 3340 | 3328 | 9216 |
| 48 | sunrise | I8 | 3340 | 3328 | 9216 |
| 48 | sunset | I8 | 3340 | 3328 | 9216 |
| 28 | clear-day | I4 | 468 | 456 | 3136 |
| 28 | clear-night | I4 | 468 | 456 | 3136 |
| 28 | partly-cloudy-day | I4 | 468 | 456 | 3136 |
| 28 | partly-cloudy-night | I4 | 468 | 456 | 3136 |
| 28 | overcast | I4 | 468 | 456 | 3136 |
| 28 | fog | I4 | 468 | 456 | 3136 |
| 28 | drizzle | I4 | 468 | 456 | 3136 |
| 28 | rain | I4 | 468 | 456 | 3136 |
| 28 | sleet | I4 | 468 | 456 | 3136 |
| 28 | snow | I4 | 468 | 456 | 3136 |
| 28 | showers-day | I4 | 468 | 456 | 3136 |
| 28 | showers-night | I4 | 468 | 456 | 3136 |
| 28 | thunderstorm | I4 | 468 | 456 | 3136 |
| 28 | hail | I4 | 468 | 456 | 3136 |
| 28 | sunrise | I4 | 468 | 456 | 3136 |
| 28 | sunset | I4 | 468 | 456 | 3136 |
| 24 | clear-day | I4 | 364 | 352 | 2304 |
| 24 | clear-night | I4 | 364 | 352 | 2304 |
| 24 | partly-cloudy-day | I4 | 364 | 352 | 2304 |
| 24 | partly-cloudy-night | I4 | 364 | 352 | 2304 |
| 24 | overcast | I4 | 364 | 352 | 2304 |
| 24 | fog | I4 | 364 | 352 | 2304 |
| 24 | drizzle | I4 | 364 | 352 | 2304 |
| 24 | rain | I4 | 364 | 352 | 2304 |
| 24 | sleet | I4 | 364 | 352 | 2304 |
| 24 | snow | I4 | 364 | 352 | 2304 |
| 24 | showers-day | I4 | 364 | 352 | 2304 |
| 24 | showers-night | I4 | 364 | 352 | 2304 |
| 24 | thunderstorm | I4 | 364 | 352 | 2304 |
| 24 | hail | I4 | 364 | 352 | 2304 |
| 24 | sunrise | I4 | 364 | 352 | 2304 |
| 24 | sunset | I4 | 364 | 352 | 2304 |
| 32 | ACC_FLAG_IMG | I4 (flash) | 588 | 0 | 4096 |
| 32 | ACC_PARTY_IMG | I4 (flash) | 588 | 0 | 4096 |
| 32 | ACC_UMBRELLA_IMG | I4 (flash) | 588 | 0 | 4096 |

| Size | Most per icon (B) |
|---:|---:|
| 64 | 5120 |
| 48 | 3328 |
| 28 | 456 |
| 24 | 352 |
