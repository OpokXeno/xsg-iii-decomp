#!/usr/bin/env python3
"""Which compiler built X3's game code? Evidence from the bytes, never proof.

    python3 -B tools/tu/x3_compiler_evidence.py     # prints and writes .work/x3-compiler/evidence.json

X3 links Sony's SDK 3.0 libraries, and Sony shipped GCC 3.2 with that SDK, so the X1 and X2 compiler
(GCC 2.96) is not a given. Nothing in the files names a compiler. This tool measures four things on
X1, X2 and X3, and reports/x3-compiler-identity.md reads them:

  runtime    The C runtime (newlib, fdlibm, libgcc) is installed with the compiler, not with the SDK.
             How much of X3's runtime zone equals X2's and X1's, relocated fields masked?
  helpers    GCC 2.9x for the EE calls soft-float helpers by their GOFAST names (dpmul, litodp, ...).
             Does X3's game code call them?
  strings    cc1 2.96 pools strings per object and ld merges nothing, so a text used by two objects
             is in .rodata twice. GCC 3 marks strings mergeable and ld keeps one copy.
  idioms     Rates of code shapes that depend on the compiler, and the distance between the operation
             statistics of each body of code: X3's new code against X1 and X2 game code, with code
             built by other compilers or options as controls (SDK and CRI zones, X2's -O0 zone).

tools/tu/x3_calibrate.py is the other half: it compiles C and compares bytes. All trees are only read.
"""
import collections
import json
import math
import re
import sys
from pathlib import Path

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))
import map_infer  # noqa: E402
import x3_homologs as homologs  # noqa: E402
import x3_scaffold  # noqa: E402

OUT = ROOT / '.work/x3-compiler/evidence.json'
X2_O0 = (0x1a75a8, 0x1d5440)
# Zones of each main executable: X3 from map_infer.MAIN_ZONES, X2 from its tree's map_infer.py, X1 from
# the library_class ranges of its build manifest.
ZONES = dict(
    X1=dict(sdk=((0x2009a0, 0x218240),), runtime=((0x323ab0, 0x338570),)),
    X2=dict(sdk=((0x120000, 0x138298), (0x29f000, 0x2a1578)), cri=((0x13b7a0, 0x194638),),
            runtime=((0x2a1578, 0x2b3740),)),
    X3=dict(sdk=((0x2e69b8, 0x303a50),), cri=((0x120250, 0x17ce40),), sndf=((0x17ce40, 0x181950),),
            runtime=((0x303a50, 0x3172e8),)))
LIKELY = {0x14, 0x15, 0x16, 0x17}


def in_zones(va, zones):
    return any(low <= va < high for low, high in zones)


def load_all():
    games = dict(
        X1=homologs.load_game('X1', homologs.X1_ROOT, homologs.X1_FILES, homologs.manifest_rows(homologs.X1_ROOT)),
        X2=homologs.load_game('X2', homologs.X2_ROOT, homologs.X2_FILES,
                              homologs.estimated_rows(homologs.X2_ROOT, homologs.X2_GAME)),
        X3=homologs.load_game('X3', ROOT, homologs.X3_FILES, homologs.estimated_rows(ROOT, homologs.X3_GAME)))
    return games


def zone(functions, game, name):
    return [f for f in functions if f.unit == 'main' and f.library and in_zones(f.va, ZONES[game][name])]


# ------------------------------------------------------------------ runtime

def runtime_identity(games):
    out = {}
    for name in ('runtime', 'sdk', 'cri'):
        mine = zone(games['X3'], 'X3', name)
        row = dict(x3_functions=len(mine), x3_bytes=sum(4 * len(f.words) for f in mine))
        for other in ('X2', 'X1'):
            if name not in ZONES[other]:
                continue
            pool = collections.Counter(f.exact for f in zone(games[other], other, name) if len(f.words) >= 4)
            same = [f for f in mine if len(f.words) >= 4 and pool[f.exact]]
            row[f'equal_in_{other}'] = dict(functions=len(same), bytes=sum(4 * len(f.words) for f in same),
                                           other_functions=sum(pool.values()))
        out[name] = row
    return out


def helper_calls(games):
    targets = {('main', va): name for va, name in x3_scaffold.SOFT_FLOAT_LIBCALLS.items()}
    calls = collections.Counter()
    callers = set()
    for f in games['X3']:
        if f.library:
            continue
        for call in f.calls:
            if call in targets:
                calls[targets[call]] += 1
                callers.add(f.key)
    return dict(call_sites=dict(sorted(calls.items())), game_functions_that_call_one=len(callers))


# ------------------------------------------------------------------ strings

def duplicate_strings(games):
    out = {}
    paths = dict(X1=homologs.X1_ROOT / 'game/extracted-iso/SLUS_204.69', X2=homologs.X2_ROOT / 'game/extracted-iso/SLUS_208.92',
                 X3=ROOT / 'game/extracted-iso/SLUS_213.89')
    for game, path in paths.items():
        image = map_infer.Image(path, 'main')
        users = collections.defaultdict(set)                # text -> addresses game code references it at
        for f in games[game]:
            if f.library or f.unit != 'main':
                continue
            for _, target, _, _ in map_infer.scan_refs(image, dict(va=f.va, end=f.va + 4 * len(f.words))):
                if image.section_of(target) in ('.rodata', '.sdata'):
                    text = map_infer.string_at(image, target)
                    if text is not None and len(text) >= 8:
                        users[text].add(target)
        repeated = {text: places for text, places in users.items() if len(places) > 1}
        out[game] = dict(texts_referenced_by_game_code=len(users), texts_at_two_addresses_or_more=len(repeated),
                         copies=sum(len(places) for places in repeated.values()),
                         examples=[text.decode('latin1')[:40] for text in sorted(repeated, key=lambda t: -len(repeated[t]))[:4]])
    return out


# ------------------------------------------------------------------- idioms

def corpus_table(games):
    paired = {}
    leads = ROOT / 'annotations/leads/homologs.tsv'
    if leads.is_file():
        rows = [line.split('\t') for line in leads.read_text().splitlines()[1:]]
        header = leads.read_text().splitlines()[0].split('\t')
        for row in rows:
            record = dict(zip(header, row))
            paired[record['x3_unit'], int(record['x3_va'], 16)] = record['cls']
    x3 = games['X3']
    game3 = [f for f in x3 if not f.library]
    corpora = collections.OrderedDict()
    corpora['X1 game'] = [f for f in games['X1'] if not f.library]
    corpora['X2 game, -O2'] = [f for f in games['X2'] if not f.library and not (f.unit == 'main' and X2_O0[0] <= f.va < X2_O0[1])]
    corpora['X3 game, copies of X1 or X2 (exact, offsets)'] = [f for f in game3 if paired.get(f.key) in ('exact', 'offsets', 'opcodes')]
    corpora['X3 game, no pair in X1 or X2'] = [f for f in game3 if f.key not in paired]
    corpora['X3 game, ov02 and ov04, no pair'] = [f for f in game3 if f.key not in paired and f.unit in ('ov02', 'ov04')]
    corpora['control: X2 game, -O0 zone'] = [f for f in games['X2'] if f.unit == 'main' and not f.library and X2_O0[0] <= f.va < X2_O0[1]]
    corpora['control: X3 SDK 3.0 zone'] = zone(x3, 'X3', 'sdk')
    corpora['control: X2 SDK 2.7 zone'] = zone(games['X2'], 'X2', 'sdk')
    corpora['control: X1 SDK 2.4 zone'] = zone(games['X1'], 'X1', 'sdk')
    corpora['control: X3 CRI zone'] = zone(x3, 'X3', 'cri')
    corpora['control: X3 SNDF client'] = zone(x3, 'X3', 'sndf')
    return corpora


def idioms(functions):
    words = total = 0
    count = collections.Counter()
    framed = first = frame16 = ra_sd = ra_sw = tail = aligned8 = 0
    for f in functions:
        ws = f.words
        if not ws:
            continue
        total += 1
        words += len(ws)
        aligned8 += f.va % 8 == 0
        frame = next((i for i, w in enumerate(ws[:12]) if w >> 16 == 0x27bd and w & 0x8000), None)
        if frame is not None:
            framed += 1
            first += frame == 0
            frame16 += (-(ws[frame] | ~0xffff)) % 16 == 0
            ra_sd += any(w >> 16 == 0xffbf for w in ws[:24])
            ra_sw += any(w >> 16 == 0xafbf for w in ws[:24])
            for i, w in enumerate(ws[:-1]):
                if w == 0x03e00008:
                    tail += ws[i + 1] >> 16 == 0x27bd and not ws[i + 1] & 0x8000
                    break
        for w in ws:
            op, rs, rt, rd, funct = w >> 26, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31, w & 0x3f
            if w == 0:
                count['nop'] += 1
            elif op == 0:
                if funct == 0x2d and rt == 0:
                    count['move as daddu'] += 1
                elif funct == 0x21 and rt == 0:
                    count['move as addu'] += 1
                elif funct == 0x25 and rt == 0:
                    count['move as or'] += 1
                elif funct == 0 and (w >> 6) & 31 == 0 and rd:
                    count['sll x, y, 0 (sign extension)'] += 1
                elif funct in (0x3c, 0x3e, 0x3f):
                    count['dsll32, dsrl32, dsra32'] += 1
                elif funct == 0x0d:
                    count['break (division checks)'] += 1
                elif funct in (0x0a, 0x0b):
                    count['movz, movn'] += 1
                elif funct == 0x18 and rd:
                    count['mult with a destination'] += 1
            elif op in LIKELY or (op == 1 and rt in (2, 3, 0x12, 0x13)):
                count['branch likely'] += 1
            elif op == 0x09 and rs == 0:
                count['li as addiu'] += 1
            elif op == 0x0d and rs == 0:
                count['li as ori'] += 1
            elif op == 0x0c and w & 0xffff in (0xff, 0xffff):
                count['andi 0xff or 0xffff (zero extension)'] += 1
    per = lambda n: round(1000 * n / max(1, words), 2)
    share = lambda n, of: round(100 * n / max(1, of), 1)
    out = collections.OrderedDict(functions=total, instructions=words)
    out['functions that start on a multiple of 8 (%)'] = share(aligned8, total)
    out['functions with a frame (%)'] = share(framed, total)
    out['frame opened by the first instruction (% of framed)'] = share(first, framed)
    out['frame size a multiple of 16 (% of framed)'] = share(frame16, framed)
    out['ra saved with sd (% of framed)'] = share(ra_sd, framed)
    out['ra saved with sw (% of framed)'] = share(ra_sw, framed)
    out['frame closed in the delay slot of jr ra (% of framed)'] = share(tail, framed)
    for name in ('nop', 'branch likely', 'move as daddu', 'move as addu', 'move as or', 'sll x, y, 0 (sign extension)',
                 'dsll32, dsrl32, dsra32', 'andi 0xff or 0xffff (zero extension)', 'li as addiu', 'li as ori',
                 'break (division checks)', 'movz, movn', 'mult with a destination'):
        out[name + ' (per 1000)'] = per(count[name])
    return out


def bigrams(functions):
    table = collections.Counter()
    for f in functions:
        ops = f.opcodes
        table.update(zip(ops, ops[1:]))
    return table


def divergence(a, b):
    """Jensen-Shannon divergence of two count tables, in bits: 0 for equal statistics, 1 for disjoint ones."""
    total_a, total_b = sum(a.values()), sum(b.values())
    out = 0.0
    for key in set(a) | set(b):
        p, q = a.get(key, 0) / total_a, b.get(key, 0) / total_b
        m = (p + q) / 2
        if p:
            out += 0.5 * p * math.log2(p / m)
        if q:
            out += 0.5 * q * math.log2(q / m)
    return round(out, 4)


def split_halves(functions):
    """Two halves of one corpus by alternating functions: the distance a body of code has to itself."""
    return divergence(bigrams(functions[0::2]), bigrams(functions[1::2]))


def main():
    games = load_all()
    corpora = corpus_table(games)
    tables = {name: bigrams(functions) for name, functions in corpora.items()}
    names = list(corpora)
    references = ('X1 game', 'X2 game, -O2')
    distances = collections.OrderedDict()
    for name in names:
        distances[name] = collections.OrderedDict((reference, divergence(tables[name], tables[reference]))
                                                  for reference in references if reference != name)
        distances[name]['two halves of itself'] = split_halves(corpora[name])
    evidence = collections.OrderedDict(
        runtime=runtime_identity(games), helpers=helper_calls(games), strings=duplicate_strings(games),
        idioms=collections.OrderedDict((name, idioms(functions)) for name, functions in corpora.items()),
        operation_pair_distance=distances)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(evidence, indent=1) + '\n')
    print(json.dumps(evidence, indent=1))
    return 0


if __name__ == '__main__':
    sys.exit(main())
