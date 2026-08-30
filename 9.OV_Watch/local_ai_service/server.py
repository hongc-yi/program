"""Local STT/Chat API for OV-Watch development."""
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import cgi
import json
import os
import tempfile
import threading
from datetime import datetime
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen
from urllib.parse import urlparse

try:
    from pypinyin import lazy_pinyin
except ImportError:
    lazy_pinyin = None



def load_dotenv(path):
    if not os.path.exists(path):
        return
    with open(path, "r", encoding="utf-8") as env_file:
        for line in env_file:
            line = line.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            key, value = line.split("=", 1)
            os.environ.setdefault(key.strip(), value.strip().strip("\\\"'"))


load_dotenv(os.path.join(os.path.dirname(__file__), ".env"))
HOST = os.environ.get("OVWATCH_HOST", "127.0.0.1")
PORT = int(os.environ.get("OVWATCH_PORT", "8765"))
DEEPSEEK_API_KEY = os.environ.get("DEEPSEEK_API_KEY", "")
DEEPSEEK_BASE_URL = os.environ.get("DEEPSEEK_BASE_URL", "https://api.deepseek.com")
DEEPSEEK_MODEL = os.environ.get("DEEPSEEK_MODEL", "deepseek-chat")
SYSTEM_PROMPT = os.environ.get(
    "OVWATCH_SYSTEM_PROMPT", "你是 OV-Watch 手表的本地 AI 助手，请用简洁、准确的中文回答。"
)
# Keep the wire payload below the STM32 256-byte receive line limit.
MAX_WIRE_PINYIN_LENGTH = int(os.environ.get("OVWATCH_MAX_PINYIN_LENGTH", "230"))
MAX_HISTORY_TURNS = int(os.environ.get("OVWATCH_MAX_HISTORY_TURNS", "10"))
chat_histories = {}
chat_history_lock = threading.Lock()


def json_bytes(payload):
    return json.dumps(payload, ensure_ascii=False).encode("utf-8")


def handle_stt(audio_path):
    raise RuntimeError("STT is disabled; use the VOFA ai: text command")


def handle_chat(body):
    message = body.get("message", "") if isinstance(body, dict) else ""
    session_id = body.get("session_id", "default") if isinstance(body, dict) else "default"
    if not isinstance(session_id, str) or not session_id.strip():
        session_id = "default"
    if not isinstance(message, str) or not message.strip():
        raise ValueError("message must be a non-empty string")
    if not DEEPSEEK_API_KEY:
        raise RuntimeError("DEEPSEEK_API_KEY is not configured")

    now = datetime.now().astimezone()
    current_time = now.strftime("%Y年%m月%d日 %H:%M:%S")
    system_prompt = (
        SYSTEM_PROMPT
        + "\\n当前电脑本地时间为：%s（请将其视为北京时间 UTC+8）。"
        "当用户询问现在时间、日期或星期时，请直接依据这个时间回答，不要说无法获取。"
        % current_time
    )
    with chat_history_lock:
        history = list(chat_histories.get(session_id.strip(), []))
    messages = [{"role": "system", "content": system_prompt}] + history
    messages.append({"role": "user", "content": message.strip()})
    payload = json.dumps({
        "model": DEEPSEEK_MODEL,
        "messages": messages,
        "user": session_id.strip(),
        "temperature": 0.7,
        "max_tokens": 600,
    }).encode("utf-8")
    request = Request(
        DEEPSEEK_BASE_URL.rstrip("/") + "/chat/completions",
        data=payload,
        headers={
            "Authorization": "Bearer " + DEEPSEEK_API_KEY,
            "Content-Type": "application/json",
        },
        method="POST",
    )
    try:
        with urlopen(request, timeout=30) as response:
            result = json.loads(response.read().decode("utf-8"))
    except HTTPError as exc:
        detail = exc.read().decode("utf-8", errors="replace")[:500]
        raise RuntimeError("DeepSeek HTTP %s: %s" % (exc.code, detail))
    except URLError as exc:
        raise RuntimeError("DeepSeek connection failed: %s" % exc.reason)

    choices = result.get("choices", [])
    if not choices or not choices[0].get("message", {}).get("content"):
        raise RuntimeError("DeepSeek returned no reply")
    reply = choices[0]["message"]["content"].strip()
    if lazy_pinyin is None:
        raise RuntimeError("pypinyin is not installed; run: python -m pip install pypinyin")
    pinyin = " ".join(lazy_pinyin(reply))
    with chat_history_lock:
        turns = chat_histories.setdefault(session_id.strip(), [])
        turns.extend([
            {"role": "user", "content": message.strip()},
            {"role": "assistant", "content": reply},
        ])
        del turns[:-MAX_HISTORY_TURNS * 2]
    if len(pinyin) > MAX_WIRE_PINYIN_LENGTH:
        pinyin = pinyin[:MAX_WIRE_PINYIN_LENGTH].rstrip()
    return {
        "ok": True,
        "reply": reply,
        "pinyin": pinyin,
        "engine": DEEPSEEK_MODEL,
    }


class Handler(BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):
        print("[%s] %s" % (self.log_date_time_string(), fmt % args))

    def send_json(self, status, payload):
        data = json_bytes(payload)
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(data)

    def do_OPTIONS(self):
        self.send_response(204)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()

    def do_GET(self):
        path = urlparse(self.path).path
        if path == "/health":
            self.send_json(200, {"ok": True, "service": "ov-watch-local-ai"})
            return
        self.send_json(404, {"ok": False, "error": "not found"})

    def do_POST(self):
        path = urlparse(self.path).path
        length = int(self.headers.get("Content-Length", "0"))
        try:
            if path == "/api/stt":
                content_type = self.headers.get("Content-Type", "")
                if content_type.startswith("multipart/form-data"):
                    form = cgi.FieldStorage(
                        fp=self.rfile,
                        headers=self.headers,
                        environ={"REQUEST_METHOD": "POST", "CONTENT_TYPE": content_type},
                    )
                    if "audio" not in form:
                        raise ValueError("multipart field 'audio' is required")
                    item = form["audio"]
                    suffix = os.path.splitext(item.filename or "audio.wav")[1] or ".wav"
                    with tempfile.NamedTemporaryFile(delete=False, suffix=suffix) as audio_file:
                        audio_file.write(item.file.read())
                        temp_path = audio_file.name
                    try:
                        result = handle_stt(temp_path)
                    finally:
                        os.unlink(temp_path)
                elif content_type.startswith("application/octet-stream") or content_type.startswith("audio/"):
                    suffix = ".mp3" if content_type.startswith("audio/mpeg") else ".wav"
                    with tempfile.NamedTemporaryFile(delete=False, suffix=suffix) as audio_file:
                        audio_file.write(self.rfile.read(length))
                        temp_path = audio_file.name
                    try:
                        result = handle_stt(temp_path)
                    finally:
                        os.unlink(temp_path)
                else:
                    raise ValueError("send WAV as multipart field 'audio' or raw audio body")
            else:
                raw = self.rfile.read(length)
                body = json.loads(raw.decode("utf-8")) if raw else {}
            if path == "/api/chat":
                result = handle_chat(body)
            elif path != "/api/stt":
                self.send_json(404, {"ok": False, "error": "not found"})
                return
            self.send_json(200, result)
        except (ValueError, json.JSONDecodeError) as exc:
            self.send_json(400, {"ok": False, "error": str(exc)})
        except Exception as exc:
            self.send_json(500, {"ok": False, "error": str(exc)})


if __name__ == "__main__":
    server = ThreadingHTTPServer((HOST, PORT), Handler)
    print("OV-Watch local AI service: http://%s:%d" % (HOST, PORT))
    print("Endpoints: GET /health, POST /api/stt, POST /api/chat")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
