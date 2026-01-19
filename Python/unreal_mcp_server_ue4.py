"""
Unreal Engine 4.27 MCP Server

A simplified MCP server for UE4.27 with 13 core tools for editor control and scene manipulation.
"""

import logging
import socket
import json
import struct
import time
import threading
from contextlib import asynccontextmanager
from typing import AsyncIterator, Dict, Any, Optional, List
from mcp.server.fastmcp import FastMCP

# Configure logging
logging.basicConfig(
    level=logging.DEBUG,
    format='%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] - %(message)s',
    handlers=[
        logging.FileHandler('unreal_mcp_ue4.log'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger("UnrealMCP_UE4")

# Configuration
UNREAL_HOST = "127.0.0.1"
UNREAL_PORT = 55557


class UnrealConnection:
    """
    Robust connection to Unreal Engine 4.27 with automatic retry and reconnection.
    """

    MAX_RETRIES = 3
    BASE_RETRY_DELAY = 0.5
    MAX_RETRY_DELAY = 5.0
    CONNECT_TIMEOUT = 10
    DEFAULT_RECV_TIMEOUT = 30
    BUFFER_SIZE = 8192

    def __init__(self):
        self.socket = None
        self.connected = False
        self._lock = threading.RLock()
        self._last_error = None

    def _create_socket(self) -> socket.socket:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(self.CONNECT_TIMEOUT)
        sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 131072)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 131072)
        try:
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack('hh', 1, 0))
        except OSError:
            pass
        return sock

    def connect(self) -> bool:
        for attempt in range(self.MAX_RETRIES + 1):
            with self._lock:
                self._close_socket_unsafe()
                try:
                    logger.info(f"Connecting to Unreal at {UNREAL_HOST}:{UNREAL_PORT} (attempt {attempt + 1}/{self.MAX_RETRIES + 1})...")
                    self.socket = self._create_socket()
                    self.socket.connect((UNREAL_HOST, UNREAL_PORT))
                    self.connected = True
                    self._last_error = None
                    logger.info("Successfully connected to Unreal Engine 4.27")
                    return True
                except socket.timeout as e:
                    self._last_error = f"Connection timeout: {e}"
                    logger.warning(f"Connection timeout (attempt {attempt + 1})")
                except ConnectionRefusedError as e:
                    self._last_error = f"Connection refused: {e}"
                    logger.warning(f"Connection refused - is Unreal Engine running? (attempt {attempt + 1})")
                except OSError as e:
                    self._last_error = f"OS error: {e}"
                    logger.warning(f"OS error during connection: {e} (attempt {attempt + 1})")
                except Exception as e:
                    self._last_error = f"Unexpected error: {e}"
                    logger.error(f"Unexpected connection error: {e} (attempt {attempt + 1})")
                self._close_socket_unsafe()
                self.connected = False

            if attempt < self.MAX_RETRIES:
                delay = min(self.BASE_RETRY_DELAY * (2 ** attempt), self.MAX_RETRY_DELAY)
                logger.info(f"Retrying connection in {delay:.1f}s...")
                time.sleep(delay)

        logger.error(f"Failed to connect after {self.MAX_RETRIES + 1} attempts. Last error: {self._last_error}")
        return False

    def _close_socket_unsafe(self):
        if self.socket:
            try:
                self.socket.shutdown(socket.SHUT_RDWR)
            except:
                pass
            try:
                self.socket.close()
            except:
                pass
            self.socket = None
        self.connected = False

    def disconnect(self):
        with self._lock:
            self._close_socket_unsafe()
            logger.debug("Disconnected from Unreal Engine")

    def _receive_response(self, command_type: str) -> bytes:
        timeout = self.DEFAULT_RECV_TIMEOUT
        self.socket.settimeout(timeout)

        chunks = []
        total_bytes = 0
        start_time = time.time()

        try:
            while True:
                elapsed = time.time() - start_time
                if elapsed > timeout:
                    raise socket.timeout(f"Overall timeout after {elapsed:.1f}s")

                try:
                    chunk = self.socket.recv(self.BUFFER_SIZE)
                except socket.timeout:
                    if chunks:
                        data = b''.join(chunks)
                        try:
                            json.loads(data.decode('utf-8'))
                            logger.info(f"Got complete response after recv timeout ({total_bytes} bytes)")
                            return data
                        except json.JSONDecodeError:
                            pass
                    raise

                if not chunk:
                    if not chunks:
                        raise ConnectionError("Connection closed before receiving any data")
                    break

                chunks.append(chunk)
                total_bytes += len(chunk)

                data = b''.join(chunks)
                try:
                    decoded = data.decode('utf-8')
                    json.loads(decoded)
                    logger.info(f"Received complete response ({total_bytes} bytes) for {command_type}")
                    return data
                except json.JSONDecodeError:
                    continue
                except UnicodeDecodeError:
                    continue

        except socket.timeout:
            elapsed = time.time() - start_time
            if chunks:
                data = b''.join(chunks)
                try:
                    json.loads(data.decode('utf-8'))
                    logger.warning(f"Using response received before timeout ({total_bytes} bytes)")
                    return data
                except:
                    pass
            raise TimeoutError(f"Timeout after {elapsed:.1f}s waiting for response to {command_type} (received {total_bytes} bytes)")

        if chunks:
            data = b''.join(chunks)
            try:
                json.loads(data.decode('utf-8'))
                return data
            except:
                raise ConnectionError(f"Connection closed with incomplete data ({total_bytes} bytes)")

        raise ConnectionError("Connection closed without response")

    def send_command(self, command: str, params: Dict[str, Any] = None) -> Optional[Dict[str, Any]]:
        last_error = None

        for attempt in range(self.MAX_RETRIES + 1):
            try:
                return self._send_command_once(command, params, attempt)
            except (ConnectionError, TimeoutError, socket.error, OSError) as e:
                last_error = str(e)
                logger.warning(f"Command failed (attempt {attempt + 1}/{self.MAX_RETRIES + 1}): {e}")
                self.disconnect()

                if attempt < self.MAX_RETRIES:
                    delay = min(self.BASE_RETRY_DELAY * (2 ** attempt), self.MAX_RETRY_DELAY)
                    logger.info(f"Retrying command in {delay:.1f}s...")
                    time.sleep(delay)
            except Exception as e:
                logger.error(f"Unexpected error sending command: {e}")
                self.disconnect()
                return {"status": "error", "error": str(e)}

        return {"status": "error", "error": f"Command failed after {self.MAX_RETRIES + 1} attempts: {last_error}"}

    def _send_command_once(self, command: str, params: Dict[str, Any], attempt: int) -> Dict[str, Any]:
        with self._lock:
            if not self.connect():
                raise ConnectionError(f"Failed to connect to Unreal Engine: {self._last_error}")

            try:
                command_obj = {
                    "type": command,
                    "params": params or {}
                }
                command_json = json.dumps(command_obj)

                logger.info(f"Sending command (attempt {attempt + 1}): {command}")
                logger.debug(f"Command payload: {command_json[:500]}...")

                self.socket.settimeout(10)
                self.socket.sendall(command_json.encode('utf-8'))

                response_data = self._receive_response(command)

                try:
                    response = json.loads(response_data.decode('utf-8'))
                except json.JSONDecodeError as e:
                    logger.error(f"JSON decode error: {e}")
                    logger.debug(f"Raw response: {response_data[:500]}")
                    raise ValueError(f"Invalid JSON response: {e}")

                logger.info(f"Command {command} completed successfully")

                if response.get("status") == "error":
                    error_msg = response.get("error") or response.get("message", "Unknown error")
                    logger.warning(f"Unreal returned error: {error_msg}")
                elif response.get("success") is False:
                    error_msg = response.get("error") or response.get("message", "Unknown error")
                    response = {"status": "error", "error": error_msg}
                    logger.warning(f"Unreal returned failure: {error_msg}")

                return response

            finally:
                self._close_socket_unsafe()


# Global connection instance
_unreal_connection: Optional[UnrealConnection] = None
_connection_lock = threading.Lock()


def get_unreal_connection() -> UnrealConnection:
    global _unreal_connection
    with _connection_lock:
        if _unreal_connection is None:
            logger.info("Creating new UnrealConnection instance")
            _unreal_connection = UnrealConnection()
        return _unreal_connection


def reset_unreal_connection():
    global _unreal_connection
    with _connection_lock:
        if _unreal_connection:
            _unreal_connection.disconnect()
            _unreal_connection = None
        logger.info("Unreal connection reset")


@asynccontextmanager
async def server_lifespan(server: FastMCP) -> AsyncIterator[Dict[str, Any]]:
    logger.info("UnrealMCP UE4.27 server starting up")
    logger.info("Connection will be established lazily on first tool call")
    try:
        yield {}
    finally:
        reset_unreal_connection()
        logger.info("Unreal MCP UE4.27 server shut down")


# Initialize server
mcp = FastMCP(
    "UnrealMCP_UE4",
    lifespan=server_lifespan
)


# ============================================================================
# Tool 1: Get Unreal Engine Path
# ============================================================================
@mcp.tool()
def get_unreal_engine_path() -> Dict[str, Any]:
    """Get the current Unreal Engine installation path and version."""
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("get_unreal_engine_path", {})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_unreal_engine_path error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 2: Get Unreal Project Path
# ============================================================================
@mcp.tool()
def get_unreal_project_path() -> Dict[str, Any]:
    """Get the current Unreal Project path and name."""
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("get_unreal_project_path", {})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_unreal_project_path error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 3: Editor Console Command
# ============================================================================
@mcp.tool()
def editor_console_command(command: str) -> Dict[str, Any]:
    """Run a console command in the Unreal Editor.

    Args:
        command: The console command to execute (e.g., "stat fps", "show collision")
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("editor_console_command", {"command": command})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"editor_console_command error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 4: Editor Project Info
# ============================================================================
@mcp.tool()
def editor_project_info() -> Dict[str, Any]:
    """Get detailed information about the current project."""
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("editor_project_info", {})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"editor_project_info error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 5: Editor Get Map Info
# ============================================================================
@mcp.tool()
def editor_get_map_info() -> Dict[str, Any]:
    """Get detailed information about the current map/level."""
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("editor_get_map_info", {})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"editor_get_map_info error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 6: Editor Search Assets
# ============================================================================
@mcp.tool()
def editor_search_assets(
    pattern: str,
    class_filter: str = "",
    max_results: int = 100
) -> Dict[str, Any]:
    """Search for assets by name or path with optional class filter.

    Args:
        pattern: Search pattern to match asset names/paths
        class_filter: Optional class name to filter results (e.g., "StaticMesh", "Material")
        max_results: Maximum number of results to return (default 100)
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "pattern": pattern,
            "class_filter": class_filter,
            "max_results": max_results
        }
        response = unreal.send_command("editor_search_assets", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"editor_search_assets error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 7: Editor Get World Outliner (Get Actors in Level)
# ============================================================================
@mcp.tool()
def editor_get_world_outliner() -> Dict[str, Any]:
    """Get all actors in the current world with their properties."""
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("get_actors_in_level", {})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"editor_get_world_outliner error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 8: Editor Validate Assets
# ============================================================================
@mcp.tool()
def editor_validate_assets(asset_path: str = "") -> Dict[str, Any]:
    """Validate assets in the project to check for errors.

    Args:
        asset_path: Optional path to validate specific assets (empty for all)
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("editor_validate_assets", {"asset_path": asset_path})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"editor_validate_assets error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 9: Editor Create Object (Spawn Actor)
# ============================================================================
@mcp.tool()
def editor_create_object(
    name: str,
    actor_type: str,
    location: List[float] = None,
    rotation: List[float] = None,
    scale: List[float] = None,
    static_mesh: str = ""
) -> Dict[str, Any]:
    """Create a new object/actor in the world.

    Args:
        name: Unique name for the actor
        actor_type: Type of actor (StaticMeshActor, PointLight, SpotLight, DirectionalLight, CameraActor)
        location: [X, Y, Z] position in the world
        rotation: [Pitch, Yaw, Roll] rotation in degrees
        scale: [X, Y, Z] scale factors
        static_mesh: Path to static mesh asset (for StaticMeshActor)
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "name": name,
            "type": actor_type
        }
        if location:
            params["location"] = location
        if rotation:
            params["rotation"] = rotation
        if scale:
            params["scale"] = scale
        if static_mesh:
            params["static_mesh"] = static_mesh

        response = unreal.send_command("spawn_actor", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"editor_create_object error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 10: Editor Update Object (Set Actor Transform)
# ============================================================================
@mcp.tool()
def editor_update_object(
    name: str,
    location: List[float] = None,
    rotation: List[float] = None,
    scale: List[float] = None
) -> Dict[str, Any]:
    """Update an existing object/actor's transform in the world.

    Args:
        name: Name of the actor to update
        location: New [X, Y, Z] position
        rotation: New [Pitch, Yaw, Roll] rotation in degrees
        scale: New [X, Y, Z] scale factors
    """
    unreal = get_unreal_connection()
    try:
        params = {"name": name}
        if location is not None:
            params["location"] = location
        if rotation is not None:
            params["rotation"] = rotation
        if scale is not None:
            params["scale"] = scale

        response = unreal.send_command("set_actor_transform", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"editor_update_object error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 11: Editor Delete Object
# ============================================================================
@mcp.tool()
def editor_delete_object(name: str) -> Dict[str, Any]:
    """Delete an object/actor from the world.

    Args:
        name: Name of the actor to delete
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("delete_actor", {"name": name})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"editor_delete_object error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 12: Editor Take Screenshot
# ============================================================================
@mcp.tool()
def editor_take_screenshot(filename: str = "") -> Dict[str, Any]:
    """Take a screenshot of the Unreal Editor viewport.

    Args:
        filename: Optional filename (without extension). If empty, uses timestamp.
    """
    unreal = get_unreal_connection()
    try:
        params = {}
        if filename:
            params["filename"] = filename
        response = unreal.send_command("editor_take_screenshot", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"editor_take_screenshot error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 13: Editor Move Camera
# ============================================================================
@mcp.tool()
def editor_move_camera(
    location: List[float] = None,
    rotation: List[float] = None
) -> Dict[str, Any]:
    """Move the viewport camera to a specific location and rotation.

    Args:
        location: [X, Y, Z] position for the camera
        rotation: [Pitch, Yaw, Roll] rotation for the camera in degrees
    """
    unreal = get_unreal_connection()
    try:
        params = {}
        if location is not None:
            params["location"] = location
        if rotation is not None:
            params["rotation"] = rotation

        response = unreal.send_command("editor_move_camera", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"editor_move_camera error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Bonus: Find Actors By Name (enhanced search)
# ============================================================================
@mcp.tool()
def find_actors_by_name(pattern: str) -> Dict[str, Any]:
    """Find actors by name pattern.

    Args:
        pattern: Pattern to match actor names (case-sensitive substring match)
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("find_actors_by_name", {"pattern": pattern})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"find_actors_by_name error: {e}")
        return {"success": False, "message": str(e)}


# Entry point
if __name__ == "__main__":
    import asyncio

    print("=" * 60)
    print("Unreal Engine 4.27 MCP Server")
    print("=" * 60)
    print(f"Listening for MCP connections...")
    print(f"Will connect to Unreal Engine at {UNREAL_HOST}:{UNREAL_PORT}")
    print()
    print("Available tools:")
    print("  1. get_unreal_engine_path")
    print("  2. get_unreal_project_path")
    print("  3. editor_console_command")
    print("  4. editor_project_info")
    print("  5. editor_get_map_info")
    print("  6. editor_search_assets")
    print("  7. editor_get_world_outliner")
    print("  8. editor_validate_assets")
    print("  9. editor_create_object")
    print(" 10. editor_update_object")
    print(" 11. editor_delete_object")
    print(" 12. editor_take_screenshot")
    print(" 13. editor_move_camera")
    print(" 14. find_actors_by_name (bonus)")
    print("=" * 60)

    mcp.run()
