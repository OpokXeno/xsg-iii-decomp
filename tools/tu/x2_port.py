#!/usr/bin/env python3
"""Bring X2's published C for the X3 functions that have an X2 homolog into X3's TU layout.

User order 2026-11-20 (express, no review; decision D7). X2 is read-only. Nothing here decides
exactness: `match` only says which X2 function compiles to X3's bytes under the X3 TU's flags, and the
whole-file gate decides afterwards. Adapted from X2's tools/tu/x1_port.py (X2 git fea2ea3e), which did
the same with X1's C on X2 on 2026-11-05.

    x2_port.py match  --out WORK [--unit U] [--cls CLS ...] [-j N]
    x2_port.py plan   --work WORK [--write]
    x2_port.py source --work WORK --kit KIT --unit U [--tu ID ...] [--skip X2NAME ...]
    x2_port.py publish --kit KIT --unit U

`match` reads annotations/leads/x2-homologs.tsv (tools/tu/x3_homologs.py), compiles each X2 source file
under the X3 TU's -G with the X3 contract (tools/tu/cc_tu.sh in X2's own build directory), compares the
paired function with the relocated fields masked, and reads what every relocation means in X3: the X3
address behind each X2 symbol. `plan` keeps the equal functions whose references agree, and with --write
names them and their references (labels only, decision D1) in config/symbols/function-names.json and
data-names.json. `source` writes KIT/base/src/U and KIT/merged, ready for tools/express.py check: X2's
function text in X3's address order, the other functions left as INCLUDE_ASM, and the X2 declarations
those bodies need. It drops a function that stays different, so it is a filter over X2's text, not a
search for a form.
"""
import argparse
import collections
import concurrent.futures
import csv
import json
import os
import re
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = Path(os.environ.get('XENO_X2_ROOT', ROOT.parents[1] / 'xenosaga2-port' / 'xenosga-ii-decomp'))
sys.path.insert(0, str(ROOT / 'tools'))
sys.path.insert(0, str(ROOT / 'tools/tu'))
import tu_edit  # noqa: E402
from x3_calibrate import ElfObject, Target  # noqa: E402

CC = ROOT / '.work/toolchains/ee-gcc2.96-realconv-lp7-fsv17-v1/bin'
AS = ROOT / '.work/toolchains/ee-legacy-binutils-la29-vsqrt-nolabel-laimm-swap-v1/bin/ee-as'
CC_TU = ROOT / 'tools/tu/cc_tu.sh'
HOMOLOGS = ROOT / 'annotations/leads/x2-homologs.tsv'
IMAGES = {'main': 'game/extracted-iso/SLUS_213.89', 'ov01': 'game/extracted-iso/OV01.OVL',
          'ov02': 'game/extracted-iso/OV02.OVL', 'ov04': 'game/extracted-iso/OV04.OVL'}
UNITS = ('main', 'ov01', 'ov02', 'ov04')
GP = 0x005571F0
R_26, R_HI16, R_LO16, R_GPREL16, R_LITERAL = 4, 5, 6, 7, 8
IDENTIFIER = re.compile(r'[A-Za-z_]\w*')
ORDER = 'X3 express port of X2 C (user 2026-11-20)'


def s16(value):
    value &= 0xffff
    return value - 0x10000 if value & 0x8000 else value


def compile_x2(job):
    """X2's file under the X3 contract with one -G. Returns (key, object or None, error)."""
    path, unit, g_flag, out_dir = job
    out = Path(out_dir) / g_flag[1:] / unit / (path.replace('/', '__') + '.o')
    out.parent.mkdir(parents=True, exist_ok=True)
    key = (path, unit, g_flag)
    if out.is_file():
        return key, str(out), None
    unit_dir = SRC / 'build' / unit
    env = dict(os.environ, XENO_EE_AS_OPTIONS='-g')
    env.pop('XENO_TU_DEPFILE', None)
    run = subprocess.run(['bash', str(CC_TU), str(SRC / path), str(out), str(CC), str(AS), g_flag, str(unit_dir),
                          str(SRC)], cwd=unit_dir, env=env, capture_output=True, text=True, timeout=200)
    if run.returncode or not out.is_file():
        out.unlink(missing_ok=True)
        return key, None, (run.stdout + run.stderr)[-400:]
    return key, str(out), None


class SourceObject(ElfObject):
    """ElfObject plus the relocation list and symbol table of .text."""

    def __init__(self, path):
        super().__init__(path)
        self.data = Path(path).read_bytes()
        self.relocations = self._relocations()

    def _relocations(self):
        data = self.data
        shoff, shentsize, shnum, shstrndx = (struct.unpack_from('<I', data, 32)[0], *struct.unpack_from('<HHH', data, 46))
        sections = [struct.unpack_from('<10I', data, shoff + i * shentsize) for i in range(shnum)]
        names = sections[shstrndx]

        def section_name(section):
            end = data.index(b'\0', names[4] + section[0])
            return data[names[4] + section[0]:end].decode()
        self.section_names = [section_name(s) for s in sections]
        self.sections_raw = sections
        text = self.section_names.index('.text')
        self.text_offset = sections[text][4]
        self.gp0 = 0
        if '.reginfo' in self.section_names:
            self.gp0 = struct.unpack_from('<i', data, sections[self.section_names.index('.reginfo')][4] + 20)[0]
        symtab = next(s for s in sections if s[1] == 2)
        strings = sections[symtab[6]]
        symbols = []
        for i in range(symtab[5] // 16):
            name, value, size, info, _, shndx = struct.unpack_from('<IIIBBH', data, symtab[4] + i * 16)
            end = data.index(b'\0', strings[4] + name)
            symbols.append(dict(name=data[strings[4] + name:end].decode(), value=value, size=size,
                                bind=info >> 4, type=info & 15, shndx=shndx))
        self.symbols = symbols
        out = []
        for section in sections:
            if section[1] == 9 and section[7] == text:      # SHT_REL against .text
                for i in range(section[5] // 8):
                    offset, info = struct.unpack_from('<II', data, section[4] + i * 8)
                    out.append((offset, info & 0xff, symbols[info >> 8]))
        return out

    def resolve(self, kind, symbol, addend, address):
        """What one relocated field names in X2, and the X3 address behind that name."""
        out = dict(kind=kind, address=address, addend=addend)
        shndx = symbol['shndx']
        section = self.section_names[shndx] if 0 < shndx < len(self.section_names) else None
        out['section'] = section
        if symbol['type'] != 3:
            out.update(name=symbol['name'], base=(address - addend) & 0xffffffff, local=symbol['bind'] == 0)
            return out
        wanted_types = (2,) if section == '.text' else (0, 1)
        named = [s for s in self.symbols if s['shndx'] == shndx and s['type'] in wanted_types and s['name']
                 and s['value'] <= addend < s['value'] + max(s['size'], 1)]
        if not named and section != '.text':
            before = [s for s in self.symbols if s['shndx'] == shndx and s['type'] in (0, 1) and s['name']
                      and s['size'] == 0 and s['value'] <= addend]
            named = sorted(before, key=lambda s: -s['value'])[:1]
        if named:
            best = named[0]
            out.update(name=best['name'], base=(address - (addend - best['value'])) & 0xffffffff, local=True)
        else:                               # a string, a literal or a jump table the compiler emits itself
            out.update(name=None, base=address, local=True, own_data=True)
        return out

    def word(self, offset):
        return int.from_bytes(self.data[self.text_offset + offset:self.text_offset + offset + 4], 'little')


def read_function(obj, name, target, va, size):
    """(same, why, references) for X2's `name` against X3's bytes at `va`, relocated fields masked."""
    if name not in obj.functions:
        return False, 'absent from the X2 object', []
    start, length = obj.functions[name]
    if length != size:
        return False, f'size {length} vs X3 {size}', []
    masks = obj.masks
    differing = [offset for offset in range(0, length - length % 4, 4)
                 if (obj.word(start + offset) ^ target.word(va + offset)) & ~masks.get(start + offset, 0) & 0xffffffff]
    if differing:
        return False, f'{len(differing)} of {length // 4} words differ, first +{differing[0]:#x}', []
    references = []
    highs = [(offset, symbol) for offset, kind, symbol in obj.relocations
             if kind == R_HI16 and start <= offset < start + length]

    def partner(offset, symbol):
        base = (obj.word(offset) >> 21) & 31
        same = [(at, s) for at, s in highs if s['name'] == symbol['name'] and s['shndx'] == symbol['shndx']]
        fitting = [at for at, _ in same if (obj.word(at) >> 16) & 31 == base] or [at for at, _ in same]
        earlier = [at for at in fitting if at < offset]
        return max(earlier) if earlier else (min(fitting) if fitting else None)

    for offset, kind, symbol in obj.relocations:
        if not start <= offset < start + length:
            continue
        here = va + offset - start
        word = target.word(here)
        own = obj.word(offset)
        if kind == R_26:
            address = (here & 0xf0000000) | ((word & 0x03ffffff) << 2)
            references.append(obj.resolve('call', symbol, (own & 0x03ffffff) << 2, address))
        elif kind == R_LO16:
            at = partner(offset, symbol)
            if at is not None:
                high, own_high = target.word(va + at - start), obj.word(at)
                address = (((high & 0xffff) << 16) + s16(word)) & 0xffffffff
                addend = (((own_high & 0xffff) << 16) + s16(own)) & 0xffffffff
                references.append(obj.resolve('abs', symbol, addend, address))
        elif kind in (R_GPREL16, R_LITERAL):
            addend = s16(own) + (obj.gp0 if symbol['type'] == 3 else 0)
            references.append(obj.resolve('literal' if kind == R_LITERAL else 'gp', symbol, addend,
                                          (GP + s16(word)) & 0xffffffff))
    return True, None, references


def x3_tables():
    """{unit: {va: (name, TU id, size)}} over every X3 TU record, whatever its scope."""
    tables = {}
    for unit in UNITS:
        tables[unit] = {}
        for path in sorted((ROOT / 'config/tu' / unit).glob('tu*.json')):
            record = json.loads(path.read_text())
            for function in record['functions']:
                tables[unit][int(function['va'], 16)] = (function['name'], record['id'], function['size'])
    return tables


def lookup(tables, unit, address):
    return tables[unit].get(address) or (tables['main'].get(address) if unit != 'main' else None)


def x3_contracts():
    return {tu['id']: tu for tu in json.loads((ROOT / 'config/tu-build.json').read_text())['tus']}


def homolog_rows(units, classes):
    with HOMOLOGS.open() as handle:
        rows = [r for r in csv.DictReader(handle, delimiter='\t') if r['source'] == 'X2']
    return [r for r in rows if r['x3_unit'] in units and (not classes or r['cls'] in classes)]


def command_match(args):
    tables = x3_tables()
    tus = x3_contracts()
    rows = []
    for row in homolog_rows(args.unit or UNITS, args.cls):
        unit, va = row['x3_unit'], int(row['x3_va'], 16)
        known = tables[unit].get(va)
        if not known:
            continue
        tu = tus[known[1]]
        if not tu.get('in_scope') or not row['src_file']:
            continue
        g_flag = next(f for f in tu['contract']['flags'] if f.startswith('-G'))
        rows.append(dict(unit=unit, tu=known[1], va=row['x3_va'], name=known[0], size=known[2], cls=row['cls'],
                         ratio=row['ratio'], x2=dict(name=row['src_name'], unit=row['src_unit'], va=row['src_va'],
                                                     path=row['src_file'], state=row['src_state']), g=g_flag))
    jobs = sorted({(r['x2']['path'], r['x2']['unit'], r['g'], str(args.out / 'obj')) for r in rows})
    print(f'{len(rows)} pairs, {len(jobs)} compiles')
    objects, errors = {}, {}
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        for key, path, error in pool.map(compile_x2, jobs):
            if path:
                objects[key] = path
            else:
                errors['|'.join(key)] = error
    loaded = {}
    targets = {unit: Target(ROOT / path, unit) for unit, path in IMAGES.items()}
    results = []
    for row in rows:
        key = (row['x2']['path'], row['x2']['unit'], row['g'])
        if key not in objects:
            row.update(same=False, why='compile failed', references=[])
        else:
            obj = loaded.setdefault(key, SourceObject(objects[key]))
            same, why, references = read_function(obj, row['x2']['name'], targets[row['unit']],
                                                  int(row['va'], 16), row['size'])
            row.update(same=same, why=why, references=references, object=objects[key])
        results.append(row)
    (args.out / 'match.json').write_text(json.dumps(dict(results=results, compile_errors=errors), indent=1) + '\n')
    hit = [r for r in results if r['same']]
    by_cls = collections.Counter(r['cls'] for r in hit)
    print(f'compile errors {len(errors)}; equal under the X3 flags {len(hit)} of {len(results)}, '
          f'{sum(r["size"] for r in hit)} bytes; by class {dict(by_cls)}')


# -- plan: X2 names for the X3 functions and data the equal bodies reference ---------------------------

AUTO_NAME = re.compile(r'^(?:D|func|jtbl|lbl|data|L|runtime)_[0-9A-Fa-f]+$')
ADDRESS_SUFFIX = re.compile(r'_[0-9A-F]{8}$')


def choose(results):
    """Equal pairs whose named references agree: one X2 name means one X3 address per X2 file.

    The pairs come from x3_homologs.py, which already pairs each X3 function with at most one X2 one. A
    reference conflict (the same X2 symbol of the same file at two X3 addresses) drops every pair of that
    file that uses the symbol: it says the bodies are alike but the data moved.
    """
    hits = [r for r in results if r['same']]
    seen = collections.defaultdict(set)
    for row in hits:
        for reference in row['references']:
            if reference.get('name'):
                seen[(row['x2']['path'], reference['name'])].add(reference['base'])
    taken = collections.Counter((r['x2']['path'], r['x2']['name']) for r in hits)
    chosen, skipped = {}, []
    for row in hits:
        key = f"{row['unit']}:{row['va']}"
        conflicts = sorted({r['name'] for r in row['references'] if r.get('name')
                            and len(seen.get((row['x2']['path'], r['name']), ())) > 1})
        if conflicts:
            skipped.append(dict(key=key, tu=row['tu'], name=row['name'], why='references move: ' + ', '.join(conflicts[:4])))
        elif taken[(row['x2']['path'], row['x2']['name'])] > 1:
            skipped.append(dict(key=key, tu=row['tu'], name=row['name'], why=f"{row['x2']['name']} fits several X3 functions"))
        else:
            chosen[key] = row
    for row in results:
        if not row['same']:
            skipped.append(dict(key=f"{row['unit']}:{row['va']}", tu=row['tu'], name=row['name'], why=row['why']))
    return chosen, skipped


def command_plan(args):
    match = json.loads((args.work / 'match.json').read_text())
    chosen, skipped = choose(match['results'])
    tables = x3_tables()
    registry = {}
    for unit, table in tables.items():
        for va, (name, _, _) in table.items():
            registry.setdefault(name, (unit, va))
    data_path = ROOT / 'config/symbols/data-names.json'
    existing_data = json.loads(data_path.read_text()) if data_path.is_file() else dict(units={})
    for unit, table in existing_data.get('units', {}).items():
        for va, name in table.items():
            registry.setdefault(name, (unit, int(va, 16)))
    function_names = collections.defaultdict(dict)
    data_names = collections.defaultdict(dict)

    def claim(name, owner, va):
        if AUTO_NAME.match(name) or registry.get(name, (owner, va)) != (owner, va):
            return False
        registry[name] = (owner, va)
        return True

    def name_function(unit, va, wanted):
        owner = unit if va in tables[unit] else 'main'
        current = tables[owner][va][0]
        if not AUTO_NAME.match(current) or va in function_names[owner]:
            return
        wanted = ADDRESS_SUFFIX.sub(f'_{va:08X}', wanted)
        if claim(wanted, owner, va):
            function_names[owner][va] = wanted
        elif not AUTO_NAME.match(wanted):
            function_names[owner][va] = f'{wanted}_{va:08X}'
            registry[function_names[owner][va]] = (owner, va)

    ordered = sorted(chosen.values(), key=lambda r: -r['size'])
    for row in ordered:
        name_function(row['unit'], int(row['va'], 16), row['x2']['name'])
    for row in ordered:
        unit = row['unit']
        for reference in row['references']:
            name, base = reference.get('name'), reference['base']
            if not name:
                continue
            if lookup(tables, unit, base):
                name_function(unit, base, name)
            else:
                owner = unit        # by the unit whose link uses the label, as x1_port did
                if base in data_names[owner] or f'0x{base:08x}' in existing_data.get('units', {}).get(owner, {}):
                    continue
                wanted = ADDRESS_SUFFIX.sub(f'_{base:08X}', name)
                if claim(wanted, owner, base):
                    data_names[owner][base] = wanted
                elif base < 0x00100000 and AUTO_NAME.match(wanted):
                    # An absolute address below the image (X2's `D_108 = 0x108;`): no splat label stands
                    # there, so the link needs the label the body is rewritten to.
                    data_names[owner][base] = f'D_{base:08X}'
    out = dict(chosen=chosen, skipped=skipped)
    (args.work / 'plan.json').write_text(json.dumps(out, indent=1) + '\n')
    size = sum(r['size'] for r in chosen.values())
    print(f'chosen {len(chosen)} functions, {size} bytes, in {len({r["tu"] for r in chosen.values()})} TUs; '
          f'skipped {len(skipped)}')
    print('skip reasons', collections.Counter(re.sub(r'[\d,+x]+', 'N', s['why'].split(':')[0])[:40]
                                              for s in skipped).most_common(8))
    print('function names', {u: len(v) for u, v in function_names.items()}, 'data names',
          {u: len(v) for u, v in data_names.items()})
    if args.write:
        names_path = ROOT / 'config/symbols/function-names.json'
        names = json.loads(names_path.read_text()) if names_path.is_file() else dict(
            schema='x3-function-names/1',
            note='Function labels (decision D1). Written by tools/tu/x2_port.py plan and other naming passes.',
            units={})
        for unit, table in function_names.items():
            if table:
                names['units'].setdefault(unit, {}).update({f'0x{va:08x}': n for va, n in sorted(table.items())})
        names_path.write_text(json.dumps(names, indent=2) + '\n')
        existing_data.setdefault('schema', 'x3-data-names/1')
        existing_data.setdefault('note', 'Data labels read from X2 relocations against X3 addresses, by the unit '
                                         'whose link uses them. Labels only (D1). Written by tools/tu/x2_port.py plan.')
        for unit, table in data_names.items():
            if table:
                existing_data['units'].setdefault(unit, {}).update({f'0x{va:08x}': n for va, n in sorted(table.items())})
        data_path.write_text(json.dumps(existing_data, indent=2) + '\n')
        print('wrote config/symbols/function-names.json and data-names.json; run x3_scaffold.py next')


# -- source: X2's text under X3's layout ---------------------------------------------------------------

X3_HEADERS = {'common.h', 'include_asm.h', 'shared.h'}             # read for their names, never re-declared
X3_ATOMIC = {'quadword.h', 'vu0_macro.h', 'debug_break.h'}          # the same files exist here: #include them
INCLUDE_LINE = re.compile(r'#\s*include\s+"([^"]+)"')
DEFINE_LINE = re.compile(r'#\s*define\s+([A-Za-z_]\w*)')
MEMBER = re.compile(r'(?:->|\.)\s*$')
TAG_DEFINED = re.compile(r'\b(?:struct|union|enum)\s+([A-Za-z_]\w*)\s*\{')
ANY_TAG = re.compile(r'\b(struct|union|enum)\s+([A-Za-z_]\w*)')


def identifiers(masked):
    out = set()
    for match in IDENTIFIER.finditer(masked):
        if not MEMBER.search(masked[max(0, match.start() - 4):match.start()]):
            out.add(match.group())
    return out


def strip_blocks(masked):
    flat, blocks, depth, start = [], [], 0, 0
    for index, char in enumerate(masked):
        if char == '{':
            if depth == 0:
                start = index + 1
            depth += 1
        elif char == '}':
            depth -= 1
            if depth == 0:
                blocks.append(masked[start:index])
        elif depth == 0:
            flat.append(char)
    return ''.join(flat), blocks


def split_commas(text):
    parts, depth, current = [], 0, []
    for char in text:
        if char in '([':
            depth += 1
        elif char in ')]':
            depth -= 1
        if char == ',' and depth == 0:
            parts.append(''.join(current))
            current = []
        else:
            current.append(char)
    parts.append(''.join(current))
    return parts


def cut_initializer(masked):
    depth = 0
    for index, char in enumerate(masked):
        if char in '([{':
            depth += 1
        elif char in ')]}':
            depth -= 1
        elif char == '=' and depth == 0:
            return index
    return None


class Declaration:
    """One top-level item of an X2 source or header."""

    def __init__(self, kind, text, comment, file):
        self.kind, self.text, self.comment, self.file = kind, text, comment, file
        self.masked = tu_edit.lexical(text)
        self.names = set()
        self.is_data = self.is_proto = self.is_static = False
        self.inline = False
        if kind == 'define':
            self.names = {DEFINE_LINE.match(text).group(1)}
            macro_body = self.masked[DEFINE_LINE.match(self.masked).end():]
            self.uses = identifiers(macro_body)
        elif kind == 'decl':
            self.read_declaration()
        elif kind == 'func':
            head = self.masked[:self.masked.index('{')]
            self.head = text[:len(head)].rstrip()
            self.names = {tu_edit.declarator_name(head)}
            self.is_static = bool(re.search(r'\bstatic\b', head))
            self.inline = bool(re.search(r'\binline\b|__inline', head))
            self.uses = identifiers(self.masked)

    def read_declaration(self):
        masked = tu_edit._strip_attributes(self.masked)
        flat, blocks = strip_blocks(masked)
        self.is_static = bool(re.search(r'\bstatic\b', flat))
        typedef = bool(re.search(r'\btypedef\b', flat))
        names = set(TAG_DEFINED.findall(masked))
        for keyword, tag in ANY_TAG.findall(flat):
            if re.fullmatch(rf'\s*{keyword}\s+{tag}\s*;\s*', flat):
                names.add(tag)
        if re.search(r'\benum\b', flat):
            for block in blocks:
                for part in split_commas(block):
                    found = IDENTIFIER.search(part)
                    if found:
                        names.add(found.group())
        at = cut_initializer(flat)
        declarators = flat[:at] if at is not None else flat.rstrip().rstrip(';')
        is_function = False
        for part in split_commas(declarators):
            pointer = re.search(r'\(\s*\*+\s*([A-Za-z_]\w*)', part)
            function = tu_edit.declarator_name(part + ';')
            if pointer:
                names.add(pointer.group(1))
                self.has_declarator = True
            elif function:
                names.add(function)
                is_function = True
            else:
                words = IDENTIFIER.findall(part.split('[')[0])
                if words and words[-1] not in ('struct', 'union', 'enum', 'int', 'char', 'long', 'short', 'float',
                                               'double', 'void', 'unsigned', 'signed', 'const', 'volatile'):
                    if not (len(words) == 2 and words[0] in ('struct', 'union', 'enum')):
                        names.add(words[-1])
                        self.has_declarator = True
        declared = getattr(self, 'has_declarator', False)
        self.is_proto = is_function and not typedef
        self.is_data = declared and not typedef and not is_function
        self.names = names
        self.emit_masked = masked
        if self.is_data:
            cut = cut_initializer(masked)
            self.emit_masked = masked[:cut] if cut is not None else masked
        self.uses = identifiers(self.emit_masked) - names

    def emit(self, keep_static=False):
        text = self.text
        if self.kind == 'func':
            head = self.head if keep_static else re.sub(r'\bstatic\s+', '', self.head)
            return head.strip() + ';'
        if self.is_proto and not keep_static:
            return re.sub(r'\bstatic\s+', '', text)
        if self.is_data:
            cut = cut_initializer(self.masked)
            if cut is not None:
                string = re.match(r'\s*=\s*"((?:[^"\\]|\\.)*)"\s*;\s*$', text[cut:], re.S)
                size = len(string.group(1).encode().decode('unicode_escape')) + 1 if string else None
                head = text[:cut].rstrip()
                if size and re.search(r'\[\s*\]\s*$', head):
                    head = re.sub(r'\[\s*\]\s*$', f'[{size}]', head)
                text = head + ';'
            text = re.sub(r'\bstatic\s+', '', text)
            text = re.sub(r'\bextern\s+', '', text)
            return 'extern ' + text.strip()
        return text


class Header:
    """A header X3 also has, kept as one #include."""

    kind = 'include'
    is_proto = is_data = is_static = inline = False
    comment = None
    masked = ''

    def __init__(self, name, path):
        self.name, self.path = name, path
        text = path.read_text(errors='replace') if path else ''
        self.names = set(DEFINE_LINE.findall(text))
        self.uses = set()
        self.text = f'#include "{name}"'

    def emit(self, keep_static=False):
        return self.text


def resolve_include(name, source, unit):
    for folder in (source.parent, SRC / 'build' / unit / 'include', SRC / 'include'):
        candidate = folder / name
        if candidate.is_file():
            return candidate
    return None


def walk_file(path, unit, seen, out):
    """Every top-level item a translation unit sees, in order, through its includes."""
    if path in seen:
        return
    seen.add(path)
    text = path.read_text(errors='replace')
    pending_comment, pending_end = None, 0
    for item in tu_edit.scan(text):
        raw = item.text(text)
        if item.kind == 'comment':
            pending_comment, pending_end = raw, item.end
            continue
        comment = pending_comment if pending_comment and text[pending_end:item.start].count('\n') <= 1 else None
        pending_comment = None
        if item.kind == 'pp':
            include = INCLUDE_LINE.match(raw)
            if include:
                name = include.group(1)
                base = name.split('/')[-1]
                if base in X3_ATOMIC:
                    out.append(Header(name, ROOT / 'include' / base))
                    continue
                found = resolve_include(name, path, unit)
                if found and base not in ('common.h', 'include_asm.h'):
                    walk_file(found, unit, seen, out)
            elif DEFINE_LINE.match(raw) and not raw.rstrip().endswith('_H'):
                out.append(Declaration('define', raw, comment, path))
        elif item.kind in ('decl', 'func'):
            out.append(Declaration(item.kind, raw, comment, path))


def provided_names():
    names = set()
    for base in X3_HEADERS:
        text = (ROOT / 'include' / base).read_text()
        for item in tu_edit.scan(text):
            raw = item.text(text)
            if item.kind == 'pp' and DEFINE_LINE.match(raw):
                names.add(DEFINE_LINE.match(raw).group(1))
            elif item.kind == 'decl':
                names |= Declaration('decl', raw, None, None).names
    return names


def rewrite(text, tokens):
    """Rename whole identifiers in code; literals, comments and field accesses stay."""
    if not tokens:
        return text
    masked = tu_edit.lexical(text)
    out, last = [], 0
    for match in IDENTIFIER.finditer(masked):
        new = tokens.get(match.group())
        if new and new != match.group() and not MEMBER.search(masked[max(0, match.start() - 4):match.start()]):
            out += [text[last:match.start()], new]
            last = match.end()
    return ''.join(out) + text[last:]


class Sources:
    def __init__(self):
        self.cache = {}
        self.provided = provided_names()

    def items(self, path, unit):
        if path not in self.cache:
            out = []
            walk_file(SRC / path, unit, set(), out)
            self.cache[path] = out
        return self.cache[path]


def asm_references(unit):
    """{label: set of the .s files that spell it}, for deciding whether a static stays static."""
    refs = collections.defaultdict(set)
    folder = ROOT / 'build' / unit / 'asm'
    for path in folder.rglob('*.s'):
        relative = str(path.relative_to(folder))
        if '/blobs/' in relative:
            continue
        for word in set(IDENTIFIER.findall(path.read_text(errors='replace'))):
            refs[word].add(relative)
    return refs


def data_table(unit):
    path = ROOT / 'config/symbols/data-names.json'
    if not path.is_file():
        return {}
    return {int(va, 16): name for va, name in (json.loads(path.read_text())['units'].get(unit) or {}).items()}


def generate_tu(record, picks, sources, tables, refs, scaffold):
    """X3's scaffold with these functions replaced by X2's text, plus the declarations they need."""
    unit = record['unit']
    wanted = {int(pick['va'], 16): pick for pick in picks.values()}
    final_name = {va: tables[unit][va][0] for va in wanted}
    tokens = {}
    data = data_table(unit)
    if unit != 'main':
        data = {**data_table('main'), **data}
    problems = []

    def add_token(src_name, x3_name, va):
        if tokens.get(src_name, x3_name) != x3_name:
            problems.append((va, f'{src_name} means both {tokens[src_name]} and {x3_name}'))
        tokens[src_name] = x3_name
    for va, pick in wanted.items():
        add_token(pick['x2']['name'], final_name[va], va)
        for reference in pick['references']:
            name, base = reference.get('name'), reference['base']
            if not name:
                continue
            known = lookup(tables, unit, base)
            add_token(name, known[0] if known else data.get(base, f'D_{base:08X}'), va)
    if problems:
        return None, problems
    primary = collections.Counter()
    for pick in wanted.values():
        primary[pick['x2']['path']] += pick['size']
    bodies = {}
    for va, pick in wanted.items():
        found = [item for item in sources.items(pick['x2']['path'], pick['x2']['unit'])
                 if item.kind == 'func' and pick['x2']['name'] in item.names]
        if not found:
            problems.append((va, f"no definition of {pick['x2']['name']} in {pick['x2']['path']}"))
            continue
        bodies[va] = found[-1]
    if problems:
        return None, problems
    carried = {pick['x2']['name'] for pick in wanted.values()}
    emitted_text, order = set(), []
    path_unit = {pick['x2']['path']: pick['x2']['unit'] for pick in wanted.values()}
    for path, _ in primary.most_common():
        items = sources.items(path, path_unit[path])
        by_name = collections.defaultdict(list)
        for item in items:
            for name in item.names:
                by_name[name].append(item)
        needed = set()
        for va, pick in wanted.items():
            if pick['x2']['path'] == path:
                needed |= bodies[va].uses
        included, stack, visited = set(), list(needed), set()
        while stack:
            name = stack.pop()
            if name in visited or name in sources.provided:
                continue
            visited.add(name)
            for item in by_name.get(name, ()):
                if id(item) in included or (item.kind == 'func' and name in carried and not item.inline):
                    continue
                included.add(id(item))
                stack.extend(item.uses)
        for item in items:
            if id(item) in included:
                if item.names and item.names <= sources.provided:
                    continue
                order.append(item)
    stem = Path(record['path']).stem
    replaced = {f'{stem}/{final_name[va]}.s' for va in wanted}
    keep_static = set()
    for va in wanted:
        body = bodies[va]
        if not body.is_static:
            continue
        spelled = refs.get(final_name[va], set())
        own_file = f'{stem}/{final_name[va]}.s'
        others = {r for r in spelled if not any(r.endswith(x) for x in replaced)}
        users = {r for r in spelled if any(r.endswith(x) for x in replaced) and not r.endswith(own_file)}
        if not others and users:
            keep_static.add(final_name[va])
    lines = []
    keep_names = {src for src, x3 in tokens.items() if x3 in keep_static}
    for item in order:
        if item.kind == 'func' and item.inline:
            text = item.text
        else:
            text = item.emit(keep_static=bool(item.names & keep_names))
        text = rewrite(text, tokens)
        if text in emitted_text:
            continue
        emitted_text.add(text)
        comment = item.comment + '\n' if item.comment and item.kind in ('define', 'decl') and not item.is_proto else ''
        lines.append(comment + text)
    text = scaffold.read_text()
    out, last = [], 0
    for item in tu_edit.scan(text):
        if item.kind != 'asm':
            continue
        va = next((v for v, n in final_name.items() if n == item.name), None)
        if va is None:
            continue
        body = bodies[va]
        source = rewrite(body.text, tokens)
        if final_name[va] not in keep_static:
            source = re.sub(r'^static\s+', '', source)
        comment = body.comment + '\n' if body.comment else ''
        out += [text[last:item.start], comment + source]
        last = item.end
    out.append(text[last:])
    merged = ''.join(out)
    prelude = '#include "common.h"\n'
    head_at = merged.index(prelude) + len(prelude)
    declarations = '#include "shared.h"\n\n'
    for index, line in enumerate(lines):
        declarations += line + '\n'
        following = lines[index + 1] if index + 1 < len(lines) else None
        if following is None or not (line.count('\n') == 0 and following.count('\n') == 0
                                     and line.split()[0] == following.split()[0]):
            declarations += '\n'
    declarations = declarations.rstrip('\n') + '\n'
    return merged[:head_at] + declarations + merged[head_at:], []


def command_source(args):
    import express
    plan = json.loads((args.work / 'plan.json').read_text())
    tables = x3_tables()
    by_tu = collections.defaultdict(dict)
    for key, pick in plan['chosen'].items():
        if pick['unit'] == args.unit and (not args.tu or pick['tu'] in args.tu):
            # A TU whose every pick is skipped stays listed, so it is withdrawn below rather than left stale.
            picks = by_tu[pick['tu']]
            if pick['x2']['name'] not in args.skip:
                picks[key] = pick
    kit = args.kit.resolve()
    claim_kit(kit, args.unit)
    (kit / 'merged').mkdir(parents=True, exist_ok=True)
    (kit / f'base/src/{args.unit}').mkdir(parents=True, exist_ok=True)
    stage = kit / 'stage/src' / args.unit
    stage.mkdir(parents=True, exist_ok=True)
    sources, refs = Sources(), asm_references(args.unit)
    outcome_path = kit / 'outcome.json'
    outcome = json.loads(outcome_path.read_text()) if outcome_path.is_file() else dict(accepted={}, dropped=[])
    outcome['dropped'] = [d for d in outcome['dropped'] if d['tu'] not in by_tu]
    original = express.Original(args.unit)
    # KIT/fixups.json: {"stem.c": [["old", "new"], ...]}, applied once each to the generated text, so a rerun
    # reproduces every hand reconciliation of two X2 files that disagree.
    fixups = json.loads((kit / 'fixups.json').read_text()) if (kit / 'fixups.json').is_file() else {}
    for tu_id in sorted(by_tu):
        record = json.loads((ROOT / 'config/tu' / args.unit / f"{tu_id.split('/')[1]}.json").read_text())
        stem = Path(record['path']).stem
        scaffold = ROOT / 'build' / args.unit / 'scaffold/src' / args.unit / f'{stem}.c'
        base = kit / f'base/src/{args.unit}' / f'{stem}.c'
        base.write_text(scaffold.read_text())
        picks = dict(by_tu[tu_id])
        check_record = dict(record)
        first = True
        while picks:
            text, problems = generate_tu(record, picks, sources, tables, refs, scaffold)
            for old, new in fixups.get(f'{stem}.c', []) if not problems else []:
                if old not in text and not first:
                    continue            # its text went with a function dropped by an earlier pass
                if old not in text:
                    (kit / f'fixup-miss-{stem}.c').write_text(text)
                    raise SystemExit(f'{stem}.c: fixup text not found: {old[:80]!r} (text in {kit}/fixup-miss-{stem}.c)')
                text = text.replace(old, new, 1)
            if problems:
                for va, why in problems:
                    key = f'{args.unit}:0x{va:08x}'
                    if key in picks:
                        outcome['dropped'].append(dict(tu=tu_id, key=key, x2=picks.pop(key)['x2']['name'], why=why))
                continue
            (kit / 'merged' / f'{stem}.c').write_text(text)
            (stage / f'{stem}.c').write_text(text)
            names = {tables[args.unit][int(p['va'], 16)][0]: key for key, p in picks.items()}
            check_record['functions'] = [f for f in record['functions'] if f['name'] in names]
            result = express.check_tu(args.unit, kit, f'{stem}.c', check_record, original)
            if 'err' in result:
                culprit = None
                for line in result['err']:
                    m = re.search(r"In function `([A-Za-z_]\w*)'|In function '([A-Za-z_]\w*)'", line)
                    if m and (m.group(1) or m.group(2)) in names:
                        culprit = m.group(1) or m.group(2)
                        break
                if culprit is None and len(picks) > 1:
                    # Name the failing line's function by its line number in the merged text.
                    for line in result['err']:
                        m = re.search(rf'{re.escape(stem)}\.c:(\d+)', line)
                        if m:
                            culprit = function_at_line(text, int(m.group(1)), names)
                            if culprit:
                                break
                if culprit is None:
                    outcome['dropped'].append(dict(tu=tu_id, key=None, x2=None,
                                                   why='compile: ' + ' | '.join(result['err'][:6])))
                    print(f'{tu_id}: COMPILE {result["err"][:3]}')
                    picks = {}
                    break
                key = names[culprit]
                outcome['dropped'].append(dict(tu=tu_id, key=key, x2=picks.pop(key)['x2']['name'],
                                               why='compile: ' + ' | '.join(result['err'][:4])))
                print(f'{tu_id}: drop {culprit}: {outcome["dropped"][-1]["why"][:200]}')
                first = False
                continue
            bad = [line.split()[1] for line in result['bad']] + result['missing']
            if not bad:
                break
            for name in bad:
                key = names[name]
                outcome['dropped'].append(dict(tu=tu_id, key=key, x2=picks.pop(key)['x2']['name'],
                                               why=next((b for b in result['bad'] if name in b), 'missing symbol')))
                print(f'{tu_id}: drop {name}: {outcome["dropped"][-1]["why"][:200]}')
            first = False
        if picks:
            outcome['accepted'][tu_id] = sorted(picks)
            print(f'{tu_id}: {len(picks)} functions, {sum(p["size"] for p in picks.values())} bytes')
        else:
            if outcome['accepted'].pop(tu_id, None) is not None:
                outcome.setdefault('withdrawn', []).append(tu_id)   # `publish` puts its scaffold back
            (kit / 'merged' / f'{stem}.c').unlink(missing_ok=True)
            (stage / f'{stem}.c').unlink(missing_ok=True)
            base.unlink(missing_ok=True)
    outcome_path.write_text(json.dumps(outcome, indent=1) + '\n')
    total = sum(len(v) for t, v in outcome['accepted'].items() if t.startswith(args.unit + '/'))
    print(f'{args.unit}: accepted {total} functions in '
          f'{sum(1 for t in outcome["accepted"] if t.startswith(args.unit + "/"))} TUs; '
          f'dropped {sum(1 for d in outcome["dropped"] if d["tu"].startswith(args.unit + "/"))}')
    for item in [d for d in outcome['dropped'] if d['tu'] in by_tu][:args.show]:
        print(f'  {item["tu"]} {item["key"]} {item["x2"]}: {item["why"][:230]}')


def claim_kit(kit, unit):
    """One kit per unit: KIT/merged is keyed by file name, and overlay stems (tuNNN.c) repeat across units."""
    kit.mkdir(parents=True, exist_ok=True)
    marker = kit / 'unit'
    if marker.is_file() and marker.read_text().strip() != unit:
        raise SystemExit(f'{kit} is the kit of {marker.read_text().strip()}, not {unit}: use one kit per unit')
    marker.write_text(unit + '\n')


def function_at_line(text, line, names):
    """The function among `names` whose definition spans `line` of `text`, or None."""
    offset = sum(len(l) + 1 for l in text.split('\n')[:line - 1])
    for item in tu_edit.scan(text):
        if item.kind == 'func' and item.start <= offset < item.end:
            found = tu_edit.declarator_name(tu_edit.lexical(item.text(text)).split('{')[0])
            return found if found in names else None
    return None


def command_publish(args):
    """Copy what `source` kept into src/; the main unit also needs `express.py enable --write`."""
    kit = args.kit.resolve()
    claim_kit(kit, args.unit)
    outcome = json.loads((kit / 'outcome.json').read_text())
    (ROOT / 'src' / args.unit).mkdir(parents=True, exist_ok=True)
    count = 0
    for tu_id in sorted(t for t in outcome['accepted'] if t.startswith(args.unit + '/')):
        record = json.loads((ROOT / 'config/tu' / args.unit / f"{tu_id.split('/')[1]}.json").read_text())
        (ROOT / record['path']).write_text((kit / 'merged' / Path(record['path']).name).read_text())
        count += 1
    withdrawn = [t for t in outcome.get('withdrawn', []) if t.startswith(args.unit + '/') and t not in outcome['accepted']]
    for tu_id in withdrawn:
        # The source file is this port's own publication: without it the TU builds from its scaffold again.
        record = json.loads((ROOT / 'config/tu' / args.unit / f"{tu_id.split('/')[1]}.json").read_text())
        (ROOT / record['path']).unlink(missing_ok=True)
    outcome['withdrawn'] = [t for t in outcome.get('withdrawn', []) if t not in withdrawn]
    (kit / 'outcome.json').write_text(json.dumps(outcome, indent=1) + '\n')
    print(f'{args.unit}: published {count} TUs, withdrew {len(withdrawn)} {withdrawn}')


def command_classes(args):
    """Carry X2's source class of each published function, and native-ti to a TU whose text needs it.

    Run before `express.py ledger`, which keeps a class recorded before it. X2's classes are the
    admitted generic ones (hardware, ee-mmi, ee-assert-break, native-ti); an admission bound to one X2
    function is not a class here and never carried (docs/agents-digest.md).
    """
    plan = json.loads((args.work / 'plan.json').read_text())
    x2_classes = json.loads((SRC / 'config/tu/source-classes.json').read_text())['tus']
    x2_ids = {tu['path']: tu['id'] for tu in json.loads((SRC / 'config/tu-build.json').read_text())['tus'] if tu.get('path')}
    generic = {'hardware', 'ee-mmi', 'ee-assert-break', 'native-ti', 'ee-fpu', 'ee-vu-clip', 'ee-asm-leaf',
               'ee-sync-barrier', 'ee-interrupt-handler-return'}
    path = ROOT / 'config/tu/source-classes.json'
    classes = json.loads(path.read_text())
    import express                      # noqa: PLC0415 - heavy (loads tu_audit), only these commands need it
    tables = x3_tables()
    macros = express.hardware_macros()
    express_pattern = re.compile(r'\b(__asm__|asm|' + '|'.join(sorted(map(re.escape, macros))) + r')\b')
    written = collections.Counter()
    for key, pick in sorted(plan['chosen'].items()):
        unit, tu = pick['tu'].split('/')
        record = json.loads((ROOT / 'config/tu' / unit / f'{tu}.json').read_text())
        source = ROOT / record['path']
        if not source.is_file():
            continue
        name = tables[unit][int(pick['va'], 16)][0]
        text = source.read_text()
        if re.search(rf'INCLUDE_ASM\([^)]*\b{re.escape(name)}\)', text):
            continue
        x2_tu = x2_ids.get(pick['x2']['path'])
        if 'mode(TI)' in text and classes['tus'].get(pick['tu'], {}).get('source_class') != 'native-ti':
            classes['tus'].setdefault(pick['tu'], {}).update(
                source_class='native-ti',
                reason=f'File-scope mode(TI) typedef carried with X2 C ({pick["x2"]["path"]}); {ORDER}.')
            written['native-ti TU'] += 1
        entry = (x2_classes.get(x2_tu) or {}).get('functions', {}).get(pick['x2']['name']) or {}
        function_class = entry.get('source_class')
        body = express.function_body(text, name)
        inferred = express.infer_class(body, express_pattern) if body else None
        if inferred in ('ee-interrupt-handler-return', 'ee-sync-barrier'):
            function_class = inferred   # a fixup rewrote X2's handler return into the admitted statements
        if function_class in generic:
            classes['tus'].setdefault(pick['tu'], {}).setdefault('functions', {})[name] = dict(
                source_class=function_class,
                reason=f'Class of X2 {x2_tu} {pick["x2"]["name"]}, whose C this is ({ORDER}).')
            written[function_class] += 1
        elif function_class:
            written[f'not carried: {function_class}'] += 1
    path.write_text(json.dumps(classes, indent=1, ensure_ascii=False) + '\n')
    print('classes written', dict(written))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest='command', required=True)
    match = sub.add_parser('match')
    match.add_argument('--out', type=Path, required=True)
    match.add_argument('--unit', action='append')
    match.add_argument('--cls', action='append')
    match.add_argument('-j', '--jobs', type=int, default=4)
    plan = sub.add_parser('plan')
    plan.add_argument('--work', type=Path, required=True)
    plan.add_argument('--write', action='store_true')
    source = sub.add_parser('source')
    source.add_argument('--work', type=Path, required=True)
    source.add_argument('--kit', type=Path, required=True)
    source.add_argument('--unit', required=True)
    source.add_argument('--tu', action='append')
    source.add_argument('--skip', action='append', default=[], help='X2 function to leave as INCLUDE_ASM')
    source.add_argument('--show', type=int, default=30)
    publish = sub.add_parser('publish')
    publish.add_argument('--kit', type=Path, required=True)
    publish.add_argument('--unit', required=True)
    classes = sub.add_parser('classes')
    classes.add_argument('--work', type=Path, required=True)
    args = parser.parse_args(argv)
    if args.command == 'classes':
        args.work = args.work.resolve()
        command_classes(args)
    elif args.command == 'match':
        args.out = args.out.resolve()
        args.out.mkdir(parents=True, exist_ok=True)
        command_match(args)
    elif args.command == 'plan':
        args.work = args.work.resolve()
        command_plan(args)
    elif args.command == 'source':
        args.work = args.work.resolve()
        command_source(args)
    elif args.command == 'publish':
        command_publish(args)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
