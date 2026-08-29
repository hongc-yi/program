# OV-Watch local AI service

Minimal local HTTP service for developing the future voice flow. It uses only Python 3 standard-library modules and proxies chat requests to the DeepSeek OpenAI-compatible API.

## Configure

Copy `.env.example` to `.env`, then set `DEEPSEEK_API_KEY` to your newly generated key. The key belongs in:

`D:\learn\program\9.OV_Watch\local_ai_service\.env`

Do not put the key in `server.py` or commit `.env`.

Install the only extra dependency:

```powershell
R:\Python38\python.exe -m pip install -r requirements.txt
```

## Start

```powershell
$env:PYTHONHOME = $null
$env:PYTHONPATH = $null
R:\AnacondaMini\python.exe server.py
```

Default address: `http://0.0.0.0:8765` (accessible from the local network; use the computer's LAN IP from ESP32)

Optional environment variables:

```powershell
$env:OVWATCH_HOST = "127.0.0.1"
$env:OVWATCH_PORT = "8765"
python server.py
```

## API

Health check:

```powershell
Invoke-RestMethod http://127.0.0.1:8765/health
```

Speech-to-text placeholder. The `filename` field reserves the future audio metadata shape:

```powershell
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8765/api/stt -ContentType "application/json" -Body '{"filename":"sample.wav"}'
```

Chat request:

```powershell
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8765/api/chat -ContentType "application/json" -Body '{"message":"现在几点？"}'
```

`/api/chat` calls DeepSeek and returns pinyin. Voice/STT is disabled; use the ESP32/VOFA `ai:` text command instead. Authentication is intentionally not implemented yet.
