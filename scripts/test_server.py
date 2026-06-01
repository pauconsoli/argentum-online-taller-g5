#!/usr/bin/env python3
"""
Test manual del protocolo cliente-servidor.
Corre el servidor primero:  ./build/taller_server 8080
Luego ejecuta:              python3 scripts/test_server.py
"""

import socket
import struct
import sys

HOST = "localhost"
PORT = 8080

# ── Opcodes (de protocol_constants.h) ─────────────────────────────────────────
CLIENT_LOGIN         = 0x01
CLIENT_LIST_MATCHES  = 0x02
CLIENT_CREATE_MATCH  = 0x03
CLIENT_JOIN_MATCH    = 0x04
CLIENT_DISCONNECT    = 0xFF

SERVER_LOGIN_OK      = 0x81
SERVER_MATCH_LIST    = 0x82
SERVER_MATCH_CREATED = 0x83
SERVER_MATCH_JOINED  = 0x84
SERVER_ERROR         = 0xEE

# ── Helpers de serialización ───────────────────────────────────────────────────
def pack_string(s: str) -> bytes:
    encoded = s.encode()
    return struct.pack(">H", len(encoded)) + encoded

def recv_exact(s: socket.socket, n: int) -> bytes:
    data = b""
    while len(data) < n:
        chunk = s.recv(n - len(data))
        if not chunk:
            raise ConnectionError("Server cerró la conexión")
        data += chunk
    return data

def recv_u8(s):  return struct.unpack("B", recv_exact(s, 1))[0]
def recv_u16(s): return struct.unpack(">H", recv_exact(s, 2))[0]
def recv_u32(s): return struct.unpack(">I", recv_exact(s, 4))[0]
def recv_string(s):
    length = recv_u16(s)
    return recv_exact(s, length).decode()

# ── Tests ──────────────────────────────────────────────────────────────────────
def test_login(s: socket.socket, nick: str) -> int:
    print(f"\n[TEST] LOGIN nick='{nick}'")
    payload = bytes([CLIENT_LOGIN]) + pack_string(nick)
    s.sendall(payload)

    opcode = recv_u8(s)
    if opcode == SERVER_LOGIN_OK:
        player_id = recv_u32(s)
        print(f"  ✓ LOGIN_OK  player_id={player_id}")
        return player_id
    elif opcode == SERVER_ERROR:
        error_code = recv_u8(s)
        msg = recv_string(s)
        print(f"  ✗ ERROR  code=0x{error_code:02x}  msg='{msg}'")
        return -1
    else:
        print(f"  ✗ Opcode inesperado: 0x{opcode:02x}")
        return -1

def test_list_matches(s: socket.socket):
    print("\n[TEST] LIST_MATCHES")
    s.sendall(bytes([CLIENT_LIST_MATCHES]))

    opcode = recv_u8(s)
    if opcode == SERVER_MATCH_LIST:
        count = recv_u16(s)
        print(f"  ✓ MATCH_LIST  count={count}")
        for _ in range(count):
            match_id      = recv_u32(s)
            name          = recv_string(s)
            current       = recv_u8(s)
            max_players   = recv_u8(s)
            print(f"    - [{match_id}] '{name}'  {current}/{max_players} jugadores")
        return count
    else:
        print(f"  ✗ Opcode inesperado: 0x{opcode:02x}")
        return -1

def test_create_match(s: socket.socket, name: str, max_players: int) -> int:
    print(f"\n[TEST] CREATE_MATCH name='{name}' max={max_players}")
    payload = bytes([CLIENT_CREATE_MATCH]) + pack_string(name) + bytes([max_players])
    s.sendall(payload)

    opcode = recv_u8(s)
    if opcode == SERVER_MATCH_CREATED:
        match_id = recv_u32(s)
        print(f"  ✓ MATCH_CREATED  match_id={match_id}")
        return match_id
    elif opcode == SERVER_ERROR:
        error_code = recv_u8(s)
        msg = recv_string(s)
        print(f"  ✗ ERROR  code=0x{error_code:02x}  msg='{msg}'")
        return -1
    else:
        print(f"  ✗ Opcode inesperado: 0x{opcode:02x}")
        return -1

def test_join_match(s: socket.socket, match_id: int):
    print(f"\n[TEST] JOIN_MATCH match_id={match_id}")
    payload = bytes([CLIENT_JOIN_MATCH]) + struct.pack(">I", match_id)
    s.sendall(payload)

    opcode = recv_u8(s)
    if opcode == SERVER_MATCH_JOINED:
        mid      = recv_u32(s)
        pid      = recv_u32(s)
        print(f"  ✓ MATCH_JOINED  match_id={mid}  player_id={pid}")
        return True
    elif opcode == SERVER_ERROR:
        error_code = recv_u8(s)
        msg = recv_string(s)
        print(f"  ✗ ERROR  code=0x{error_code:02x}  msg='{msg}'")
        return False
    else:
        print(f"  ✗ Opcode inesperado: 0x{opcode:02x}")
        return False

# ── Main ───────────────────────────────────────────────────────────────────────
def main():
    print(f"Conectando a {HOST}:{PORT}...")
    try:
        s = socket.create_connection((HOST, PORT), timeout=5)
    except ConnectionRefusedError:
        print("ERROR: No se pudo conectar. ¿El servidor está corriendo?")
        print(f"  Inicialo con:  ./build/taller_server {PORT}")
        sys.exit(1)

    print("Conectado.")

    try:
        # Flujo completo: login → listar → crear → listar de nuevo → unirse
        pid = test_login(s, "tester")
        if pid < 0:
            return

        test_list_matches(s)

        match_id = test_create_match(s, "sala_test", 4)
        if match_id < 0:
            return

        test_list_matches(s)

        # Segundo cliente en otra conexión para unirse
        print("\n[TEST] Segundo cliente se une al match")
        s2 = socket.create_connection((HOST, PORT), timeout=5)
        try:
            pid2 = test_login(s2, "tester2")
            if pid2 >= 0:
                test_join_match(s2, match_id)
        finally:
            s2.sendall(bytes([CLIENT_DISCONNECT]))
            s2.close()

    except ConnectionError as e:
        print(f"\nConexión cortada: {e}")
    finally:
        s.sendall(bytes([CLIENT_DISCONNECT]))
        s.close()
        print("\nTest finalizado.")

if __name__ == "__main__":
    main()
