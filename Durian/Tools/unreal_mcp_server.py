"""Small stdio MCP bridge for Unreal Engine Remote Control API.

This intentionally uses only the Python standard library.
"""
import json
import sys
import urllib.error
import urllib.parse
import urllib.request

BASE = "http://127.0.0.1:30010"


def http(method, path, body=None):
    data = None if body is None else json.dumps(body).encode("utf-8")
    req = urllib.request.Request(
        BASE + path,
        data=data,
        method=method,
        headers={"Content-Type": "application/json"},
    )
    with urllib.request.urlopen(req, timeout=5) as response:
        raw = response.read().decode("utf-8")
        try:
            return json.loads(raw)
        except json.JSONDecodeError:
            return raw


TOOLS = [
    {
        "name": "unreal_status",
        "description": "Check whether Unreal Engine Remote Control API is reachable.",
        "inputSchema": {"type": "object", "properties": {}},
    },
    {
        "name": "unreal_get_preset",
        "description": "Read a Remote Control Preset from Unreal Engine.",
        "inputSchema": {
            "type": "object",
            "properties": {"preset": {"type": "string"}},
            "required": ["preset"],
        },
    },
    {
        "name": "unreal_set_property",
        "description": "Set an exposed Unreal object property through Remote Control.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "object_path": {"type": "string"},
                "property_name": {"type": "string"},
                "property_value": {},
            },
            "required": ["object_path", "property_name", "property_value"],
        },
    },
]


def result(value):
    return {"content": [{"type": "text", "text": json.dumps(value, ensure_ascii=False, indent=2)}]}


def handle(message):
    method = message.get("method")
    params = message.get("params") or {}
    if method == "initialize":
        return {
            "protocolVersion": "2024-11-05",
            "capabilities": {"tools": {}},
            "serverInfo": {"name": "durian-unreal", "version": "0.1.0"},
        }
    if method == "notifications/initialized":
        return None
    if method == "tools/list":
        return {"tools": TOOLS}
    if method != "tools/call":
        return {}

    name = params.get("name")
    args = params.get("arguments") or {}
    try:
        if name == "unreal_status":
            return result(http("GET", "/remote/info"))
        if name == "unreal_get_preset":
            preset = urllib.parse.quote(args["preset"], safe="")
            return result(http("GET", f"/remote/preset/{preset}"))
        if name == "unreal_set_property":
            body = {
                "ObjectPath": args["object_path"],
                "PropertyName": args["property_name"],
                "PropertyValue": args["property_value"],
            }
            return result(http("PUT", "/remote/object/property", body))
        raise ValueError(f"Unknown tool: {name}")
    except Exception as exc:
        return {"isError": True, "content": [{"type": "text", "text": str(exc)}]}


for line in sys.stdin:
    try:
        message = json.loads(line)
        response = handle(message)
        if "id" in message and response is not None:
            print(json.dumps({"jsonrpc": "2.0", "id": message["id"], "result": response}), flush=True)
    except Exception as exc:
        if "id" in locals().get("message", {}):
            print(json.dumps({"jsonrpc": "2.0", "id": message["id"], "error": {"code": -32000, "message": str(exc)}}), flush=True)
