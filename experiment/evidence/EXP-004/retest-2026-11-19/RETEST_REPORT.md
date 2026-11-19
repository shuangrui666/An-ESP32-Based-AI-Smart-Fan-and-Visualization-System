# EXP-004 Retest Report — 2026-11-19

## 1. Objective

Repeat the ESP32 Wi-Fi connectivity and sensor-data upload verification, and preserve the serial and backend API evidence.

## 2. Test Environment

| Item | Observed value |
|---|---|
| Microcontroller | ESP32 |
| Serial port | COM6 |
| Baud rate | 115200 |
| ESP32 IP address | 192.168.137.171 |
| Backend address | 192.168.137.1 |
| Backend port | 5000 |
| Upload endpoint | /api/sensor/data |
| Status endpoint | /api/status |
| Sensor | DHT11 |

## 3. API Status Observations

| Snapshot | Data points | Status | Last seen |
|---|---:|---|---|
| First | 116 | online | 2026-11-19T11:01:56.352025 |
| Second | 124 | online | 2026-11-19T11:02:37.406170 |
| Final | 161 | online | 2026-11-19T11:06:39.385918 |

The data-point count increased by 8 between the first and second snapshots, 37 between the second and final snapshots, and 45 overall.

All three API status queries returned `success` and reported the system as `online`.

The data-point count alone does not prove that every new point originated from this specific ESP32 retest.

## 4. ESP32 Serial Observations

The captured serial output includes:

- Wi-Fi connection status: Connected
- ESP32 IP address: 192.168.137.171
- ESP32-reported RSSI: -26 dBm
- DHT11 reading: 23.2 °C and 51.0% humidity
- Upload URL: http://192.168.137.1:5000/api/sensor/data
- Firmware output: Data sent successfully
- Firmware output: Sensor data uploaded successfully

Example payload:

    {"temperature":23.20,"humidity":51.00,"fan_gear":-1}

These are observations from the ESP32 serial output. A separately archived server-side POST log confirming the HTTP response code for this retest is not available.

## 5. Data Integrity

The four original evidence files were copied from the desktop evidence directory to this repository directory.

SHA256 verification confirmed that all four copied files matched their source files. Each recorded hash contains 64 hexadecimal characters.

See SHA256SUMS.csv.

## 6. Verified Results

1. ESP32 serial output reports successful Wi-Fi connection.
2. ESP32 serial output reports the backend upload destination.
3. ESP32 serial output contains sensor readings and upload-success messages.
4. Three API status queries returned success.
5. All three API snapshots reported the system as online.
6. API data-point count increased from 116 to 161.
7. All four original evidence files passed SHA256 comparison.

## 7. Limitations

The following items have not been independently verified:

- Server-side HTTP response code for each POST request during this retest.
- Origin of every data point counted by the API.
- Long-duration upload stability.
- Automatic recovery after network disconnection.
- Backend-to-ESP32 reverse command transmission.
- Actual fan control operation.

The API field `wifi_signal: 0` has not been interpreted as RSSI. The ESP32 serial log separately reports RSSI of -26 dBm.

## 8. Conclusion

The collected evidence supports ESP32 Wi-Fi connectivity, sensor readings, firmware-reported upload success, and an online backend API with increasing data-point counts.

The evidence does not independently establish the HTTP response code for each POST request or prove long-term reliability and fan-control functionality.
