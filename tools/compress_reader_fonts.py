#!/usr/bin/env python3
"""Losslessly encode existing 4-bpp LVGL glyphs; retain metrics and character maps."""
import argparse
import hashlib
from pathlib import Path
import re

GLYPH = re.compile(r'\{\.bitmap_index=(\d+),\.adv_w=(\d+),\.box_w=(\d+),\.box_h=(\d+),\.ofs_x=(-?\d+),\.ofs_y=(-?\d+)\},')


def encode(pixels):
    bits = []
    state, previous, repeated, counter = 'single', None, 0, 0
    def put(value, width):
        bits.append(format(value, f'0{width}b'))
    for index, value in enumerate(pixels):
        if state == 'single':
            put(value, 4)
            if previous is not None and value == previous:
                state, repeated = 'repeat', 0
            previous = value
        elif state == 'repeat':
            if value != previous:
                put(0, 1); put(value, 4)
                state, previous = 'single', value
            else:
                put(1, 1); repeated += 1
                if repeated == 11:
                    count = 0
                    for following in pixels[index + 1:index + 63]:
                        if following != previous: break
                        count += 1
                    counter = count + 1
                    put(counter, 6); state = 'counter'
        else:
            counter -= 1
            if counter == 0:
                put(value, 4); previous, state = value, 'single'
            else:
                assert value == previous
    stream = ''.join(bits)
    return bytes(int(stream[i:i + 8].ljust(8, '0'), 2) for i in range(0, len(stream), 8)) + b'\0\0'


def decode(data, count):
    bit = 0
    state, previous, repeated, counter = 'single', 0, 0, 0
    def take(width):
        nonlocal bit
        result = 0
        for _ in range(width):
            if bit >= len(data) * 8: raise ValueError('Truncated glyph')
            result = (result << 1) | ((data[bit // 8] >> (7 - bit % 8)) & 1); bit += 1
        return result
    out = []
    for _ in range(count):
        if state == 'single':
            value = take(4)
            if bit != 4 and value == previous: state, repeated = 'repeat', 0
            previous = value
        elif state == 'repeat':
            flag = take(1); repeated += 1
            value = previous
            if flag:
                if repeated == 11:
                    counter = take(6)
                    if counter: state = 'counter'
                    else: previous = value = take(4); state = 'single'
            else: previous = value = take(4); state = 'single'
        else:
            value = previous; counter -= 1
            if not counter: previous = value = take(4); state = 'single'
        out.append(value)
    return out


def source_glyphs(text):
    match = re.search(r'static const uint8_t glyph_bitmap\[\] = \{(.*?)\};', text, re.S)
    if not match: raise ValueError('Missing glyph bitmap')
    bitmap = bytes(int(x, 16) for x in re.findall(r'0x([0-9a-fA-F]{2})', match[1]))
    glyphs = [tuple(map(int, m.groups())) for m in GLYPH.finditer(text)]
    format_id = int(re.search(r'\.bitmap_format=(\d+)', text)[1])
    if int(re.search(r'\.bpp=(\d+)', text)[1]) != 4 or format_id not in (0, 1, 2):
        raise ValueError('Only known 4-bpp plain/RLE fonts are supported')
    values = []
    for offset, advance, width, height, x, y in glyphs:
        count = width * height
        if format_id == 0:
            pixels = [(bitmap[offset + i // 2] >> (0 if i % 2 else 4)) & 15 for i in range(count)]
        else:
            pixels = decode(bitmap[offset:], count)
            if format_id == 1:
                for i in range(width, count): pixels[i] ^= pixels[i - width]
        values.append(pixels)
    return match, glyphs, values, len(bitmap)


def pixel_hash(glyphs, values):
    digest = hashlib.sha256()
    for glyph, pixels in zip(glyphs, values):
        digest.update(str(glyph[1:]).encode()); digest.update(bytes(pixels))
    return digest.hexdigest()


def verify(path):
    text = path.read_text()
    _, glyphs, pixels, _ = source_glyphs(text)
    expected = re.search(r'Lossless glyph pixels SHA256: ([0-9a-f]{64})', text)
    if expected and pixel_hash(glyphs, pixels) != expected[1]: raise ValueError('Pixel verification failed')
    for offset, advance, width, height, x, y in glyphs:
        if not (offset < (1 << 20) and advance < (1 << 12) and width < 256 and height < 256 and
                -128 <= x < 128 and -128 <= y < 128):
            raise ValueError('Glyph exceeds compact LVGL metrics; compress or review the font configuration')
    return len(glyphs) - 1


def compress(path):
    text = path.read_text()
    bitmap_match, glyphs, original, old_size = source_glyphs(text)
    candidates = []
    for prefilter in (False, True):
        bitmap = bytearray(); offsets = []
        for glyph, pixels in zip(glyphs, original):
            offsets.append(len(bitmap))
            if not pixels: continue
            width = glyph[2]
            filtered = [value ^ pixels[i - width] if prefilter and i >= width else value
                        for i, value in enumerate(pixels)]
            packed = encode(filtered)
            recovered = decode(packed, len(filtered))
            if prefilter:
                for i in range(width, len(recovered)): recovered[i] ^= recovered[i - width]
            assert recovered == pixels
            bitmap.extend(packed)
        candidates.append((len(bitmap), prefilter, offsets, bitmap))
    size, prefilter, offsets, bitmap = min(candidates, key=lambda item: item[0])
    if size >= old_size: return old_size, old_size
    lines = ['    ' + ','.join(f'0x{x:02x}' for x in bitmap[i:i + 24]) + ',' for i in range(0, size, 24)]
    changed = text[:bitmap_match.start(1)] + '\n' + '\n'.join(lines) + '\n' + text[bitmap_match.end(1):]
    iterator = iter(offsets)
    changed = GLYPH.sub(lambda m: m[0].replace('.bitmap_index=' + m[1], '.bitmap_index=' + str(next(iterator)), 1), changed)
    changed = re.sub(r'\.bitmap_format=\d+', '.bitmap_format=' + ('1' if prefilter else '2'), changed)
    changed = re.sub(r'/\* Lossless glyph pixels SHA256: [0-9a-f]{64} \*/\n', '', changed)
    changed = '/* Lossless glyph pixels SHA256: ' + pixel_hash(glyphs, original) + ' */\n' + changed
    _, final_glyphs, final_pixels, _ = source_glyphs(changed)
    assert [x[1:] for x in glyphs] == [x[1:] for x in final_glyphs] and original == final_pixels
    path.write_text(changed)
    return old_size, size


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--verify', action='store_true')
    args = parser.parse_args()
    for name in ('reader_font', 'reader_chart_font'):
        path = args.root / f'assets/fonts/{name}.c'
        if args.verify: print(f'{name}: {verify(path)} glyphs verified')
        else:
            old, new = compress(path)
            print(f'{name}: bitmap {old} -> {new} bytes, lossless verification PASS')


if __name__ == '__main__': main()
