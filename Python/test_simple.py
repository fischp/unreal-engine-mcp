import socket
import json

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.settimeout(10)

try:
    print("Connecting...")
    sock.connect(("127.0.0.1", 55557))
    print("Connected!")

    # Send a simple command (raw JSON, no length prefix)
    command = {"type": "get_actors_in_level", "params": {}}
    json_str = json.dumps(command)
    data = json_str.encode('utf-8')

    print(f"Sending: {json_str}")
    sock.sendall(data)
    print("Sent! Waiting for response...")

    # Receive response (raw JSON, no length prefix)
    response_data = b''
    while True:
        try:
            chunk = sock.recv(8192)
            if not chunk:
                break
            response_data += chunk
            # Try to parse as JSON - if successful, we have a complete message
            try:
                json.loads(response_data.decode('utf-8'))
                break  # Complete JSON received
            except json.JSONDecodeError:
                continue  # Keep receiving
        except socket.timeout:
            break

    if response_data:
        print(f"Response: {response_data.decode('utf-8')}")
    else:
        print("No response received")

except Exception as e:
    print(f"Error: {e}")
finally:
    sock.close()
