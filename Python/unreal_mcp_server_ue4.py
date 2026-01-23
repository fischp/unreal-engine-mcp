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
async def server_lifespan(app: FastMCP):
    logger.info("UnrealMCP UE4.27 server starting up")
    logger.info("Connection will be established lazily on first tool call")
    try:
        yield
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
# Tool 12: Get Actor Property
# ============================================================================
@mcp.tool()
def get_actor_property(name: str, property: str) -> Dict[str, Any]:
    """Get a property value from an actor.

    Args:
        name: Name of the actor
        property: Name of the property to get
    """
    unreal = get_unreal_connection()
    try:
        params = {"name": name, "property": property}
        response = unreal.send_command("get_actor_property", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_actor_property error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 13: Set Actor Property
# ============================================================================
@mcp.tool()
def set_actor_property(name: str, property: str, value: Any) -> Dict[str, Any]:
    """Set a property value on an actor.

    Args:
        name: Name of the actor
        property: Name of the property to set
        value: The value to set (type depends on property)
    """
    unreal = get_unreal_connection()
    try:
        params = {"name": name, "property": property, "value": value}
        response = unreal.send_command("set_actor_property", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"set_actor_property error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 14: Get Asset Property
# ============================================================================
@mcp.tool()
def get_asset_property(asset_path: str, property: str) -> Dict[str, Any]:
    """Get a property value from a Data Asset or any UObject asset.

    Args:
        asset_path: Path to the asset (e.g., "/Game/Data/DA_GameDifficultySettings")
        property: Name of the property to get
    """
    unreal = get_unreal_connection()
    try:
        params = {"asset_path": asset_path, "property": property}
        response = unreal.send_command("get_asset_property", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_asset_property error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 15: Set Asset Property
# ============================================================================
@mcp.tool()
def set_asset_property(asset_path: str, property: str, value: Any) -> Dict[str, Any]:
    """Set a property value on a Data Asset or any UObject asset.

    Args:
        asset_path: Path to the asset (e.g., "/Game/Data/DA_GameDifficultySettings")
        property: Name of the property to set
        value: The value to set (type depends on property - supports arrays, maps, structs)
    """
    unreal = get_unreal_connection()
    try:
        params = {"asset_path": asset_path, "property": property, "value": value}
        response = unreal.send_command("set_asset_property", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"set_asset_property error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 16: List Data Table Rows
# ============================================================================
@mcp.tool()
def list_data_table_rows(data_table_path: str) -> Dict[str, Any]:
    """List all row names in a Data Table.

    Args:
        data_table_path: Path to the Data Table asset (e.g., "/Game/DataAssets/DT_AllPlayerItems")
    """
    unreal = get_unreal_connection()
    try:
        params = {"data_table_path": data_table_path}
        response = unreal.send_command("list_data_table_rows", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"list_data_table_rows error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 17: Get Data Table Row
# ============================================================================
@mcp.tool()
def get_data_table_row(data_table_path: str, row_name: str) -> Dict[str, Any]:
    """Get a specific row from a Data Table.

    Args:
        data_table_path: Path to the Data Table asset
        row_name: Name of the row to retrieve
    """
    unreal = get_unreal_connection()
    try:
        params = {"data_table_path": data_table_path, "row_name": row_name}
        response = unreal.send_command("get_data_table_row", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_data_table_row error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 18: Set Data Table Row Field
# ============================================================================
@mcp.tool()
def set_data_table_row_field(data_table_path: str, row_name: str, field_name: str, value: Any) -> Dict[str, Any]:
    """Set a field value in a Data Table row.

    Args:
        data_table_path: Path to the Data Table asset
        row_name: Name of the row to modify
        field_name: Name of the field to set
        value: The value to set (supports arrays, maps, structs)
    """
    unreal = get_unreal_connection()
    try:
        params = {"data_table_path": data_table_path, "row_name": row_name, "field_name": field_name, "value": value}
        response = unreal.send_command("set_data_table_row_field", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"set_data_table_row_field error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 19: Add Data Table Row
# ============================================================================
@mcp.tool()
def add_data_table_row(data_table_path: str, row_name: str) -> Dict[str, Any]:
    """Add a new row to a Data Table with default values.

    Args:
        data_table_path: Path to the Data Table asset
        row_name: Name for the new row
    """
    unreal = get_unreal_connection()
    try:
        params = {"data_table_path": data_table_path, "row_name": row_name}
        response = unreal.send_command("add_data_table_row", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"add_data_table_row error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 20: Delete Data Table Row
# ============================================================================
@mcp.tool()
def delete_data_table_row(data_table_path: str, row_name: str) -> Dict[str, Any]:
    """Delete a row from a Data Table.

    Args:
        data_table_path: Path to the Data Table asset
        row_name: Name of the row to delete
    """
    unreal = get_unreal_connection()
    try:
        params = {"data_table_path": data_table_path, "row_name": row_name}
        response = unreal.send_command("delete_data_table_row", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"delete_data_table_row error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 20b: Set Data Table Array Element
# ============================================================================
@mcp.tool()
def set_data_table_array_element(
    data_table_path: str,
    row_name: str,
    array_field: str,
    element_index: int,
    element_field: str,
    value: Any
) -> Dict[str, Any]:
    """Set a specific field within an array element in a Data Table row.

    This is useful for modifying a single field within a struct that is inside an array,
    without having to replace the entire array.

    Args:
        data_table_path: Path to the Data Table asset (e.g., "/Game/DataTables/DT_MyTable")
        row_name: Name of the row containing the array
        array_field: Name of the array field in the row struct
        element_index: Zero-based index of the element within the array
        element_field: Name of the field within the struct element to modify
        value: New value for the field

    Example:
        set_data_table_array_element(
            "/Game/DataAssets/RTS_Stuff/DT_RTSButtonsInfos_Paul",
            "BlackSmith",
            "Buttons",
            3,
            "ButtonAction",
            "UpgradeSeparateStructure"
        )
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "data_table_path": data_table_path,
            "row_name": row_name,
            "array_field": array_field,
            "element_index": element_index,
            "element_field": element_field,
            "value": value
        }
        response = unreal.send_command("set_data_table_array_element", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"set_data_table_array_element error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 21: Spawn Blueprint Actor
# ============================================================================
@mcp.tool()
def spawn_blueprint_actor(
    blueprint_path: str,
    actor_name: str,
    location: List[float] = None,
    rotation: List[float] = None
) -> Dict[str, Any]:
    """Spawn an actor from a Blueprint class.

    Args:
        blueprint_path: Full path to Blueprint (e.g., "/Game/Blueprints/BP_MyActor.BP_MyActor")
        actor_name: Unique name for the new actor
        location: Optional [X, Y, Z] spawn location
        rotation: Optional [Pitch, Yaw, Roll] spawn rotation
    """
    unreal = get_unreal_connection()
    try:
        params = {"blueprint_path": blueprint_path, "actor_name": actor_name}
        if location:
            params["location"] = location
        if rotation:
            params["rotation"] = rotation
        response = unreal.send_command("spawn_blueprint_actor", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"spawn_blueprint_actor error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 22: Copy Actor
# ============================================================================
@mcp.tool()
def copy_actor(
    source_name: str,
    new_name: str,
    location: List[float] = None,
    rotation: List[float] = None
) -> Dict[str, Any]:
    """Copy an existing actor to create a new one.

    Args:
        source_name: Name of the actor to copy
        new_name: Name for the new copied actor
        location: Optional [X, Y, Z] location for the copy
        rotation: Optional [Pitch, Yaw, Roll] rotation for the copy
    """
    unreal = get_unreal_connection()
    try:
        params = {"source_name": source_name, "new_name": new_name}
        if location:
            params["location"] = location
        if rotation:
            params["rotation"] = rotation
        response = unreal.send_command("copy_actor", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"copy_actor error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 23: Get Blueprint Default Property
# ============================================================================
@mcp.tool()
def get_blueprint_default_property(blueprint_path: str, property: str) -> Dict[str, Any]:
    """Get a default property value from a Blueprint's Class Default Object (CDO).

    Args:
        blueprint_path: Path to the Blueprint asset
        property: Name of the property to get
    """
    unreal = get_unreal_connection()
    try:
        params = {"blueprint_path": blueprint_path, "property": property}
        response = unreal.send_command("get_blueprint_default_property", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_blueprint_default_property error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 24: Set Blueprint Default Property
# ============================================================================
@mcp.tool()
def set_blueprint_default_property(blueprint_path: str, property: str, value: Any) -> Dict[str, Any]:
    """Set a default property value on a Blueprint's Class Default Object (CDO).

    Args:
        blueprint_path: Path to the Blueprint asset
        property: Name of the property to set
        value: The value to set (supports arrays, maps, structs)
    """
    unreal = get_unreal_connection()
    try:
        params = {"blueprint_path": blueprint_path, "property": property, "value": value}
        response = unreal.send_command("set_blueprint_default_property", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"set_blueprint_default_property error: {e}")
        return {"success": False, "message": str(e)}


# ============================================================================
# Tool 25: Editor Take Screenshot
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


# ============================================================================
# Widget Blueprint Tools
# ============================================================================

@mcp.tool()
def create_widget_blueprint(
    name: str,
    path: str = "/Game/Widgets"
) -> Dict[str, Any]:
    """Create a new Widget Blueprint asset.

    Args:
        name: Name for the new Widget Blueprint
        path: Content folder path for the asset (default: /Game/Widgets)
    """
    unreal = get_unreal_connection()
    try:
        params = {"name": name, "path": path}
        response = unreal.send_command("create_widget_blueprint", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"create_widget_blueprint error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def add_widget_to_blueprint(
    blueprint_path: str,
    widget_type: str,
    widget_name: str,
    parent_widget: str = ""
) -> Dict[str, Any]:
    """Add a widget to an existing Widget Blueprint.

    Args:
        blueprint_path: Path to the Widget Blueprint asset
        widget_type: Type of widget (Button, TextBlock, Image, CanvasPanel,
                     VerticalBox, HorizontalBox, Border, Overlay, SizeBox,
                     ScrollBox, Spacer)
        widget_name: Name for the new widget
        parent_widget: Optional name of parent widget (if empty, adds to root)
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "blueprint_path": blueprint_path,
            "widget_type": widget_type,
            "widget_name": widget_name,
            "parent_widget": parent_widget
        }
        response = unreal.send_command("add_widget_to_blueprint", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"add_widget_to_blueprint error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def list_widget_blueprints(path: str = "/Game") -> Dict[str, Any]:
    """List all Widget Blueprints in the project.

    Args:
        path: Content path to search (default: /Game for entire project)
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("list_widget_blueprints", {"path": path})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"list_widget_blueprints error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def get_widget_hierarchy(blueprint_path: str) -> Dict[str, Any]:
    """Get the widget hierarchy tree of a Widget Blueprint.

    Args:
        blueprint_path: Path to the Widget Blueprint asset
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("get_widget_hierarchy", {"blueprint_path": blueprint_path})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_widget_hierarchy error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def get_widget_properties(blueprint_path: str, widget_name: str) -> Dict[str, Any]:
    """Get properties of a specific widget in a Widget Blueprint.

    Args:
        blueprint_path: Path to the Widget Blueprint asset
        widget_name: Name of the widget to inspect
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "blueprint_path": blueprint_path,
            "widget_name": widget_name
        }
        response = unreal.send_command("get_widget_properties", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"get_widget_properties error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def set_widget_properties(
    blueprint_path: str,
    widget_name: str,
    text: str = None,
    font_size: int = None,
    visibility: str = None,
    position: List[float] = None,
    size: List[float] = None,
    anchors: List[float] = None
) -> Dict[str, Any]:
    """Set properties of a widget in a Widget Blueprint.

    Args:
        blueprint_path: Path to the Widget Blueprint asset
        widget_name: Name of the widget to modify
        text: Text content (for TextBlock widgets)
        font_size: Font size (for TextBlock widgets)
        visibility: Visibility state (Visible, Hidden, Collapsed, HitTestInvisible, SelfHitTestInvisible)
        position: [X, Y] position in pixels (for widgets in CanvasPanel)
        size: [Width, Height] in pixels (for widgets in CanvasPanel)
        anchors: [MinX, MinY, MaxX, MaxY] anchor values 0-1 (for widgets in CanvasPanel)
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "blueprint_path": blueprint_path,
            "widget_name": widget_name
        }
        if text is not None:
            params["text"] = text
        if font_size is not None:
            params["font_size"] = font_size
        if visibility is not None:
            params["visibility"] = visibility
        if position is not None:
            params["position"] = position
        if size is not None:
            params["size"] = size
        if anchors is not None:
            params["anchors"] = anchors

        response = unreal.send_command("set_widget_properties", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"set_widget_properties error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def rename_widget(
    blueprint_path: str,
    widget_name: str,
    new_name: str
) -> Dict[str, Any]:
    """Rename a widget in a Widget Blueprint.

    Args:
        blueprint_path: Path to the Widget Blueprint asset
        widget_name: Current name of the widget
        new_name: New name for the widget
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "blueprint_path": blueprint_path,
            "widget_name": widget_name,
            "new_name": new_name
        }
        response = unreal.send_command("rename_widget", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"rename_widget error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def reparent_widget(
    blueprint_path: str,
    widget_name: str,
    new_parent: str
) -> Dict[str, Any]:
    """Move a widget to a new parent in the hierarchy.

    Args:
        blueprint_path: Path to the Widget Blueprint asset
        widget_name: Name of the widget to move
        new_parent: Name of the new parent panel widget
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "blueprint_path": blueprint_path,
            "widget_name": widget_name,
            "new_parent": new_parent
        }
        response = unreal.send_command("reparent_widget", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"reparent_widget error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def remove_widget_from_blueprint(
    blueprint_path: str,
    widget_name: str
) -> Dict[str, Any]:
    """Remove a widget from a Widget Blueprint.

    Args:
        blueprint_path: Path to the Widget Blueprint asset
        widget_name: Name of the widget to remove
    """
    unreal = get_unreal_connection()
    try:
        params = {
            "blueprint_path": blueprint_path,
            "widget_name": widget_name
        }
        response = unreal.send_command("remove_widget_from_blueprint", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"remove_widget_from_blueprint error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def delete_widget_blueprint(asset_path: str) -> Dict[str, Any]:
    """Delete a Widget Blueprint asset from the project.

    Args:
        asset_path: Full path to the Widget Blueprint asset to delete
    """
    unreal = get_unreal_connection()
    try:
        response = unreal.send_command("delete_widget_blueprint", {"asset_path": asset_path})
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"delete_widget_blueprint error: {e}")
        return {"success": False, "message": str(e)}


@mcp.tool()
def show_widget(blueprint_path: str, z_order: int = 0) -> Dict[str, Any]:
    """Display a Widget Blueprint on screen during Play in Editor (PIE).

    Note: This command only works when a Play session is active.
    Press Play in the editor first, then call this to show the widget.

    Args:
        blueprint_path: Path to the Widget Blueprint asset (e.g., /Game/Widgets/TestUI)
        z_order: Z-order for layering, higher values appear on top (default: 0)
    """
    unreal = get_unreal_connection()
    try:
        params = {"blueprint_path": blueprint_path, "z_order": z_order}
        response = unreal.send_command("show_widget", params)
        return response or {"success": False, "message": "No response from Unreal"}
    except Exception as e:
        logger.error(f"show_widget error: {e}")
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
    print("  Editor Tools:")
    print("    - get_unreal_engine_path")
    print("    - get_unreal_project_path")
    print("    - editor_console_command")
    print("    - editor_project_info")
    print("    - editor_get_map_info")
    print("    - editor_search_assets")
    print("    - editor_validate_assets")
    print("    - editor_take_screenshot")
    print("    - editor_move_camera")
    print("    - find_actors_by_name")
    print()
    print("  Widget Blueprint Tools:")
    print("    - create_widget_blueprint")
    print("    - add_widget_to_blueprint")
    print("    - list_widget_blueprints")
    print("    - get_widget_hierarchy")
    print("    - get_widget_properties")
    print("    - set_widget_properties")
    print("    - rename_widget")
    print("    - reparent_widget")
    print("    - remove_widget_from_blueprint")
    print("    - delete_widget_blueprint")
    print("    - show_widget")
    print("=" * 60)

    mcp.run()
