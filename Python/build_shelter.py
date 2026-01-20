import socket
import json
import time

def send_command(sock, command):
    """Send a command and receive response"""
    json_str = json.dumps(command)
    data = json_str.encode('utf-8')

    print(f"Sending: {command.get('type', 'unknown')} - {command.get('params', {}).get('name', '')}")
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

def create_connection():
    """Create a new socket connection"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(10)
    sock.connect(("127.0.0.1", 55557))
    return sock

def spawn_actor(name, location, scale, mesh_path):
    """Spawn a static mesh actor"""
    sock = create_connection()
    try:
        command = {
            "type": "spawn_actor",
            "params": {
                "type": "StaticMeshActor",
                "name": name,
                "location": location,
                "scale": scale,
                "static_mesh": mesh_path
            }
        }
        result = send_command(sock, command)
        print(f"  Result: {result}")
        return result
    finally:
        sock.close()

def take_screenshot(filename):
    """Take a screenshot"""
    sock = create_connection()
    try:
        command = {
            "type": "take_screenshot",
            "params": {
                "filename": filename
            }
        }
        result = send_command(sock, command)
        print(f"Screenshot result: {result}")
        return result
    finally:
        sock.close()

def move_camera(location, rotation):
    """Move the editor camera"""
    sock = create_connection()
    try:
        command = {
            "type": "set_editor_camera",
            "params": {
                "location": location,
                "rotation": rotation
            }
        }
        result = send_command(sock, command)
        print(f"Camera result: {result}")
        return result
    finally:
        sock.close()

def main():
    print("=" * 50)
    print("Building a walkable shelter structure")
    print("=" * 50)

    # Basic shape paths
    CUBE = "/Engine/BasicShapes/Cube"
    CYLINDER = "/Engine/BasicShapes/Cylinder"

    # Structure parameters
    # Floor center at X=500, floor is 400x400 units
    floor_center_x = 500
    floor_center_y = 0
    floor_size = 400  # Total floor size
    pillar_inset = 150  # How far pillars are from center
    pillar_height = 300
    roof_height = pillar_height + 10  # Roof sits on top of pillars

    print("\n1. Spawning Floor...")
    spawn_actor(
        name="Shelter_Floor",
        location=[floor_center_x, floor_center_y, 5],
        scale=[4.0, 4.0, 0.1],  # 400x400x10 units (cube is 100 units by default)
        mesh_path=CUBE
    )
    time.sleep(0.5)

    print("\n2. Spawning Pillars...")
    # Pillar positions at corners (relative to floor center)
    pillar_positions = [
        ("Shelter_Pillar_NE", [floor_center_x + pillar_inset, floor_center_y + pillar_inset, pillar_height / 2]),
        ("Shelter_Pillar_NW", [floor_center_x - pillar_inset, floor_center_y + pillar_inset, pillar_height / 2]),
        ("Shelter_Pillar_SE", [floor_center_x + pillar_inset, floor_center_y - pillar_inset, pillar_height / 2]),
        ("Shelter_Pillar_SW", [floor_center_x - pillar_inset, floor_center_y - pillar_inset, pillar_height / 2]),
    ]

    for name, location in pillar_positions:
        spawn_actor(
            name=name,
            location=location,
            scale=[0.3, 0.3, 3.0],  # Thin and tall cylinder
            mesh_path=CYLINDER
        )
        time.sleep(0.3)

    print("\n3. Spawning Roof...")
    spawn_actor(
        name="Shelter_Roof",
        location=[floor_center_x, floor_center_y, roof_height],
        scale=[4.5, 4.5, 0.2],  # Slightly larger than floor for overhang
        mesh_path=CUBE
    )
    time.sleep(0.5)

    print("\n4. Moving camera to view structure...")
    # Position camera to see the structure from an angle
    move_camera(
        location=[0, -500, 300],  # Behind and to the side
        rotation=[0, -20, 30]  # Looking slightly up and toward structure
    )
    time.sleep(0.5)

    print("\n5. Taking screenshot...")
    take_screenshot("shelter_exterior")
    time.sleep(1)

    print("\n6. Moving camera inside structure...")
    move_camera(
        location=[floor_center_x, floor_center_y, 100],  # Inside the shelter
        rotation=[0, 0, 45]  # Looking out
    )
    time.sleep(0.5)

    print("\n7. Taking interior screenshot...")
    take_screenshot("shelter_interior")

    print("\n" + "=" * 50)
    print("Structure complete!")
    print("=" * 50)

if __name__ == "__main__":
    main()
