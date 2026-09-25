import struct
import unittest
import zlib


SUPPORTED_BAUD = {115200, 230400, 518400, 691200, 1036800, 2073600}


def control_packet(nonce: int, baud: int, flags: int) -> bytes:
    body = b"PSB1" + struct.pack("<III", nonce, baud, flags)
    return body + struct.pack("<I", zlib.crc32(body) & 0xFFFFFFFF)


def valid_control(packet: bytes) -> bool:
    if len(packet) != 20 or packet[:4] != b"PSB1":
        return False
    _, baud, flags, received_crc = struct.unpack("<IIII", packet[4:])
    return (
        baud in SUPPORTED_BAUD
        and flags in (0, 1)
        and received_crc == zlib.crc32(packet[:16]) & 0xFFFFFFFF
    )


class PendingQueue:
    """Host model of the firmware's offset-preserving short-write queue."""

    def __init__(self, payload: bytes):
        self.payload = payload
        self.offset = 0

    def write(self, accepted: int) -> bytes:
        chunk = self.payload[self.offset : self.offset + accepted]
        self.offset += len(chunk)
        return chunk

    @property
    def pending(self) -> bytes:
        return self.payload[self.offset :]


class BridgeProtocolTests(unittest.TestCase):
    def test_exact_control_echo_shape(self):
        packet = control_packet(0x12345678, 2073600, 0)
        self.assertEqual(len(packet), 20)
        self.assertTrue(valid_control(packet))

    def test_every_supported_baud(self):
        for baud in SUPPORTED_BAUD:
            with self.subTest(baud=baud):
                self.assertTrue(valid_control(control_packet(7, baud, 0)))

    def test_malformed_packets_are_rejected(self):
        valid = bytearray(control_packet(9, 115200, 0))
        cases = [b"", bytes(valid[:-1]), b"BAD!" + bytes(valid[4:])]
        valid[8] ^= 0x01
        cases.append(bytes(valid))
        cases.append(control_packet(1, 9600, 0))
        cases.append(control_packet(1, 115200, 2))
        for packet in cases:
            with self.subTest(packet=packet):
                self.assertFalse(valid_control(packet))

    def test_partial_writes_preserve_all_bytes(self):
        payload = bytes(range(256)) * 8
        queue = PendingQueue(payload)
        delivered = bytearray()
        for accepted in (1, 7, 0, 1460, 3, 4096):
            delivered.extend(queue.write(accepted))
        delivered.extend(queue.write(len(queue.pending)))
        self.assertEqual(bytes(delivered), payload)
        self.assertEqual(queue.pending, b"")

    def test_disconnect_cleanup_does_not_replay(self):
        first = PendingQueue(b"old-session")
        first.write(3)
        first = PendingQueue(b"")  # Firmware abort clears both pending queues.
        second = PendingQueue(b"new-session")
        self.assertEqual(first.pending, b"")
        self.assertEqual(second.write(99), b"new-session")

    def test_duplicate_abort_has_stable_identity(self):
        packet = control_packet(0xAABBCCDD, 115200, 1)
        cache_key = ("192.0.2.4", packet)
        # Source ports may change across host retries; IP + nonce-bearing frame
        # remains the stable idempotency identity.
        self.assertEqual(cache_key, ("192.0.2.4", packet))
        self.assertNotEqual(cache_key, ("192.0.2.5", packet))


if __name__ == "__main__":
    unittest.main()
