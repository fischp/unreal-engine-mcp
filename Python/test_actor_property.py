"""
Simple test script for get_actor_property and set_actor_property commands.
Run this while UE4 editor is open with the UnrealMCP plugin loaded.
"""

import socket
import json

UNREAL_HOST = "127.0.0.1"
UNREAL_PORT = 55557

def send_command(command_type: str, params: dict) -> dict:
    """Send a command to Unreal and get the response."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(10)

    try:
        sock.connect((UNREAL_HOST, UNREAL_PORT))

        # Build command
        command = {
            "type": command_type,
            "params": params
        }

        # Serialize to JSON and send raw (no length prefix)
        json_str = json.dumps(command)
        data = json_str.encode('utf-8')
        sock.sendall(data)

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
                    return json.loads(response_data.decode('utf-8'))
                except json.JSONDecodeError:
                    continue  # Keep receiving
            except socket.timeout:
                break

        if response_data:
            return json.loads(response_data.decode('utf-8'))
        return {"error": "No response received"}

    except socket.timeout:
        return {"error": "Connection timed out"}
    except ConnectionRefusedError:
        return {"error": "Connection refused - is Unreal Editor running with the MCP plugin?"}
    except Exception as e:
        return {"error": str(e)}
    finally:
        sock.close()

def main():
    print("=" * 60)
    print("Testing get_actor_property and set_actor_property commands")
    print("=" * 60)

    # First, get list of actors to find one to test with
    print("\n1. Getting actors in level...")
    result = send_command("get_actors_in_level", {})

    if "error" in result:
        print(f"   Error: {result['error']}")
        return

    # Handle the nested response structure
    if result.get("status") == "success":
        actors = result.get("result", {}).get("actors", [])
    else:
        actors = result.get("actors", [])

    print(f"   Found {len(actors)} actors")

    if not actors:
        print("   No actors found in level. Add some actors first.")
        return

    # Find a good test actor (prefer StaticMeshActor or PointLight)
    test_actor = None
    for actor in actors:
        name = actor.get("name", "")
        actor_class = actor.get("class", "")
        if "StaticMeshActor" in actor_class or "PointLight" in actor_class:
            test_actor = name
            print(f"   Using actor: {name} ({actor_class})")
            break

    if not test_actor:
        # Just use the first actor
        test_actor = actors[0].get("name")
        print(f"   Using first actor: {test_actor}")

    # Test get_actor_property with bHidden
    print(f"\n2. Getting 'bHidden' property from {test_actor}...")
    result = send_command("get_actor_property", {
        "name": test_actor,
        "property": "bHidden"
    })
    print(f"   Result: {json.dumps(result, indent=2)}")

    # Handle nested result structure
    if result.get("status") == "success":
        inner_result = result.get("result", {})
        success = inner_result.get("success", False)
        original_value = inner_result.get("value")
    else:
        success = result.get("success", False)
        original_value = result.get("value")

    if success:
        # Test set_actor_property - toggle bHidden
        new_value = not original_value
        print(f"\n3. Setting 'bHidden' to {new_value}...")
        result = send_command("set_actor_property", {
            "name": test_actor,
            "property": "bHidden",
            "value": new_value
        })
        print(f"   Result: {json.dumps(result, indent=2)}")

        # Check success in nested structure
        if result.get("status") == "success":
            inner_result = result.get("result", {})
            set_success = inner_result.get("success", False)
        else:
            set_success = result.get("success", False)

        if set_success:
            print(f"\n   SUCCESS! Actor should now be {'hidden' if new_value else 'visible'} in the viewport.")
            print(f"   Check the Unreal Editor viewport to verify.")

            # Set it back
            print(f"\n4. Setting 'bHidden' back to {original_value}...")
            result = send_command("set_actor_property", {
                "name": test_actor,
                "property": "bHidden",
                "value": original_value
            })
            print(f"   Result: {json.dumps(result, indent=2)}")

    print("\n" + "=" * 60)
    print("Test complete!")
    print("=" * 60)

if __name__ == "__main__":
    main()
