"""Explicit control for bridge-baud-1 only. No game data uses this socket."""
import secrets, socket, struct, zlib

def set_baud(host, baud, reset=False):
    if baud not in (115200,230400,518400,691200,1036800,2073600): raise ValueError('Unsupported baud')
    head=struct.pack('<4s3I',b'PSB1',secrets.randbits(32),baud,int(reset))
    frame=head+struct.pack('<I',zlib.crc32(head))
    with socket.socket(socket.AF_INET,socket.SOCK_DGRAM) as control:
        control.connect((host,3334)); control.settimeout(.5)
        for _ in range(6):
            control.send(frame)
            try: reply=control.recv(64)
            except TimeoutError: continue
            if reply==frame: return baud
    raise TimeoutError('Bridge did not acknowledge baud; do not send game data')
