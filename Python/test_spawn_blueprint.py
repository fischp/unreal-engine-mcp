import socket
import json

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.settimeout(10)

try:
    print("Connecting...")
    sock.connect(("127.0.0.1", 55557))
    print("Connected!")

    # Test spawn_blueprint_actor command
    command = {
        "type": "spawn_blueprint_actor",
        "params": {
            "blueprint_name": "/Game/Blueprints/SVGLegionSpawn_BP.SVGLegionSpawn_BP",
            "actor_name": "TestLegionSpawn_MCP",
            "location": [0, 0, 100],
            "rotation": [0, 0, 0]
        }
    }
    json_str = json.dumps(command)
    data = json_str.encode('utf-8')

    print(f"Sending: {json_str}")
    sock.sendall(data)
    print("Sent! Waiting for response...")

    # Receive response
    response_data = b''
    while True:
        try:
            chunk = sock.recv(8192)
            if not chunk:
                break
            response_data += chunk
            try:
                json.loads(response_data.decode('utf-8'))
                break
            except json.JSONDecodeError:
                continue
        except socket.timeout:
            break

    if response_data:
        response = json.loads(response_data.decode('utf-8'))
        print(f"Response: {json.dumps(response, indent=2)}")
    else:
        print("No response received")

except Exception as e:
    print(f"Error: {e}")
finally:
    sock.close()
