import socket
import json
import time

def send_command(command):
    """Send a command and receive response"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(10)
    sock.connect(("127.0.0.1", 55557))

    try:
        json_str = json.dumps(command)
        data = json_str.encode('utf-8')
        print(f"Sending: {command.get('type', 'unknown')}")
        sock.sendall(data)

        # Receive response
        response_data = b''
        while True:
            try:
                chunk = sock.recv(8192)
                if not chunk:
                    break
                response_data += chunk
                try:
                    result = json.loads(response_data.decode('utf-8'))
                    return result
                except json.JSONDecodeError:
                    continue
            except socket.timeout:
                break

        if response_data:
            return json.loads(response_data.decode('utf-8'))
        return None
    finally:
        sock.close()

def move_camera(location, rotation):
    """Move the editor camera"""
    command = {
        "type": "editor_move_camera",
        "params": {
            "location": location,
            "rotation": rotation
        }
    }
    result = send_command(command)
    print(f"  Camera result: {result}")
    return result

def take_screenshot(filename):
    """Take a screenshot"""
    command = {
        "type": "editor_take_screenshot",
        "params": {
            "filename": filename
        }
    }
    result = send_command(command)
    print(f"  Screenshot result: {result}")
    return result

def main():
    print("=" * 50)
    print("Taking photos of the shelter structure")
    print("=" * 50)

    # Structure is centered at X=500, Y=0
    structure_center_x = 500
    structure_center_y = 0

    print("\n1. Exterior view from front-right...")
    move_camera(
        location=[structure_center_x + 600, structure_center_y - 400, 200],
        rotation=[-15, 145, 0]  # Pitch down, looking toward structure
    )
    time.sleep(1)
    take_screenshot("shelter_exterior_front")

    print("\n2. Side view...")
    move_camera(
        location=[structure_center_x, structure_center_y - 600, 200],
        rotation=[-10, 90, 0]  # Looking sideways at structure
    )
    time.sleep(1)
    take_screenshot("shelter_side_view")

    print("\n3. Interior view (inside the shelter)...")
    move_camera(
        location=[structure_center_x, structure_center_y, 100],  # Inside at eye level
        rotation=[-5, 45, 0]  # Looking diagonally
    )
    time.sleep(1)
    take_screenshot("shelter_interior")

    print("\n4. Top-down overview...")
    move_camera(
        location=[structure_center_x, structure_center_y, 600],  # High above
        rotation=[-80, 0, 0]  # Looking down
    )
    time.sleep(1)
    take_screenshot("shelter_overview")

    print("\n" + "=" * 50)
    print("Photos complete! Check the Screenshots folder.")
    print("=" * 50)

if __name__ == "__main__":
    main()
