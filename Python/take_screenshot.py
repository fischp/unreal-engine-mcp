import socket
import json

def send_command(command_type, params):
    s = socket.socket()
    s.settimeout(10)
    s.connect(('127.0.0.1', 55557))
    s.sendall(json.dumps({'type': command_type, 'params': params}).encode())
    response = b''
    while True:
        try:
            chunk = s.recv(8192)
            if not chunk:
                break
            response += chunk
            try:
                json.loads(response)
                break
            except:
                continue
        except socket.timeout:
            break
    s.close()
    return json.loads(response) if response else None

# Just pass a filename, not a full path - it will save to Unreal's screenshot directory
result = send_command('editor_take_screenshot', {'filename': 'mcp_screenshot'})
print(json.dumps(result, indent=2))
