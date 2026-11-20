#!/usr/bin/env python3
"""Does the compile contract of Xenosaga I and II reproduce Xenosaga III's code?

Phase 3 of docs/x3-bootstrap.md. The annotations (tools/tu/x3_annotate.py) mark H1 functions, whose
X3 code equals a function of X1 or X2 that has published C. This compiles that C, in its own tree
and under candidate contracts, and compares each function with the X3 bytes:

    x3_calibrate.py run [--files N] [--contracts a,b] [--jobs 4]
    x3_calibrate.py report
    x3_calibrate.py changed [--limit N]

`changed` goes one step past copies. tools/tu/x3_homologs.py lists `offsets` pairs: X3 functions with the
instructions and registers of an X1 or X2 function and other immediates, so code that was compiled again
for X3. Where the immediates that differ are plain constants (a load, a compare, an add, a mask), this
writes the X3 constant into a private copy of the X1 or X2 source, compiles it under the contract and
compares the function with X3's bytes. A match is C that reproduces X3 code which exists in no other game.

The comparison masks relocated fields, because the games place symbols at different addresses.
So this is calibration evidence and never exactness: only the whole-file gate decides that. It is
also a biased sample: code that did not change since X1 or X2. The sibling trees are only read,
and all output stays under .work/x3-calibrate/.
"""
import argparse
import collections
import concurrent.futures
import json
import os
import re
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
X1_ROOT = Path('/home/pc/xenosaga1-port/xenosaga-i-decomp')
X2_ROOT = Path('/home/pc/xenosaga2-port/xenosga-ii-decomp')
SOURCE_ROOTS = dict(X1=X1_ROOT, X2=X2_ROOT)
OUT = ROOT / '.work/x3-calibrate'
CC_TU = ROOT / 'tools/tu/cc_tu.sh'
ANNOTATIONS = (('main', 'SLUS_213.89', 'annotations/slus_213.89_annotations.csv'),
               ('ov01', 'OV01.OVL', 'annotations/overlays/ov01_annotations.csv'),
               ('ov02', 'OV02.OVL', 'annotations/overlays/ov02_annotations.csv'),
               ('ov04', 'OV04.OVL', 'annotations/overlays/ov04_annotations.csv'))
AS_SWAP = '.work/toolchains/ee-legacy-binutils-la29-vsqrt-nolabel-laimm-swap-v1/bin/ee-as'
AS_NO_SWAP = '.work/toolchains/ee-legacy-binutils-la29-vsqrt-nolabel-laimm-v1/bin/ee-as'
CC_FSV12 = '.work/toolchains/ee-gcc2.96-realconv-lp7-fsv16-v1/bin'   # the contract cc1 (name kept; fsv16 since 2026-11-17)
# name -> (compiler dir, assembler, assembler -g, -G rule). None keeps the source TU's own value.
CONTRACTS = {
    'x1': (CC_FSV12, AS_SWAP, None, None),
    'no-g': (CC_FSV12, AS_SWAP, False, None),
    'no-swap': (CC_FSV12, AS_NO_SWAP, False, None),
    'g-flipped': (CC_FSV12, AS_SWAP, None, 'flip'),
    'no-fsv': ('.work/toolchains/ee-gcc2.96-realconv-lp7-v1/bin', AS_SWAP, None, None),
    'no-lp7': ('.work/toolchains/ee-gcc2.96-realconv-v1/bin', AS_SWAP, None, None),
    'no-realconv': ('.work/toolchains/ee-gcc2.96-lp7-v1/bin', AS_SWAP, None, None),
}
# Controls: the compilers Sony shipped with the SDK 3.0 family that X3 links, each with its own assembler
# (public archives of github.com/decompme/compilers, unpacked under compiler-variants/; absent = skipped).
for control in ('ee-gcc3.2-040921', 'ee-gcc3.2-030926'):
    if (ROOT / '.work/toolchains/compiler-variants' / control / 'bin/ee-gcc').exists():
        CONTRACTS['sdk3-' + control.removeprefix('ee-')] = (
            f'.work/toolchains/compiler-variants/{control}/bin',
            f'.work/toolchains/compiler-variants/{control}/bin/ee-as', None, None)
# GCC 3.2 has no separate preprocessor to run and wants its own assembler, so these go through the
# compiler driver in one step instead of cc_tu.sh's cpp, cc1 and as.
ONE_STEP = {name for name in CONTRACTS if name.startswith('sdk3-')}
# Controls: every other public EE compiler, with the contract's assembler so that only cc1 varies.
# Sony's earlier and unpatched builds have the contract's layout and run through cc_tu.sh.
VARIANTS = ROOT / '.work/toolchains/compiler-variants'
for control in ('ee-gcc2.9-990721', 'ee-gcc2.9-991111', 'ee-gcc2.9-991111-01', 'ee-gcc2.9-991111a', 'ee-gcc2.96'):
    if (VARIANTS / control / 'ee-gcc').exists():
        CONTRACTS['sony-' + control.removeprefix('ee-gcc') + ('-unpatched' if control == 'ee-gcc2.96' else '')] = (
            str((VARIANTS / control).relative_to(ROOT)), AS_SWAP, None, None)
# Windows builds (SN Systems' ProDG compilers, and one Sony build): the contract's preprocessor, then
# their cc1.exe under wine, then the contract's assembler.
CC1_ONLY = {}
for name, cc1 in (('sn-2.95.2-273a', 'ee-gcc2.95.2-273a/cc1.exe'),
                  ('sn-2.95.3-114', 'ee-gcc2.95.3-114/lib/gcc-lib/ee/2.95.3/cc1.exe'),
                  ('sn-2.95.3-136', 'ee-gcc2.95.3-136/lib/gcc-lib/ee/2.95.3/cc1.exe'),
                  ('sony-2.9-991111b-r4', 'ee-gcc2.9-991111b-r4/cc1.exe')):
    if (VARIANTS / cc1).exists() and (VARIANTS / 'wine-cc1').exists():
        CC1_ONLY[name] = [str(VARIANTS / 'wine-cc1'), str(VARIANTS / cc1), '-quiet', '-fno-builtin']
        CONTRACTS[name] = (CC_FSV12, AS_SWAP, None, None)
# The patch ladder: each earlier compiler, with the rest of the contract unchanged.
for ladder in sorted((ROOT / '.work/toolchains').glob('ee-gcc2.96-realconv-lp7-fsv*')):
    if (ladder / 'bin/ee-gcc').exists() and 'fsv16' not in ladder.name:
        CONTRACTS.setdefault(ladder.name.removeprefix('ee-gcc2.96-realconv-lp7-'),
                             (str(ladder.relative_to(ROOT) / 'bin'), AS_SWAP, None, None))
R_MIPS_MASK = {2: 0xffffffff, 4: 0x03ffffff, 5: 0xffff, 6: 0xffff, 7: 0xffff, 8: 0xffff}


class ElfObject:
    """Function bytes of one relocatable object, with the words that relocations touch."""

    def __init__(self, path):
        data = Path(path).read_bytes()
        shoff = struct.unpack_from('<I', data, 0x20)[0]
        size, count, names = struct.unpack_from('<HHH', data, 0x2e)
        rows = [struct.unpack_from('<10I', data, shoff + i * size) for i in range(count)]
        strings = rows[names][4]
        self.sections = [(data[strings + r[0]:data.index(b'\0', strings + r[0])].decode(), r) for r in rows]
        by_name = {name: row for name, row in self.sections}
        text = by_name['.text']
        self.text = data[text[4]:text[4] + text[5]]
        text_index = next(i for i, (name, _) in enumerate(self.sections) if name == '.text')
        self.masks = {}
        if '.rel.text' in by_name:
            rel = by_name['.rel.text']
            for offset in range(rel[4], rel[4] + rel[5], 8):
                where, info = struct.unpack_from('<II', data, offset)
                self.masks[where] = self.masks.get(where, 0) | R_MIPS_MASK.get(info & 0xff, 0xffffffff)
        self.functions = {}
        symtab = by_name['.symtab']
        names_row = rows[symtab[6]]
        for offset in range(symtab[4], symtab[4] + symtab[5], 16):
            name_at, value, length, info, _, section = struct.unpack_from('<IIIBBH', data, offset)
            if info & 15 == 2 and section == text_index:
                start = names_row[4] + name_at
                self.functions[data[start:data.index(b'\0', start)].decode()] = (value, length)


class Target:
    """Original X3 bytes by address, for one unit."""

    def __init__(self, path, unit):
        self.data = Path(path).read_bytes()
        shoff = struct.unpack_from('<I', self.data, 0x20)[0]
        size, count, names = struct.unpack_from('<HHH', self.data, 0x2e)
        rows = [struct.unpack_from('<10I', self.data, shoff + i * size) for i in range(count)]
        strings = rows[names][4]
        by_name = {self.data[strings + r[0]:self.data.index(b'\0', strings + r[0])].decode('latin1'): r for r in rows}
        section = by_name['.text' if unit == 'main' else unit]
        self.base = section[4] - section[3]

    def word(self, va):
        return struct.unpack_from('<I', self.data, va + self.base)[0]


def load_h1():
    """(unit, va, source name, (game, source path)) of each H1 annotation that names its source file."""
    rows = []
    for unit, _, path in ANNOTATIONS:
        for line in (ROOT / path).read_text().splitlines():
            match = re.match(r'0x([0-9a-fA-F]+),\s*\S+ - \[H1\].*?(X[12]):\s*(\S+) in (\S+\.c)', line)
            if match:
                rows.append((unit, int(match.group(1), 16), match.group(3), (match.group(2), match.group(4))))
    return rows


def source_tus():
    """(game, source path) -> TU record of that game's build manifest."""
    out = {}
    for game, root in SOURCE_ROOTS.items():
        build = json.loads((root / 'config/tu-build.json').read_text())
        out.update({(game, tu['path']): tu for tu in build['tus'] if tu.get('path')})
    return out


def asm_bodies(source):
    """Functions the source does not hold in C: the object would only echo that game's own bytes."""
    game, path = source
    text = (SOURCE_ROOTS[game] / path).read_text(errors='replace')
    return set(re.findall(r'INCLUDE_ASM\s*\(\s*(?:"[^"]*"\s*,\s*)?"?([A-Za-z_]\w*)"?\s*\)', text))


def compile_tu(contract_name, source, tu, text=None, label=None):
    """One X1 or X2 TU under one contract, through the project's own cc_tu.sh. Returns (object, error).

    `text` compiles that text instead of the published file, from a private copy named `label`.
    """
    compiler, assembler, keep_g, g_rule = CONTRACTS[contract_name]
    game, path = source
    root = SOURCE_ROOTS[game]
    flags = tu['contract']['flags']
    g_flag = next(flag for flag in flags if flag.startswith('-G'))
    if g_rule == 'flip':
        g_flag = '-G0' if g_flag == '-G8' else '-G8'
    has_g = '-g' in tu['contract']['assembler_flags']
    as_options = '-g' if (has_g if keep_g is None else keep_g) else ''
    work = OUT / contract_name / game / tu['unit']
    work.mkdir(parents=True, exist_ok=True)
    out = work / f'{label or Path(path).stem}.o'
    unit_dir = root / 'build' / tu['unit']
    env = dict(os.environ, XENO_EE_AS_OPTIONS=as_options)
    input_path = root / path
    if text is not None:
        # A private copy: the original tree is never written. An include the original finds next to
        # itself is named by its full path, since the copy lives elsewhere.
        beside = (root / path).parent
        text = re.sub(r'(?m)^(\s*#\s*include\s*")([^"]+)(")',
                      lambda m: m.group(1) + (str(beside / m.group(2)) if (beside / m.group(2)).is_file() else m.group(2)) + m.group(3),
                      text)
        input_path = work / f'{label}.c'
        input_path.write_text(f'#line 1 "{root / path}"\n' + text)
    if contract_name in ONE_STEP:
        driver = ROOT / compiler
        out.unlink(missing_ok=True)
        run = subprocess.run(['timeout', '-k', '5', '60', 'env', '-i', 'PATH=/usr/bin:/bin', 'LC_ALL=C',
                              str(driver / 'ee-gcc'), '-B', f'{driver}/ee-', '-c', '-nostdinc', '-fno-builtin', '-O2', g_flag,
                              '-I', 'include', '-I', str(root / 'include'), '-Wa,-I.', f'-Wa,-I{unit_dir}', f'-Wa,-I{root}',
                              str(input_path), '-o', str(out)],
                             cwd=unit_dir, capture_output=True, text=True, timeout=120)
    elif contract_name in CC1_ONLY:
        out.unlink(missing_ok=True)
        base = ROOT / compiler
        stem = out.with_suffix('')
        steps = (
            [str(base / 'ee-gcc'), f'-B{base}/', '-nostdinc', '-fno-builtin', '-E', '-O2', g_flag, '-I', 'include',
             '-I', str(root / 'include'), str(input_path), '-o', f'{stem}.i'],
            CC1_ONLY[contract_name] + ['-O2', g_flag, f'{stem}.i', '-o', f'{stem}.s'],
            [str(ROOT / assembler), '-EL', '-m5900', '-mabi=eabi', g_flag] + ([as_options] if as_options else [])
            + ['-I', '.', '-I', str(unit_dir), '-I', str(root), '-o', str(out), f'{stem}.s'])
        for step in steps:
            run = subprocess.run(['timeout', '-k', '5', '60'] + step, cwd=unit_dir, capture_output=True, text=True, timeout=120)
            if run.returncode:
                break
    else:
        run = subprocess.run(['bash', str(CC_TU), str(input_path), str(out), str(ROOT / compiler),
                              str(ROOT / assembler), g_flag, str(unit_dir), str(root)],
                             cwd=unit_dir, env=env, capture_output=True, text=True, timeout=120)
    if run.returncode or not out.is_file():
        return None, (run.stdout + run.stderr).strip()[-400:]
    return out, ''


def compare(obj, name, target, va):
    """'same', or the first differing offset, comparing only bits no relocation touches."""
    value, length = obj.functions[name]
    length -= length % 4
    differing = []
    for offset in range(0, length, 4):
        mask = obj.masks.get(value + offset, 0)
        ours = struct.unpack_from('<I', obj.text, value + offset)[0]
        if (ours & ~mask) != (target.word(va + offset) & ~mask):
            differing.append(offset)
    return dict(status='same' if not differing else 'differs', size=length, differing_words=len(differing),
                first_difference=differing[0] if differing else None)


def run_contract(contract_name, sources, tus, targets):
    results = {}
    errors = {}
    for source, functions in sources.items():
        label = f'{source[0]}:{source[1]}'
        tu = tus.get(source)
        unusable = 'no_source_tu' if tu is None else None
        if tu is not None and not tu['contract'].get('assembler_flags'):
            unusable = f'source_contract_{tu["contract"]["id"]}'
        if tu is not None and '-O0' in tu['contract']['flags']:
            unusable = 'source_contract_O0'          # cc_tu.sh compiles at -O2 only
        if unusable:
            for row in functions:
                results[f'{row[0]}:{row[1]:#x}'] = dict(name=row[2], source=label, status=unusable)
            continue
        out, error = compile_tu(contract_name, source, tu)
        if out is None:
            errors[label] = error
            for row in functions:
                results[f'{row[0]}:{row[1]:#x}'] = dict(name=row[2], source=label, status='compile_failed')
            continue
        obj = ElfObject(out)
        in_asm = asm_bodies(source)
        for unit, va, name, _ in functions:
            key = f'{unit}:{va:#x}'
            if name in in_asm:
                results[key] = dict(name=name, source=label, status='source_asm')
            elif name not in obj.functions:
                results[key] = dict(name=name, source=label, status='not_in_object')
            else:
                results[key] = dict(name=name, source=label, **compare(obj, name, targets[unit], va))
    return contract_name, results, errors


def command_run(args):
    rows = load_h1()
    sources = collections.defaultdict(list)
    for row in rows:
        sources[row[3]].append(row)
    chosen = sorted(sources, key=lambda source: (-len(sources[source]), source))
    if args.files:
        chosen = chosen[:args.files]
    subset = {path: sources[path] for path in chosen}
    names = args.contracts.split(',') if args.contracts else list(CONTRACTS)
    missing = [name for name in names if not (ROOT / CONTRACTS[name][0] / 'ee-gcc').exists()]
    if missing:
        raise SystemExit(f'compiler missing for contracts: {missing}')
    tus = source_tus()
    targets = {unit: Target(ROOT / 'game/extracted-iso' / file_name, unit) for unit, file_name, _ in ANNOTATIONS}
    summary = dict(functions=sum(len(v) for v in subset.values()), files=len(subset), contracts={}, errors={})
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for name, results, errors in pool.map(lambda c: run_contract(c, subset, tus, targets), names):
            summary['contracts'][name] = results
            if errors:
                summary['errors'][name] = errors
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'result.json').write_text(json.dumps(summary, indent=1) + '\n')
    print_summary(summary)
    return 0


def print_summary(summary):
    print(f'{summary["functions"]} H1 functions in {summary["files"]} X1 and X2 files')
    base = summary['contracts'].get('x1', {})
    for name, results in summary['contracts'].items():
        counts = collections.Counter(row['status'] for row in results.values())
        gained = [key for key, row in results.items() if row['status'] == 'same' and base.get(key, {}).get('status') == 'differs']
        lost = [key for key, row in results.items() if row['status'] == 'differs' and base.get(key, {}).get('status') == 'same']
        print(f'{name:12} {dict(counts)}  fixes against x1: {len(gained)}  breaks against x1: {len(lost)}')
    for name, errors in summary['errors'].items():
        for path, error in list(errors.items())[:3]:
            print(f'  error {name} {path}: {error[:260]}')


# ------------------------------------------------------------------ changed code

CONSTANT_OPS = {0x0a: 'signed', 0x0b: 'signed', 0x0c: 'unsigned', 0x0d: 'unsigned', 0x0e: 'unsigned'}
ADD_OPS = {0x08, 0x09, 0x18, 0x19}


def immediate_changes(source_words, target_words):
    """{source constant: target constant} when every differing instruction holds a plain constant, else None."""
    mapping = {}
    for ours, theirs in zip(source_words, target_words):
        if ours == theirs:
            continue
        op, base = ours >> 26, (ours >> 21) & 31
        if ours >> 16 != theirs >> 16:
            return None                                     # not an immediate
        a, b = ours & 0xffff, theirs & 0xffff
        if op in ADD_OPS and base != 29:
            a, b = (a ^ 0x8000) - 0x8000, (b ^ 0x8000) - 0x8000
        elif op in CONSTANT_OPS:
            if CONSTANT_OPS[op] == 'signed':
                a, b = (a ^ 0x8000) - 0x8000, (b ^ 0x8000) - 0x8000
        else:
            return None                                     # a load, a store or the frame: a layout, not a constant
        if mapping.setdefault(a, b) != b:
            return None
    return mapping or None


def function_span(text, name):
    """(start, end) of the body of the top-level definition of `name`, braces included, or None."""
    for found in re.finditer(r'\b%s\s*\(' % re.escape(name), text):
        depth, position = 1, found.end()
        while position < len(text) and depth:
            depth += {'(': 1, ')': -1}.get(text[position], 0)
            position += 1
        rest = re.match(r'(?:\s|/\*.*?\*/|//[^\n]*\n|[\w\s\*,\[\]]+;)*\{', text[position:], re.S)
        if not rest or text.count('{', 0, found.start()) != text.count('}', 0, found.start()):
            continue
        start = position + rest.end() - 1
        depth, end = 0, start
        while end < len(text):
            depth += {'{': 1, '}': -1}.get(text[end], 0)
            end += 1
            if not depth:
                return start, end
    return None


def patch_constants(body, mapping):
    """The body with each changed constant rewritten, trying the spellings a compare or a subtraction gives."""
    literal = re.compile(r'(?<![\w.])(0[xX][0-9a-fA-F]+|\d+)(?![\w.xX])')
    out, notes = body, []
    for old, new in mapping.items():
        for shift, sign in ((0, 1), (0, -1), (-1, 1), (1, 1)):   # N, -N (x - N), N-1 (<= N-1), N+1 (> N+1)
            wanted = sign * old + shift

            def swap(match, wanted=wanted, value=sign * new + shift):
                if int(match.group(1), 0) != wanted or value < 0:
                    return match.group(0)
                return hex(value) if match.group(1).lower().startswith('0x') else str(value)
            changed = literal.sub(swap, out)
            if changed != out:
                out = changed
                notes.append(f'{old} -> {new} as {"-" if sign < 0 else ""}N{shift:+d}' if shift or sign < 0 else f'{old} -> {new}')
                break
        else:
            return None, [f'no literal for {old}']
    return out, notes


def command_changed(args):
    sys.path.insert(0, str(ROOT / 'tools/tu'))
    import map_infer
    tus = source_tus()
    targets = {unit: Target(ROOT / 'game/extracted-iso' / file_name, unit) for unit, file_name, _ in ANNOTATIONS}
    source_files = dict(X1=dict(main='SLUS_204.69', ov01='OV01.OVL', ov02='OV02.OVL', ov10='OV10.OVL', ov11='OV11.OVL', ov12='OV12.OVL'),
                        X2=dict(main='SLUS_208.92', ov01='OV01.OVL', ov02='OV02.OVL'))
    images = {}
    rows, seen = [], set()
    for game in ('X1', 'X2'):
        path = ROOT / f'annotations/leads/{game.lower()}-homologs.json'
        if not path.is_file():
            raise SystemExit(f'{path} is missing: run tools/tu/x3_homologs.py first')
        for pair in json.loads(path.read_text())['pairs']:
            key = (pair['x3_unit'], pair['x3_va'])
            if pair['cls'] == 'offsets' and pair['src_state'] == 'exact_c' and pair['src_file'] and key not in seen:
                seen.add(key)
                rows.append((game, pair))
    results = []
    counts = collections.Counter()
    for game, pair in rows:
        unit, va = pair['src_unit'], int(pair['src_va'], 16)
        if (game, unit) not in images:
            images[game, unit] = map_infer.Image(SOURCE_ROOTS[game] / 'game/extracted-iso' / source_files[game][unit], unit)
        image, target = images[game, unit], targets[pair['x3_unit']]
        x3_va, words = int(pair['x3_va'], 16), pair['x3_size'] // 4
        mapping = immediate_changes([image.word(va + 4 * i) for i in range(words)],
                                    [target.word(x3_va + 4 * i) for i in range(words)])
        record = dict(x3=f'{pair["x3_unit"]}:{pair["x3_va"]}', source=f'{game}:{pair["src_file"]}', name=pair['src_name'],
                      size=pair['x3_size'])
        results.append(record)
        if mapping is None:
            record['status'] = 'layout_changed'
            counts['layout_changed'] += 1
            continue
        record['constants'] = {str(old): new for old, new in mapping.items()}
        source = (game, pair['src_file'])
        tu = tus.get(source)
        file = SOURCE_ROOTS[game] / pair['src_file']
        if tu is None or not file.is_file() or '-O0' in tu['contract']['flags'] or not tu['contract'].get('assembler_flags'):
            record['status'] = 'no_source'
            counts['no_source'] += 1
            continue
        text = file.read_text(errors='replace')
        span = function_span(text, pair['src_name'])
        if span is None:
            record['status'] = 'definition_not_found'
            counts['definition_not_found'] += 1
            continue
        body, notes = patch_constants(text[span[0]:span[1]], mapping)
        record['patch'] = notes
        if body is None:
            record['status'] = 'constant_not_in_source'
            counts['constant_not_in_source'] += 1
            continue
        if args.limit and counts['compiled'] >= args.limit:
            record['status'] = 'skipped_by_limit'
            continue
        counts['compiled'] += 1
        label = f'{Path(pair["src_file"]).stem}__{pair["src_name"]}'
        out, error = compile_tu('x1', source, tu, text[:span[0]] + body + text[span[1]:], label)
        if out is None:
            record.update(status='compile_failed', error=error[-300:])
            counts['compile_failed'] += 1
            continue
        obj = ElfObject(out)
        if pair['src_name'] not in obj.functions:
            record['status'] = 'not_in_object'
            counts['not_in_object'] += 1
            continue
        record.update(compare(obj, pair['src_name'], target, x3_va))
        counts[record['status']] += 1
        # The unpatched source must not match: otherwise the patch proved nothing.
        base, _ = compile_tu('x1', source, tu)
        if base is not None and pair['src_name'] in ElfObject(base).functions:
            record['unpatched'] = compare(ElfObject(base), pair['src_name'], target, x3_va)['status']
    summary = dict(offsets_pairs_with_published_c=len(rows), counts=dict(counts), results=results)
    (OUT / 'changed.json').parent.mkdir(parents=True, exist_ok=True)
    (OUT / 'changed.json').write_text(json.dumps(summary, indent=1) + '\n')
    print(json.dumps(dict(offsets_pairs_with_published_c=len(rows), counts=dict(counts)), indent=1))
    for record in results:
        if record['status'] in ('same', 'differs'):
            print(f'  {record["status"]:8} {record["x3"]} {record["name"]} ({record["size"]} bytes) {record["source"]} '
                  f'{record["patch"]} unpatched={record.get("unpatched")} differing_words={record.get("differing_words")}')
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest='command', required=True)
    run = sub.add_parser('run', help='compile the H1 functions under candidate contracts')
    run.add_argument('--files', type=int, help='only the N source files with the most H1 functions')
    run.add_argument('--contracts', help='comma-separated subset of: ' + ', '.join(CONTRACTS))
    run.add_argument('--jobs', type=int, default=4)
    sub.add_parser('report', help='print the last result')
    changed = sub.add_parser('changed', help='compile X1 and X2 sources with the constants X3 changed')
    changed.add_argument('--limit', type=int, help='compile at most N functions')
    args = parser.parse_args(argv)
    if args.command == 'run':
        return command_run(args)
    if args.command == 'changed':
        return command_changed(args)
    print_summary(json.loads((OUT / 'result.json').read_text()))
    return 0


if __name__ == '__main__':
    sys.exit(main())
