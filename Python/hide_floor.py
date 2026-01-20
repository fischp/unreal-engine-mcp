import socket
import json
import time

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
    return response.decode() if response else None

print("Hiding Floor for 10 seconds...")
send_command('set_actor_property', {'name': 'Floor', 'property': 'bHidden', 'value': True})
print("Floor is now HIDDEN - watch the viewport!")

time.sleep(10)

print("Making Floor visible again...")
send_command('set_actor_property', {'name': 'Floor', 'property': 'bHidden', 'value': False})
print("Floor is now VISIBLE again!")
