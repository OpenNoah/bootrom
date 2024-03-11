#!/usr/bin/env python3
import argparse
import imageio as iio
import numpy as np
from functools import cmp_to_key

def main():
    parser = argparse.ArgumentParser(prog='image_find_regions',
                                     description='Find rectangle regions of certain colour in image')
    parser.add_argument('-c', '--colour', help="Region colour")
    parser.add_argument('input', help="input image")
    parser.add_argument('output', help="output header file")
    args = parser.parse_args()

    # Note: This tool does not work when rectangle touches image edges
    img = iio.imread(args.input, pilmode="RGB")
    img_h, img_w, _ = img.shape
    clr = int(args.colour, 0)
    clr = [clr >> 16, (clr >> 8) & 0xff, clr & 0xff]

    # Horizontal scan pass
    scan_vh = {}
    for y in range(img_h):
        scan_h = {}
        start_x = None
        for x in range(img_w):
            if not np.all(img[y][x] == clr):
                if start_x is not None:
                    scan_h[start_x] = x - start_x
                start_x = None
            elif start_x is None:
                start_x = x
        scan_vh[y] = scan_h

    # Vertical scan pass
    scan = []
    for y in range(img_h):
        for x, w in scan_vh[y].items():
            h = 1
            while x in scan_vh[y + h]:
                del scan_vh[y + h][x]
                h += 1
            scan.append((y, x, h, w))

    # Fuzzy sort, ignore y difference < 8
    scan.sort(key=cmp_to_key(lambda a, b: a[0] - b[0] if a[0] - b[0] >= 8 else a[1] - b[1]))

    with open(args.output, "w") as hfile:
        hfile.write("// Screen size {0, 0, h, w}\n")
        hfile.write(f"{{0, 0, {img_h}, {img_w}}},\n")
        hfile.write("// {y, x, h, w}\n")
        for y, x, h, w in scan:
            hfile.write(f"{{{y}, {x}, {h}, {w}}},\n")

if __name__ == '__main__':
    main()
