"""Offline tests only: synthetic bytes, no console/network or personal save data."""
import importlib.util
from pathlib import Path
import socket
import struct
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location(
    'memcard_backup', Path(__file__).resolve().parents[1] / 'host' / 'memcard_backup.py')
backup = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(backup)


class Peer:
    def __init__(self, data, fragment=37):
        self.data = bytearray(data)
        self.sent = []
        self.fragment = fragment

    def settimeout(self, timeout):
        assert timeout > 0

    def recv(self, count):
        if not self.data:
            raise socket.timeout()
        count = min(count, self.fragment, len(self.data))
        part = self.data[:count]
        del self.data[:count]
        return bytes(part)

    def sendall(self, data):
        self.sent.append(data)


PAYLOAD = b'MC' + bytes(range(256)) * 511 + bytes(range(254))


def peer_for(version=2, size=131072, address=0x80080000, bad_checksum=False):
    hello = b'MCDNOKV' + str(version).encode() + b'OKAYMCRD'
    hello += struct.pack('<II', address, size)
    hello += b'DUMPOKV' + str(version).encode() + b'OKAY'
    check = backup.checksum(PAYLOAD, version) ^ int(bad_checksum)
    return Peer(hello + PAYLOAD + struct.pack('<I', check))


class BackupTests(unittest.TestCase):
    def test_v2_and_v3_fragmented_full_download(self):
        for version, slot in ((2, 1), (3, 2)):
            with self.subTest(version=version, slot=slot):
                peer = peer_for(version)
                data, report = backup.download(peer, slot, lambda _: None)
                self.assertEqual(data, PAYLOAD)
                self.assertEqual(len(data), 131072)
                self.assertTrue(report['checksum_verified'])
                self.assertTrue(report['card_magic_MC'])
                self.assertEqual(peer.sent[:6], [b'MCDN', f'UPV{version}'.encode(),
                    struct.pack('<I', slot - 1), b'DUMP', f'UPV{version}'.encode(),
                    struct.pack('<II', 0x80080000, 131072)])
                self.assertEqual(peer.sent[6:], [b'MORE'] * 64)
                self.assertFalse(peer.data)

    def test_known_checksum_vectors(self):
        self.assertEqual(backup.checksum(b'ABC', 2), 198)
        self.assertEqual(backup.checksum(b'', 3), 5381)
        self.assertEqual(backup.checksum(b'A', 3), (5381 * 33) ^ 65)

    def test_echo_is_not_handshake(self):
        with self.assertRaisesRegex(TimeoutError, 'MCDN'):
            backup.handshake(Peer(b'MCDN'), b'OKAY')

    def test_rejection_tokens(self):
        for token in (b'HECK', b'ONLY', b'UNSP'):
            with self.assertRaisesRegex(RuntimeError, 'rejected'):
                backup.handshake(Peer(token), b'OKAY')

    def test_corrupt_transfer_rejected(self):
        with self.assertRaisesRegex(RuntimeError, 'Checksum mismatch'):
            backup.download(peer_for(bad_checksum=True), 1, lambda _: None)

    def test_truncated_transfer_rejected(self):
        peer = peer_for()
        del peer.data[-10:]
        with self.assertRaises(TimeoutError):
            backup.download(peer, 1, lambda _: None)

    def test_wrong_generation_ack_does_not_send_slot(self):
        peer = Peer(b'MCDNHLTD')
        with self.assertRaises(TimeoutError):
            backup.download(peer, 1, lambda _: None)
        self.assertEqual(peer.sent, [b'MCDN'])

    def test_unexpected_size_or_address_stops_before_dump(self):
        for size, address in ((1, 0x80080000), (131072, 0x1F801040),
                              (131072, 0x801FFFF0)):
            peer = peer_for(size=size, address=address)
            with self.assertRaisesRegex(RuntimeError, 'buffer'):
                backup.download(peer, 1, lambda _: None)
            self.assertNotIn(b'DUMP', peer.sent)

    def test_invalid_slot_sends_nothing(self):
        peer = peer_for()
        with self.assertRaises(ValueError):
            backup.download(peer, 0)
        self.assertEqual(peer.sent, [])

    def test_ready_guard(self):
        ready = dict(active=False, fault=False, initialized=True, baud=115200)
        backup.require_ready(ready)
        for key, value in (('active', True), ('fault', True),
                           ('initialized', False), ('baud', 2073600)):
            with self.assertRaises(RuntimeError):
                backup.require_ready(dict(ready, **{key: value}))

    def test_exclusive_file_and_readback(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'synthetic.mcd'
            digest = backup.save_verified(path, PAYLOAD)
            self.assertEqual(path.read_bytes(), PAYLOAD)
            self.assertEqual(len(digest), 64)
            with self.assertRaises(FileExistsError):
                backup.save_verified(path, b'changed')
            self.assertEqual(path.read_bytes(), PAYLOAD)
            with self.assertRaises(RuntimeError):
                backup.check_output(Path(directory) / 'missing' / 'card.mcd')


if __name__ == '__main__':
    unittest.main()
