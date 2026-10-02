#!/usr/bin/env python3
"""Read-only Unirom 8.0.K memory-card backup over the PlayBridge TCP bridge.

No uploads, flash commands, memory-card writes, baud changes, or automatic retries.
The console stages the card in temporary RAM before the host downloads it.
Protocol reference: NOTPSXSerial commit 63a47555353404ff067563f26529897bf84cf84c,
NOTPSXSERIAL.CS and TransferLogic.cs. See README.md for evidence and limitations.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import socket
import struct
import time
import urllib.request

CARD_SIZE = 131072
COUNTERS = ('tcpReceived', 'uartWritten', 'uartReceived', 'tcpWritten',
            'uartErrors', 'socketErrors')


def exact(stream, count, timeout=5):
    data = bytearray()
    deadline = time.monotonic() + timeout
    while len(data) < count:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            raise TimeoutError(f'Read {len(data)}/{count} bytes')
        stream.settimeout(remaining)
        part = stream.recv(count - len(data))
        if not part:
            raise ConnectionError('Bridge closed during read')
        data.extend(part)
    return bytes(data)


def handshake(stream, expected, timeout=8):
    transcript = bytearray()
    version = 1
    deadline = time.monotonic() + timeout
    while len(transcript) < 16384:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            break
        try:
            transcript.extend(exact(stream, 1, remaining))
        except TimeoutError:
            break
        tail = bytes(transcript[-4:])
        if tail in (b'UNSP', b'HECK', b'ONLY'):
            raise RuntimeError(f'Unirom rejected request: {tail!r}')
        if version == 1 and tail in (b'OKV2', b'OKV3'):
            version = int(chr(tail[-1]))
            stream.sendall(b'UPV' + tail[-1:])
        if tail == expected:
            return version
    raise TimeoutError(f'No {expected!r}; received {bytes(transcript)!r}')


def checksum(data, version):
    value = 5381 if version == 3 else 0
    for byte in data:
        value = ((((value << 5) + value) ^ byte)
                 if version == 3 else value + byte) & 0xffffffff
    return value


def download(stream, slot, progress=print):
    if slot not in (1, 2):
        raise ValueError('Physical slot must be 1 or 2')
    # In the 8.0.K-era client this is OKAY, NOT the newer client's HLTD.
    stream.sendall(b'MCDN')
    handshake(stream, b'OKAY')
    stream.sendall(struct.pack('<I', slot - 1))
    handshake(stream, b'MCRD', timeout=60)
    address, size = struct.unpack('<II', exact(stream, 8))
    if size != CARD_SIZE or not (
        0x80000000 <= address <= 0x80200000 - size or
        0xA0000000 <= address <= 0xA0200000 - size
    ):
        raise RuntimeError('Unexpected card buffer address/size; refusing RAM dump')
    stream.sendall(b'DUMP')
    version = handshake(stream, b'OKAY')
    stream.sendall(struct.pack('<II', address, size))
    payload = bytearray()
    while len(payload) < size:
        payload.extend(exact(stream, min(2048, size - len(payload))))
        # Unirom requires MORE after each 2048 bytes, including the final block.
        stream.sendall(b'MORE')
        if len(payload) % 32768 == 0:
            progress(f'Received {len(payload)}/{size} bytes')
    expected = struct.unpack('<I', exact(stream, 4))[0]
    actual = checksum(payload, version)
    if actual != expected:
        raise RuntimeError(f'Checksum mismatch: {actual:08x} != {expected:08x}; no image saved')
    return bytes(payload), {'protocol_version': version, 'checksum': f'{actual:08x}',
                            'checksum_verified': True, 'card_magic_MC': payload[:2] == b'MC'}


def bridge_status(host):
    with urllib.request.urlopen(f'http://{host}/bridge/status', timeout=3) as response:
        return json.load(response)


def require_ready(state):
    if state['active'] or state['fault'] or not state['initialized'] or state['baud'] != 115200:
        raise RuntimeError('Requires idle, healthy bridge at 115200; nothing sent')


def check_output(path):
    # Existing destination is required: a missing SD mount must not be silently created.
    if not path.parent.is_dir():
        raise RuntimeError('Output directory missing; mount the SD card/create the directory first')
    if path.exists() or path.is_symlink():
        raise FileExistsError(f'Refusing to overwrite {path}')


def save_verified(path, payload):
    check_output(path)
    digest = hashlib.sha256(payload).hexdigest()
    with path.open('xb') as output:
        output.write(payload)
        output.flush()
        os.fsync(output.fileno())
    if hashlib.sha256(path.read_bytes()).hexdigest() != digest:
        raise RuntimeError(f'File read-back failed; treat this file as unverified: {path}')
    return digest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--host', required=True, help='ESP32 IPv4 address or hostname on trusted LAN')
    parser.add_argument('--slot', required=True, type=int, choices=(1, 2))
    parser.add_argument('--output', required=True, type=Path, help='New raw .mcd file; parent must exist')
    args = parser.parse_args()
    path = args.output.expanduser().absolute()
    check_output(path)
    # Catch disappearance or replacement of a removable volume before writing.
    destination_device = path.parent.stat().st_dev
    before = bridge_status(args.host)
    require_ready(before)
    started = time.monotonic()
    with socket.create_connection((args.host, 3333), timeout=3) as stream:
        stream.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        # Separate any pre-existing TTY output from this request's handshake.
        deadline = time.monotonic() + .2
        while time.monotonic() < deadline:
            stream.settimeout(max(.001, deadline - time.monotonic()))
            try:
                if not stream.recv(4096):
                    raise ConnectionError('Bridge closed before MCDN')
            except socket.timeout:
                break
        print(f'Reading slot {args.slot}; keep console and cables connected.', flush=True)
        payload, report = download(stream, args.slot, lambda msg: print(msg, flush=True))
    # Status snapshots update every 100 ms. Diagnostics cannot veto a valid backup.
    time.sleep(.2)
    try:
        after = bridge_status(args.host)
        report['counter_deltas'] = {key: after[key] - before[key] for key in COUNTERS}
    except (OSError, ValueError, KeyError) as error:
        report['status_warning'] = str(error)
    check_output(path)
    if path.parent.stat().st_dev != destination_device:
        raise RuntimeError('Destination volume changed; no image saved')
    digest = save_verified(path, payload)
    report.update(file=str(path), slot=args.slot, bytes=len(payload), sha256=digest,
                  file_readback_verified=True, elapsed_seconds=round(time.monotonic() - started, 3))
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, KeyError, RuntimeError) as error:
        raise SystemExit(f'Backup failed: {error}. Do not blindly retry a partial protocol session; '
                         'return the PS1 to a fresh Unirom menu first.')
