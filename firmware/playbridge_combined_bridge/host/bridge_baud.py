"""PSB1 baud/abort control client; game data never uses this UDP socket."""

import secrets
import socket
import struct
import zlib


SUPPORTED_BAUD = (115200, 230400, 518400, 691200, 1036800, 2073600)


def set_baud(host: str, baud: int, reset: bool = False) -> int:
    if baud not in SUPPORTED_BAUD:
        raise ValueError("Unsupported baud")
    head = struct.pack("<4s3I", b"PSB1", secrets.randbits(32), baud, int(reset))
    frame = head + struct.pack("<I", zlib.crc32(head) & 0xFFFFFFFF)
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as control:
        control.connect((host, 3334))
        control.settimeout(0.5)
        for _ in range(6):
            control.send(frame)
            try:
                reply = control.recv(64)
            except TimeoutError:
                continue
            if reply == frame:
                return baud
    raise TimeoutError("Bridge did not acknowledge baud; do not send game data")
