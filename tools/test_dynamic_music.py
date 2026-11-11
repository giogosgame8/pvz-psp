"""Music preparation regressions using small synthetic modules, no game assets."""
import array
from pathlib import Path
import struct
import tempfile
import unittest

from xm_layers import channel_layer, order_timeline, read_patterns, extend_render_cycle
from prepare_dynamic_music import normalize_layers, write_dynamic_index


def module(patterns, channels=2, orders=None, speed=6, bpm=125):
    header=bytearray(336);header[:17]=b'Extended Module: '
    struct.pack_into('<I',header,60,276)
    orders=list(range(len(patterns))) if orders is None else orders
    struct.pack_into('<8H',header,64,len(orders),0,channels,len(patterns),0,0,speed,bpm)
    header[80:80+len(orders)]=bytes(orders)
    for notes in patterns:
        payload=bytes(v for note in notes for v in note)
        header.extend(struct.pack('<IBHH',9,0,len(notes)//channels,len(payload))+payload)
    return bytes(header)


class DynamicMusicTests(unittest.TestCase):
    def test_render_extension_preserves_live_tempo_and_redirects_only_backward_jumps(self):
        source=module([[(0,0,0,15,100),(0,0,0,0,0)],[(0,0,0,11,1),(0,0,0,0,0)]])
        extended=extend_render_cycle(source);_,patterns,_=read_patterns(extended)
        self.assertEqual(struct.unpack_from('<H',extended,64)[0],4)
        self.assertEqual([patterns[n][2][0][3:] for n in range(4)],[(15,100),(11,3),(15,100),(11,3)])
    def test_channel_filter_retains_tempo_and_control_without_other_instruments(self):
        source=module([[(49,1,32,0,0),(50,2,40,15,100),
                        (51,1,32,0,0),(52,2,40,11,0)]])
        filtered=channel_layer(source,[0]);_,patterns,_=read_patterns(filtered)
        self.assertEqual(patterns[0][2],[(49,1,32,0,0),(0,0,0,15,100),
                                        (51,1,32,0,0),(0,0,0,11,0)])
        self.assertEqual(order_timeline(filtered),order_timeline(source))

    def test_first_row_tempo_reset_does_not_create_a_false_intro(self):
        source=module([[(0,0,0,15,100),(0,0,0,0,0),
                        (0,0,0,11,0),(0,0,0,0,0)]],bpm=183)
        self.assertEqual(order_timeline(source),dict(samples=6615,loop_start=0,orders=[(0,0)]))

    def test_authored_intro_and_loop_are_distinct(self):
        source=module([[(0,0,0,0,0)]*2,[(0,0,0,11,1),(0,0,0,0,0)]])
        self.assertEqual(order_timeline(source),dict(samples=5292,loop_start=2646,orders=[(0,0),(2646,1)]))

    def test_pattern_delay_and_loop_have_their_authored_duration(self):
        source=module([[(0,0,0,14,0x60),(0,0,0,0,0),
                        (0,0,0,14,0x62),(0,0,0,0,0),
                        (0,0,0,14,0xe1),(0,0,0,11,0)]])
        self.assertEqual(order_timeline(source)['samples'],21168)

    def test_pattern_break_and_tempo_changes_retain_sample_timing(self):
        source=module([[(0,0,0,13,1),(0,0,0,0,0)],
                       [(0,0,0,0,0)]*2+[(0,0,0,15,3),(0,0,0,11,0)]])
        result=order_timeline(source)
        self.assertEqual(result,dict(samples=5292,loop_start=2646,orders=[(0,0),(2646,1),(3969,0)]))

    def test_truncated_payload_and_invalid_channel_are_rejected(self):
        source=module([[(49,1,0,0,0)]*2])
        with self.assertRaises(ValueError):read_patterns(source[:-1])
        with self.assertRaises(ValueError):channel_layer(source,[2])
        bad=bytearray(source);struct.pack_into('<H',bad,341,257)
        with self.assertRaises(ValueError):read_patterns(bad)

    def test_shared_gain_preserves_layer_balance_and_headroom(self):
        layers=[array.array('h',values) for values in ([1000,-2000,0],[-500,1000,1500],[100,-3000,-250])]
        actual,peak=normalize_layers(layers)
        self.assertEqual(peak,5000)
        self.assertEqual([a.tolist() for a in actual],[a.tolist() for a in layers])
        for gains in ((1,0,0),(1,1,0),(1,0,1),(1,1,1)):
            self.assertLessEqual(max(abs(sum(row[n]*gains[n] for n in range(3))) for row in zip(*actual)),5000)

    def test_night_different_lengths_use_conservative_shared_gain(self):
        layers=[array.array('h',[20000,-10000]),array.array('h',[12000])]
        actual,peak=normalize_layers(layers)
        self.assertEqual(peak,32000)
        self.assertLessEqual(sum(max(abs(s) for s in a) for a in actual),5001)
        self.assertEqual(normalize_layers([array.array('h',[0])]*3)[1],0)

    def test_binary_plan_includes_authored_loop_and_layer_fingerprints(self):
        with tempfile.TemporaryDirectory() as temporary:
            folder=Path(temporary);facts=[{},dict(dynamic=dict(scheme=1,samples=100,loop_start=20,orders=[(0,0),(20,1)]),layers=[dict(layer=1,reference_samples=4096,reference_fnv32=123)])]
            write_dynamic_index(folder,facts)
            data=(folder/'music-dynamics.bin').read_bytes()
            self.assertEqual(struct.unpack_from('<4s10I',data), (b'PVAM',1,1,1,1,100,20,2,0,0,0))
            self.assertEqual(struct.unpack('<4s5I',(folder/'music-layer-checks.bin').read_bytes()),(b'PVLC',1,1,1,4096,123))


if __name__=='__main__':unittest.main()
