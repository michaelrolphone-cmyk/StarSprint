import re
import unittest
from pathlib import Path


class MainLoopTimingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = Path("src/main.c").read_text()

    def test_main_loop_scans_pads_every_frame(self):
        self.assertIn("scanPads();", self.source)

    def test_nmi_callback_flushes_console_via_dma_path(self):
        callback_match = re.search(
            r"static void vblank_dma_transfer\(void\) \{(?P<body>.*?)\n\}",
            self.source,
            re.S,
        )
        self.assertIsNotNone(callback_match, "vblank_dma_transfer() not found")
        callback_body = callback_match.group("body")
        self.assertIn("consoleVblank();", callback_body)
        self.assertIn("nmiSet(vblank_dma_transfer);", self.source)

    def test_main_loop_waits_for_vblank_before_input(self):
        match = re.search(r"while \(1\) \{(?P<body>.*?)\n    \}", self.source, re.S)
        self.assertIsNotNone(match, "main loop while(1) body not found")

        body = match.group("body")
        wait_index = body.find("WaitForVBlank();")
        scan_index = body.find("scanPads();")
        pads_index = body.find("pad0 = padsCurrent(0);")

        self.assertNotEqual(wait_index, -1, "WaitForVBlank() missing from main loop")
        self.assertNotEqual(scan_index, -1, "scanPads() missing from main loop")
        self.assertNotEqual(pads_index, -1, "padsCurrent(0) missing from main loop")
        self.assertLess(wait_index, scan_index)
        self.assertLess(scan_index, pads_index)

    def test_main_loop_waits_once_per_frame(self):
        match = re.search(r"while \(1\) \{(?P<body>.*?)\n    \}", self.source, re.S)
        self.assertIsNotNone(match, "main loop while(1) body not found")
        body = match.group("body")
        self.assertEqual(body.count("WaitForVBlank();"), 1)


if __name__ == "__main__":
    unittest.main()
