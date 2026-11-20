#!/usr/bin/env python3
"""Name leads for X3 from equal bodies in Xenosaga I and II. Writes annotations/, never identity.

    python3 -B tools/tu/x3_annotate.py            # write the annotation files and a report
    python3 -B tools/tu/x3_annotate.py --check    # say what would change, write nothing

X3 has no symbols. An X3 function whose whole body equals a function of X1 (which has a symbol
table and real object boundaries) or of X2 (whose names and files are themselves recovered
labels) once relocated fields are masked gets that function's name and source file as a lead:

    0x00182290, xglFontGetSPcodeSize - [H1] same code as X1; X1: xglFontGetSPcodeSize in src/main/xgl_font.c
    0x00181D10, xglFontCharToEuc - [H1] same code as X2; X2: xglFontCharToEuc in src/main/xgl_font.c

tools/tu/map_infer.py reads these files: the name labels the function, and the file feeds the
homolog rule (neighbours from one file hold a span, neighbours from two files hold a cut, compared
only within one source game). An X2 file is given only when X2 named that TU on evidence; an
inferred `tuNNN` is a guess about X2 and says nothing here.

Rules, all conservative: bodies of 8 words or more; the X3 body, the source body and the name must
each be unique; X1 wins over X2; only in-scope (game) TUs of the source are read. The mask covers
`j`/`jal` targets, `lui` immediates, `$gp`-relative immediates and the low half that follows a
`lui`, so two functions that differ only in a struct offset do NOT match: this finds copies, not
relatives. Both sibling trees are only read.
"""
import argparse
import collections
import json
import re
import sys
from pathlib import Path

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))
import map_infer  # noqa: E402

SOURCES = (
    ('X1', Path('/home/pc/xenosaga1-port/xenosaga-i-decomp'),
     dict(main='SLUS_204.69', ov01='OV01.OVL', ov02='OV02.OVL', ov10='OV10.OVL', ov11='OV11.OVL', ov12='OV12.OVL')),
    ('X2', Path('/home/pc/xenosaga2-port/xenosga-ii-decomp'),
     dict(main='SLUS_208.92', ov01='OV01.OVL', ov02='OV02.OVL')),
)
TARGETS = (('main', 'SLUS_213.89', 'annotations/slus_213.89_annotations.csv'),) + tuple(
    (unit, name, path) for unit, name, path in map_infer.OVERLAYS)
MIN_WORDS = 8
REPORT = '.work/x3-bootstrap/x3-annotate.json'
LOADSTORE = set(range(0x20, 0x30)) | {0x31, 0x35, 0x36, 0x37, 0x39, 0x3d, 0x3e, 0x3f, 0x1a, 0x1b, 0x1e, 0x1f}
SETS_RT = {0x09, 0x0d, 0x37, 0x1a, 0x1b, 0x1e} | set(range(0x20, 0x28))


def masked(words):
    """The body with every field a relocation can fill set to zero."""
    out, high = [], set()
    for word in words:
        op, rs, rt = word >> 26, (word >> 21) & 31, (word >> 16) & 31
        if op in (2, 3):
            word &= 0xfc000000
        elif op == 0x0f:
            word &= 0xffff0000
            high.add(rt)
        elif op in (0x09, 0x0d) or op in LOADSTORE:
            if rs == map_infer.GP or rs in high:
                word &= 0xffff0000
            if op in SETS_RT:
                high.discard(rt)
        out.append(word)
    return tuple(out)


def body(image, va, size):
    words = [image.word(at) for at in range(va, va + size - size % 4, 4)]
    while words and words[-1] == 0:
        words.pop()
    return masked(words) if len(words) >= MIN_WORDS else None


def source_functions(tag, root, files):
    """masked body -> [(unit, name, file or None)] for the in-scope TUs of one sibling project."""
    build = json.loads((root / 'config/tu-build.json').read_text())
    images, bodies = {}, collections.defaultdict(list)
    for tu in build['tus']:
        unit = tu['unit']
        if not tu.get('in_scope') or unit not in files:
            continue
        if unit not in images:
            images[unit] = map_infer.Image(root / 'game/extracted-iso' / files[unit], unit)
        image = images[unit]
        # X1's files are real objects; an X2 file counts only when X2 named the TU on evidence.
        # An ordinal file name (tuNNN) is X2's own numbering, whatever evidence X2 attached to it.
        named = tag == 'X1' or (tu.get('name_confidence') not in (None, 'none')
                                and not re.fullmatch(r'tu\d+', Path(tu['path']).stem))
        for function in tu['functions']:
            va = int(function['va'], 16)
            if not image.text_start <= va < image.limit_va:
                continue
            key = body(image, va, function['size'])
            if key is not None:
                bodies[key].append((unit, function['name'], tu['path'] if named else None))
    return bodies


def annotate(root=ROOT):
    game = root / 'game/extracted-iso'
    sources = [(tag, source_functions(tag, tree, files)) for tag, tree, files in SOURCES]
    out, report = {}, dict(units={}, rules=dict(min_words=MIN_WORDS))
    for unit_name, file_name, path in TARGETS:
        image = map_infer.Image(game / file_name, unit_name)
        if image.overlay:
            image.text_end = map_infer.find_text_end(image)
        else:
            image.o0_zones = map_infer.MAIN_O0_ZONES
        functions, _ = map_infer.find_functions(image)
        # A library zone of main can hold a body equal to a small game function of X1 or X2; a game
        # name there would be wrong, so only game zones are annotated.
        in_game = (lambda va: True) if image.overlay else (
            lambda va: any(z['cls'] == 'game' and z['start'] <= va < z['end'] for z in map_infer.MAIN_ZONES))
        mine = collections.defaultdict(list)
        for function in functions:
            if not in_game(function['va']):
                continue
            key = body(image, function['va'], function['end'] - function['va'])
            if key is not None:
                mine[key].append(function['va'])
        rows, stats = {}, collections.Counter(functions=len(functions), comparable_game=sum(len(v) for v in mine.values()))
        for key, vas in mine.items():
            for tag, bodies in sources:
                found = bodies.get(key)
                if not found:
                    continue
                if len(vas) > 1:
                    stats[f'{tag}_x3_body_not_unique'] += len(vas)
                    break
                names = {name for _, name, _ in found}
                if len(names) > 1:
                    stats[f'{tag}_source_body_not_unique'] += 1
                    break
                source_unit, name, source_file = found[0]
                if name.startswith('func_'):
                    stats[f'{tag}_unnamed_in_source'] += 1
                    break
                rows[vas[0]] = (name, tag, source_unit, source_file, 4 * len(key))
                stats[f'{tag}_equal'] += 1
                stats[f'{tag}_equal_bytes'] += 4 * len(key)
                break
        # One name, one function: a name two X3 functions would share is dropped from both.
        used = collections.Counter(name for name, *_ in rows.values())
        for va in [va for va, row in rows.items() if used[row[0]] > 1]:
            stats['name_not_unique_in_x3'] += 1
            del rows[va]
        lines = []
        for va in sorted(rows):
            name, tag, source_unit, source_file, _ = rows[va]
            where = f'; {tag}: {name} in {source_file}' if source_file else f' ({source_unit}, file not named there)'
            lines.append(f'0x{va:08X}, {name} - [H1] same code as {tag}{where}')
        out[path] = '\n'.join(lines) + ('\n' if lines else '')
        stats['annotated'] = len(rows)
        stats['with_file'] = sum(1 for row in rows.values() if row[3])
        report['units'][unit_name] = dict(sorted(stats.items()))
    return out, report


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--check', action='store_true', help='report differences and write nothing')
    args = parser.parse_args(argv)
    out, report = annotate()
    changed = [path for path, text in out.items()
               if not (ROOT / path).is_file() or (ROOT / path).read_text() != text]
    if not args.check:
        for path, text in out.items():
            (ROOT / path).parent.mkdir(parents=True, exist_ok=True)
            (ROOT / path).write_text(text)
        (ROOT / REPORT).parent.mkdir(parents=True, exist_ok=True)
        (ROOT / REPORT).write_text(json.dumps(report, indent=1) + '\n')
    print(json.dumps(dict(report, changed=changed, written=not args.check), indent=1))
    return 1 if args.check and changed else 0


if __name__ == '__main__':
    sys.exit(main())
