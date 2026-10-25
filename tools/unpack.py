#!/usr/bin/env python3
"""The demo, unpacked by its own decruncher.

    unpack.py           game/!FishTank2/Fishtank2 -> unpacked/fishtank2
                        (the image the decruncher leaves at &8000)

Fishtank2 is an absolute (&FF8) packed with a decruncher on its end: its
first word branches to &80208, which copies the decruncher above where the
unpacked image will end, calls OS_SynchroniseCodeAreas on that copy
(R1 = its start: the end of the image), decrunches the program backwards
into &8000 upwards, synchronises it and enters it at &8000. Here it runs
under SuperSpider's tools/arm2.py until it reaches &8000.
"""
import os
import sys

SS = os.environ.get('SUPERSPIDER', os.path.expanduser('~/stuff/superspider'))
sys.path.insert(0, os.path.join(SS, 'tools'))
import arm2  # noqa: E402

TOP = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
APP = os.path.join(TOP, 'game', '!FishTank2')
OUT = os.path.join(TOP, 'unpacked')
BASE = 0x8000
SYNC = 0x2006E                          # XOS_SynchroniseCodeAreas


def find(d, name):
    """a RISC OS file in a host directory: any case, any ,load-exec suffix"""
    for f in os.listdir(d):
        if f.split(',')[0].lower() == name.lower():
            return os.path.join(d, f)
    raise FileNotFoundError(os.path.join(d, name))


def unpack(packed):
    m = arm2.ARM(memsize=0x1000000)
    m.mem[BASE:BASE + len(packed)] = packed
    m.r[13], m.r[14] = 0, 0xDEAD0
    end = []

    def swi(m, n, pc):
        if n != SYNC:
            raise RuntimeError('unpack: SWI &%X at &%X' % (n, pc))
        if not end:                     # the decruncher's copy of itself
            end.append(m.r[1])
    m.swi_handler = swi
    m.r[15] = BASE
    while not (m.r[15] == BASE and m.steps):
        m.step()
        if m.steps > 100_000_000:
            raise RuntimeError('unpack: no end, pc=&%X' % m.r[15])
    if not end:
        raise RuntimeError('unpack: entered &8000 without synchronising the decruncher')
    return bytes(m.mem[BASE:end[0]])


def main():
    os.makedirs(OUT, exist_ok=True)
    image = unpack(open(find(APP, 'Fishtank2'), 'rb').read())
    open(os.path.join(OUT, 'fishtank2'), 'wb').write(image)
    print('fishtank2: %d bytes at &%X' % (len(image), BASE))


if __name__ == '__main__':
    main()
