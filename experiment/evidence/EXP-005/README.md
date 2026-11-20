# EXP-005 Evidence Index

- Experiment: EXP-005 — MOSFET driver module assembly and preliminary wiring
- Experiment date: 2026-11-19
- Status: original image files uploaded; detailed photo-to-evidence mapping is partially pending.

## Uploaded original image files

The following 10 JPG files are present in this directory on the `main` branch:

| File | Capture timestamp inferred from filename | Status |
|---|---|---|
| `IMG_20261109_174544.jpg` | 2026-11-09 17:45:44 | Uploaded; content-to-event mapping pending |
| `IMG_20261109_174550.jpg` | 2026-11-09 17:45:50 | Uploaded; content-to-event mapping pending |
| `IMG_20261109_183936.jpg` | 2026-11-09 18:39:36 | Uploaded; content-to-event mapping pending |
| `IMG_20261114_113048.jpg` | 2026-11-14 11:30:48 | Uploaded; content-to-event mapping pending |
| `IMG_20261114_193135.jpg` | 2026-11-14 19:31:35 | Uploaded; likely related to the previously discussed PIR evidence; exact mapping requires visual confirmation |
| `IMG_20261118_181921.jpg` | 2026-11-18 18:19:21 | Uploaded; likely historical breadboard/sensor setup; exact mapping requires visual confirmation |
| `IMG_20261119_095756.jpg` | 2026-11-19 09:57:56 | Uploaded; content-to-event mapping pending |
| `IMG_20261119_175636.jpg` | 2026-11-19 17:56:36 | Uploaded; content-to-event mapping pending |
| `IMG_20261119_175650.jpg` | 2026-11-19 17:56:50 | Uploaded; content-to-event mapping pending |
| `_cgi-bin_mmwebwx-bin_webwxgetmsgimg__&MsgID=7263387889681310891&skey=@crypt_220c0a15_b198ba3680cbc8a026ab7936aa9468e4&mmweb_appid=wx_webfilehelper.jpg` | Not reliably inferable from filename | Uploaded; filename retained from source; content/date mapping pending |
| `_cgi-bin_mmwebwx-bin_webwxgetmsgimg__&MsgID=8964425806593660555&skey=@crypt_220c0a15_b198ba3680cbc8a026ab7936aa9468e4&mmweb_appid=wx_webfilehelper.jpg` | Not reliably inferable from filename | Uploaded; filename retained from source; content/date mapping pending |

## Evidence register

| Evidence ID | Intended evidence description | File mapping |
|---|---|---|
| EXP005-PHOTO-001 | PIR module close-up; white Fresnel lens, two adjustment potentiometers and mode jumper; a separate board marked “Water Sensor” appears in the background | Candidate: `IMG_20261114_193135.jpg`; visual confirmation pending |
| EXP005-PHOTO-002 | Overall assembly view showing ESP32-S3, DHT11/PIR modules, fan and driver module | Candidate among 2026-11-19 images; exact mapping pending |
| EXP005-PHOTO-003 | MOSFET driver terminal and conductor close-up | Candidate among 2026-11-19 images; exact mapping pending |
| EXP005-PHOTO-004 | Fan rear switch and cable | Candidate among 2026-11-19 images; exact mapping pending |
| EXP005-PHOTO-005 | ESP32-S3 board and connected jumper leads | Candidate among 2026-11-19 images; exact mapping pending |
| EXP005-PHOTO-006 | Earlier breadboard/sensor arrangement | Candidate: `IMG_20261118_181921.jpg`; visual confirmation pending |

## Sensor inventory supported by current records

- DHT11 temperature/humidity module: previously tested on GPIO4; see EXP-001.
- PIR motion sensor module: photographed previously; appearance is consistent with an HC-SR501-style module, but exact model and electrical specifications are unconfirmed.
- A separate red board marked “Water Sensor” is visible in the previously discussed 2026-11-14 photograph. It is recorded as an observed component only, not as an integrated SmartFan sensor.

## Evidence integrity and naming notes

- The image directory contains 10 JPG files. The original files are retained; no image content has been edited by this index update.
- The two files beginning `_cgi-bin_mmwebwx-bin_webwxgetmsgimg__` have source-generated filenames that are not suitable as long-term evidence identifiers. They are intentionally left unchanged until their contents and provenance are confirmed; rename them only after recording the mapping.
- Timestamps above are inferred from filenames, not independently verified EXIF metadata.
- The six evidence IDs preserve the earlier conversation-based descriptions. Exact file-to-ID mapping remains pending where explicitly stated.
- SHA-256 hashes have not yet been recorded in this index.

## Safety / verification boundary

The photographed assembly is not proof of electrical continuity, correct terminal topology, safe polarity, or successful operation. Keep the circuit unpowered until the checks listed in the EXP-005 experiment record are completed.
