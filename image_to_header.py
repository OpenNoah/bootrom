#!/usr/bin/env python3
import imageio as iio
import argparse

def main():
    parser = argparse.ArgumentParser(prog='image_to_header', description='Image data to C header')
    parser.add_argument('input', help="input image")
    parser.add_argument('output', help="output header file")
    args = parser.parse_args()

    img = iio.imread(args.input, pilmode="RGBX")
    # Fix colour channel order
    img[:,:,[0,1,2]] = img[:,:,[2,1,0]]
    ba = bytearray(img)

    with open(args.output, "w") as hfile:
        ofs = 0
        while ofs < len(ba):
            hfile.write(", ".join([f"{v:#04x}" for v in ba[ofs:ofs+16]]) + ",\n")
            ofs += 16

if __name__ == '__main__':
    main()
