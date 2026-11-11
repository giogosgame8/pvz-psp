"""Select owned XM channels without removing global timing commands."""
import struct


def read_patterns(source):
    if source[:17] != b'Extended Module: ' or len(source) < 336:
        raise ValueError('Invalid XM header')
    channels, count = struct.unpack_from('<HH', source, 68)
    if not 1 <= channels <= 64 or not 1 <= count <= 256:
        raise ValueError('Invalid XM dimensions')
    offset = 60 + struct.unpack_from('<I', source, 60)[0]
    patterns = []
    for _ in range(count):
        if offset + 9 > len(source):
            raise ValueError('Truncated XM pattern')
        header = struct.unpack_from('<I', source, offset)[0]
        rows, size = struct.unpack_from('<HH', source, offset + 5)
        if header < 9 or not 1 <= rows <= 256 or offset + header + size > len(source):
            raise ValueError('Invalid XM pattern bounds')
        cursor, end = offset + header, offset + header + size
        notes = []
        if not size:
            notes = [(0, 0, 0, 0, 0)] * (rows * channels)
        else:
            for _ in range(rows * channels):
                if cursor >= end:
                    raise ValueError('Truncated XM event')
                first = source[cursor]; cursor += 1
                values = [0] * 5
                if first & 128:
                    for col in range(5):
                        if first & (1 << col):
                            if cursor >= end: raise ValueError('Truncated XM event field')
                            values[col] = source[cursor]; cursor += 1
                else:
                    if cursor + 4 > end: raise ValueError('Truncated XM note')
                    values = [first, *source[cursor:cursor + 4]]; cursor += 4
                notes.append(tuple(values))
            if cursor != end: raise ValueError('Unexpected XM pattern payload')
        patterns.append((source[offset:offset + header], rows, notes))
        offset = end
    return channels, patterns, offset


def channel_layer(source, keep):
    channels, patterns, tail = read_patterns(source)
    keep = set(keep)
    if any(not 0 <= n < channels for n in keep): raise ValueError('Invalid XM channel selection')
    output = bytearray(source[:60 + struct.unpack_from('<I', source, 60)[0]])
    for header, rows, notes in patterns:
        payload = bytearray()
        for n, event in enumerate(notes):
            values = list(event)
            if n % channels not in keep:
                # Keep jumps, breaks, tempo, global volume, pattern loop/delay.
                effect, value = values[3:]
                global_effect = effect in (11, 13, 15, 16, 17) or (effect == 14 and value >> 4 in (6, 14))
                values = [0, 0, 0, effect if global_effect else 0, value if global_effect else 0]
            payload.extend(values)
        h = bytearray(header); struct.pack_into('<H', h, 7, len(payload))
        output.extend(h); output.extend(payload)
    output.extend(source[tail:])
    return bytes(output)


def extend_render_cycle(source):
    """Keep decoder loop detection beyond the exported first cycle.

    Duplicate order-specific patterns so a backward position jump continues
    into the second copy with its live instruments and tempo intact. This is
    only a render input; the shipped loop/intro still comes from the source.
    """
    channels,patterns,tail=read_patterns(source)
    length=struct.unpack_from('<H',source,64)[0]
    if not 1<=length<=128:raise ValueError('XM section is too long to extend')
    orders=source[80:80+length];output=bytearray(source[:60+struct.unpack_from('<I',source,60)[0]])
    struct.pack_into('<H',output,64,length*2);struct.pack_into('<H',output,70,length*2)
    output[80:336]=bytes(range(length*2))+bytes(256-length*2)
    for cycle in range(2):
        for position,pattern in enumerate(orders):
            header,rows,notes=patterns[pattern];payload=bytearray()
            for note in notes:
                values=list(note)
                if values[3]==11:
                    target=values[4]
                    values[4]=target+length*(1 if target<=position else cycle)
                payload.extend(values)
            h=bytearray(header);struct.pack_into('<H',h,7,len(payload))
            output.extend(h);output.extend(payload)
    output.extend(source[tail:]);return bytes(output)


def order_timeline(source, sample_rate=22050):
    """Trace global XM control flow to the first repeated sequencer state."""
    channels, patterns, _ = read_patterns(source)
    length, restart = struct.unpack_from('<HH', source, 64)
    if not 1 <= length <= 256 or restart >= length: raise ValueError('Invalid XM order list')
    orders = source[80:80 + length]
    speed, bpm = struct.unpack_from('<HH', source, 76)
    if speed < 1 or bpm < 32: raise ValueError('Invalid XM tempo')
    order = row = 0; seconds = 0.0; seen = {}; entries = []
    starts = [0] * channels; loops = [0] * channels
    for _ in range(200000):
        if order >= length: order = restart; row = 0
        if orders[order] >= len(patterns): raise ValueError('Invalid XM pattern reference')
        _, rows, notes = patterns[orders[order]]
        if row >= rows: order += 1; row = 0; continue
        # A row that sets tempo renders identically on its first visit even
        # when the inherited header tempo differs from a later loop's tempo.
        for event in notes[row * channels:(row + 1) * channels]:
            if event[3] == 15 and event[4]:
                if event[4] < 32: speed = event[4]
                else: bpm = event[4]
        state = (order, row, speed, bpm, tuple(starts), tuple(loops))
        sample = round(seconds * sample_rate)
        if state in seen:
            return dict(samples=sample, loop_start=seen[state], orders=entries)
        seen[state] = sample
        if (row == 0 or not entries or entries[-1][1] != order) and (not entries or entries[-1][0] != sample): entries.append((sample, order))
        jump = next_row = loop_row = None; delay = 0
        for channel in range(channels):
            effect, value = notes[row * channels + channel][3:]
            if effect == 15 and value:
                if value < 32: speed = value
                else: bpm = value
            elif effect == 11: jump = value
            elif effect == 13: next_row = (value >> 4) * 10 + (value & 15)
            elif effect == 14:
                command, amount = value >> 4, value & 15
                if command == 14: delay = max(delay, amount)
                elif command == 6:
                    if not amount: starts[channel] = row
                    elif not loops[channel]: loops[channel] = amount; loop_row = starts[channel]
                    else:
                        loops[channel] -= 1
                        if loops[channel]: loop_row = starts[channel]
        seconds += speed * 2.5 / bpm * (delay + 1)
        if jump is not None or next_row is not None:
            order = jump if jump is not None else order + 1
            row = next_row or 0; starts = [0] * channels; loops = [0] * channels
        elif loop_row is not None: row = loop_row
        else:
            row += 1
            if row >= rows: order += 1; row = 0; starts = [0] * channels; loops = [0] * channels
    raise ValueError('XM sequence exceeds bounded timeline trace')
