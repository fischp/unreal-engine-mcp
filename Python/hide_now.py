import socket
import json

s = socket.socket()
s.settimeout(10)
s.connect(('127.0.0.1', 55557))
s.sendall(json.dumps({'type': 'set_actor_property', 'params': {'name': 'Floor', 'property': 'bHidden', 'value': True}}).encode())
print('Floor is now hidden')
s.close()
