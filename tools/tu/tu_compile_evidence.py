#!/usr/bin/env python3
"""Which callees must share a file with their caller, by the compiler itself.

cc1 knows that a function the file already compiled does not throw, and its delay-slot pass then emits other code
around the calls to it. For every recovered function of X3 main, this compiles the function with each earlier callee
visible as C and not visible:

  join  the published code needs the callee compiled before it in the same file
  cut   the published code would change if the callee, in another TU today, were in the same file

The test trusts the recovered C, which was written under the current cuts: it confirms or refutes them, and another C of
the same function could answer otherwise. Needs a configured main build (the preprocessed .i files). Writes
config/tu-compile-evidence.json; tu_map.py reads it. Nothing else changes.
"""
import argparse
import concurrent.futures
import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import tu_limits as tl  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'config/tu-compile-evidence.json'
BUILD = ROOT / 'build/main/build/c'
COMPILER = ROOT / '.work/toolchains/ee-gcc2.96-realconv-lp7-fsv17-v1/bin'
REACH = 150          # functions between caller and callee
BASE_TYPES = {'int', 'char', 'short', 'long', 'float', 'double', 'void'}


def top_level(text):
    """(start, end) of the top-level declarations and definitions of preprocessed C."""
    out, i, start, depth = [], 0, 0, 0
    while i < len(text):
        c = text[i]
        if c in '"\'':
            i += 1
            while i < len(text) and text[i] != c:
                i += 2 if text[i] == '\\' else 1
        elif c == '#' and (i == 0 or text[i - 1] == '\n'):
            while i < len(text) and text[i] != '\n':
                i += 1
        elif c in '{(':
            depth += 1
        elif c in '})':
            depth -= 1
            if c == '}' and depth == 0 and text[start:i].split('{', 1)[0].rstrip().endswith(')'):
                out.append((start, i + 1))
                start = i + 1
        elif c == ';' and depth == 0:
            out.append((start, i + 1))
            start = i + 1
        i += 1
    out.append((start, len(text)))
    return out


def defined_name(text):
    head = text.split('{', 1)
    if len(head) < 2 or '=' in head[0] or not head[0].rstrip().endswith(')'):
        return None
    head = re.sub(r'^\s*#.*$', '', head[0], flags=re.M)
    match = re.search(r'\b([A-Za-z_]\w*)\s*\([^()]*(?:\([^()]*\)[^()]*)*\)\s*$', head)
    return match.group(1) if match else None


def name_parameters(text):
    """Parameter list of a prototype with a name for each parameter, for a definition."""
    if text.strip() in ('', 'void'):
        return text
    parts, depth, current = [], 0, ''
    for c in text + ',':
        if c == ',' and depth == 0:
            parts.append(current.strip())
            current = ''
            continue
        depth += (c in '([') - (c in ')]')
        current += c
    out = []
    for k, part in enumerate(parts):
        if '(' in part:
            part = re.sub(r'\(\s*\*\s*\)', f'(*p{k})', part, count=1)
        elif '[' in part:
            if not re.search(r'\w\s+\*?\w+\s*\[', part):
                part = part.replace('[', f' p{k}[', 1)
        elif part != '...':
            words = re.sub(r'\b(const|volatile|struct|union|enum|unsigned|signed|register)\b', '', part).replace('*', ' ').split()
            if len(words) <= 1 or all(w in BASE_TYPES for w in words):
                part += f' p{k}'
        out.append(part)
    return ', '.join(out)


def compile_functions(source, folder):
    """name -> assembly with renumbered labels, or None when the source does not compile."""
    path = Path(folder) / 't.i'
    path.write_text(source, encoding='latin1')
    run = subprocess.run(['timeout', '-k', '5', '30', str(COMPILER / 'ee-gcc'), f'-B{COMPILER}/', '-nostdinc', '-fno-builtin', '-S',
                          '-O2', '-G4', str(path), '-o', str(path.with_suffix('.s'))], capture_output=True)
    if run.returncode:
        return None
    out = {}
    for match in re.finditer(r'^\t\.ent\t(\w+)\n(.*?)^\t\.end\t\1', path.with_suffix('.s').read_text(encoding='latin1'), re.S | re.M):
        labels = {}
        body = re.sub(r'\$L[A-Z]*\d+', lambda m: labels.setdefault(m.group(0), f'$L{len(labels)}'), match.group(2))
        out[match.group(1)] = '\n'.join(line for line in body.split('\n') if not re.match(r'\s*(\.loc|\.file|#)', line))
    return out


def examine(job):
    """job: source path, and caller -> its earlier callees, by name."""
    path, callees = job
    result = dict(joins=[], cuts=[], tested=0, skipped=0)
    text = Path(path).read_text(encoding='latin1')
    parts = top_level(text)
    place = {}
    for k, (a, b) in enumerate(parts):
        name = defined_name(text[a:b])
        if name:
            place[name] = k
    by_place = {k: n for n, k in place.items()}
    inline = {n for n, k in place.items() if re.search(r'inline', text[parts[k][0]:parts[k][1]].split('{', 1)[0])}
    with tempfile.TemporaryDirectory() as folder:
        whole = compile_functions(text, folder)
        if whole is None:
            return dict(result, skipped=1)

        def variant(name, hidden=(), stubs=''):
            out = []
            for k, (a, b) in enumerate(parts[:place[name] + 1]):
                part, mine = text[a:b], by_place.get(k)
                head = part.split('{', 1)[0]
                if mine == name:
                    part = '\n' + stubs + part
                elif mine in hidden:
                    part = head + ';'
                elif mine and mine not in inline and mine in KNOWN:
                    part = head + '{ }'
                out.append(part)
            return (compile_functions(''.join(out), folder) or {}).get(name)
        for caller, names in callees.items():
            if caller not in place or caller not in whole:
                continue
            inside = [c for c in names if c in place and place[c] < place[caller] and c not in inline]
            stubs = {}
            for c in names:
                if c in place:
                    continue
                found = None
                for found in re.finditer(r'(?<![\w.])((?:extern\s+)?[A-Za-z_][\w\s\*]*?[\s\*])' + c + r'\s*\(((?:[^()]|\([^()]*\))*)\)\s*;',
                                         text[:parts[place[caller]][0]]):
                    pass
                if found is None:
                    stubs[c] = f'int {c}() {{ }}\n'
                elif 'return' not in found.group(1):
                    stubs[c] = f'{found.group(1).replace("extern", "").strip()} {c}({name_parameters(found.group(2))}) {{ }}\n'
            if not inside and not stubs:
                continue
            base = variant(caller)
            if base is None or base != whole[caller]:
                result['skipped'] += 1
                continue
            result['tested'] += len(inside) + len(stubs)
            if inside and variant(caller, inside) != base:
                result['joins'] += [(c, caller) for c in inside if (variant(caller, [c]) or base) != base]
            if stubs and variant(caller, (), ''.join(stubs.values())) != base:
                result['cuts'] += [(c, caller) for c, stub in stubs.items() if (variant(caller, (), stub) or base) != base]
    return result


KNOWN = set()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--jobs', type=int, default=8)
    args = parser.parse_args(argv)
    audit = tl.Audit()
    unit = audit.unit
    KNOWN.update(audit.names.values())
    jobs, missing = [], []
    for tu in audit.tus:
        path = BUILD / f'{tu["name"]}.o.i'
        if not path.is_file():
            missing.append(tu['name'])
            continue
        callees = {}
        for f in tu['functions']:
            i = unit.index[int(f['va'], 16)]
            names = {audit.name(unit.index[t]) for t in unit.calls[i]
                     if t in unit.index and unit.index[t] in audit.owner and 0 < i - unit.index[t] <= REACH}
            if names:
                callees[f['name']] = sorted(names)
        jobs.append((str(path), callees))
    with concurrent.futures.ProcessPoolExecutor(args.jobs) as pool:
        results = list(pool.map(examine, jobs))
    va = {name: start for start, name in audit.names.items()}
    doc = dict(schema='tu-compile-evidence/1', original_sha256=audit.image.sha256)
    for key in ('joins', 'cuts'):
        rows = sorted({(va[c], va[f]) for r in results for c, f in r[key]})
        doc[key] = [[hex(c), hex(f)] for c, f in rows]
    OUT.write_text(json.dumps(doc, indent=1) + '\n')
    print(json.dumps(dict(out=str(OUT.relative_to(ROOT)), joins=len(doc['joins']), cuts=len(doc['cuts']),
                          tested=sum(r['tested'] for r in results), skipped=sum(r['skipped'] for r in results),
                          tus_without_c=len(missing))))


if __name__ == '__main__':
    main()
