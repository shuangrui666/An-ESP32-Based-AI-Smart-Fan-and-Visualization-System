# EXP-004 Wi-Fi / HTTP Upload Retest Evidence

## Overview

- Experiment ID: EXP-004
- Retest date: 2026-11-19
- Device: ESP32
- Backend: Flask
- Upload endpoint: `/api/sensor/data`
- Status endpoint: `/api/status`

## Evidence Files

| File | Description |
|---|---|
| api_status.json | First API status snapshot |
| api_status_second.json | Second API status snapshot |
| api_status_final.json | Final API status snapshot |
| esp32_serial.log | ESP32 serial capture |
| SHA256SUMS.csv | SHA256 manifest |
| RETEST_REPORT.md | Detailed retest report |

## Observations

The three API snapshots reported 116, 124, and 161 data points respectively. All three reported the system as online.

The ESP32 serial log records Wi-Fi connection success, IP address `192.168.137.171`, RSSI `-26 dBm`, DHT11 readings, and firmware messages indicating successful data transmission.

## Integrity

All four original evidence files were copied to this repository directory and passed SHA256 comparison against their source files.

## Limitations

The API snapshots do not independently identify the source of every data point.

A server-side POST access log for this specific retest has not been separately archived. The firmware upload-success message is not independent proof of the server-side HTTP response code.

The API field `wifi_signal: 0` has not been interpreted as RSSI. The ESP32 serial log separately reports `-26 dBm`.
