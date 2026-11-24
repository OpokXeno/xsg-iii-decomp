#!/usr/bin/env python3
"""Report of the translation-unit limits of X3 main: which cuts the data proves, refutes or leaves open.

    tu_limits.py calibrate        measure every rule on X1 (symbols give the truth), write the model
    tu_limits.py report           audit the current cuts, write reports/x3-tu-limits.md and .json
    tu_limits.py show 0x1E8850    the evidence at one function start

A gap is the place between two consecutive functions. Each rule reads only original bytes (code references,
strings, section order), never the recovered C:

  join rules   a range of gaps that one object covers, so no limit can be there
  cut rules    a range of gaps that holds at least one limit

`calibrate` counts each rule's errors on X1 main, and the errors that ranges of the same lengths make by
chance there (one gap in twenty is a limit, so short ranges are right by luck). A rule is `hard` with no error where
chance makes MIN_CHANCE or more, `soft` with at most SOFT_SHARE of the chance errors, and `off` otherwise. Off rules are
reported and never used. The thresholds of the rules were chosen on that same unit, so the error counts are
optimistic; the split by halves shows how far.

X1 alone is not enough: a rule can be right there and wrong on X3. `report` repeats the test on X3, against ranges that
hold a limit by other evidence (equal bodies in two X1 objects, two __FILE__ names, two copies of a string, .lit4 going
back). A join that covers such a range whole is wrong. One error makes a hard rule soft; more than SOFT_SHARE of the
chance errors turns it off. Nothing here edits the registry, and every verdict is analysis: the whole-file gate cannot see
a TU that is cut too often.
"""
import argparse
import bisect
import collections
import hashlib
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import map_infer as mi  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
MODEL = ROOT / 'config/tu-limits-model.json'
REGISTRY = ROOT / 'config/objects/main.objects.json'
CARVES = ROOT / 'config/objects/data-carves.json'
ORIGINAL = ROOT / 'game/extracted-iso/SLUS_213.89'
ANNOTATIONS = ROOT / 'annotations/slus_213.89_annotations.csv'
REPORT = ROOT / 'reports/x3-tu-limits.md'

MIN_CHANCE = 3.0     # about 5% to see no error when the rule knows nothing
SOFT_SHARE = 0.35
HOMOLOG_BYTES = 64   # smaller equal bodies are not evidence of one source
POOL_REACH = 512    # bytes to the neighbour datum whose order is checked
LIT4_BACK = 16       # bytes: neighbours swap closer constants inside one object
SECTIONS = ('.rodata', '.sdata', '.sbss', '.bss', '.data')
# (section, span of a private datum's users, functions between the two users, bytes between the two data)
INVERSIONS = [(s, 8, 4, 1024) for s in SECTIONS] + [
    ('.rodata', 8, 16, 1024), ('.sdata', 0, 16, 8192), ('.sbss', 8, 16, 8192), ('.bss', 8, 64, 8192),
    ('.data', 32, 4, 65536), ('.data', 8, 16, 1024), ('.bss', 8, 256, 65536), ('.data', 8, 64, 8192),
    ('.sbss', 8, 64, 8192), ('.sdata', 8, 64, 8192)]
# (section, longest span of the users of one datum)
SHARED = [(s, span) for s in SECTIONS for span in (8, 32, 128)]
# (section, span of a private datum, functions between the users of the outer data, functions to the inner one)
SANDWICHES = [(s, 8, 32, far) for s in ('.sbss', '.bss', '.data', '.sdata') for far in (64, 256)] + [
    (s, 0, 0, 256) for s in ('.rodata', '.sdata', '.bss', '.data')]


class LimitUnit(mi.Unit):
    """A Unit over a given function table ([(va, end)]) instead of the estimated one."""

    def __init__(self, image, table=None):
        if table is None:
            super().__init__(image)
            return
        self.image = image
        found, self.starts = mi.find_functions(image)
        low, high = min(va for va, _ in table), max(end for _, end in table)
        rows = [dict(va=va, end=end) for va, end in table]
        rows += [f for f in found if not low <= f['va'] < high]
        inside = [(va, end) for va, end in table]
        rows += [f for f in found if low <= f['va'] < high and not any(a <= f['va'] < b for a, b in
                 inside[max(0, bisect.bisect_right(inside, (f['va'], 1 << 40)) - 1):][:1])]
        self.functions = sorted(rows, key=lambda f: f['va'])
        self.vas = [f['va'] for f in self.functions]
        self.index = {va: i for i, va in enumerate(self.vas)}
        self.refs = [mi.scan_refs(image, f) for f in self.functions]
        self.calls = [mi.scan_calls(image, f) for f in self.functions]
        self._section_users = {}
        self._link()


def private_items(unit, section, span):
    """[(address, first user, last user)] of the data whose users all lie within `span` functions."""
    return sorted((target, min(who), max(who)) for target, who in unit.users.items()
                  if unit.image.section_of(target) == section and max(who) - min(who) <= span)


def inversion_spans(unit, section, span, window, distance):
    high, low = {}, {}
    for target, first, last in private_items(unit, section, span):
        for i in unit.users[target]:
            high[i] = max(high.get(i, target), target)
            low[i] = min(low.get(i, target), target)
    order = sorted(high)
    for position, i in enumerate(order):
        for j in order[position + 1:]:
            if j - i > window:
                break
            if 0 < high[i] - low[j] <= distance:
                yield i, j, f'{section} {high[i]:#x} before {low[j]:#x}'


def shared_spans(unit, section, span):
    """A datum used by several functions. cc1 emits a pooled .rodata constant at its first user, so there the
    neighbour data must keep that order: one that does not may be an extern of another file."""
    items = private_items(unit, section, span)
    for k, (target, first, last) in enumerate(items):
        if last == first:
            continue
        if section == '.rodata':
            before, after = items[k - 1] if k else None, items[k + 1] if k + 1 < len(items) else None
            if before and target - before[0] <= POOL_REACH and before[1] > first:
                continue
            if after and after[0] - target <= POOL_REACH and after[1] < first:
                continue
        yield first, last, f'{section} {target:#x} used by both sides'


def sandwich_spans(unit, section, span, near, far):
    """Data a < c < b in a row: the users of a and b are close, and c's user sits elsewhere."""
    items = private_items(unit, section, span)
    for (a, a1, a2), (c, c1, c2), (b, b1, b2) in zip(items, items[1:], items[2:]):
        first, last = min(a1, b1), max(a2, b2)
        if last - first > near or b - a > 2 * 16384:
            continue
        if (c1 > last and c2 - last <= far) or (c2 < first and first - c1 <= far):
            yield min(first, c1), max(last, c2), f'{section} {c:#x} between {a:#x} and {b:#x}'


def join_rules():
    """name -> function(unit) giving (first function, last function, note): one object covers them."""
    rules = collections.OrderedDict()
    rules['text'] = lambda u: ((a, b, f'literal at {t:#x} used by both sides') for a, b, t, _ in u.text_spans)
    rules['string'] = lambda u: ((a, b, 'string used by both sides') for a, b in u.string_spans)
    for args in SHARED:
        rules['shared %s span %d' % args] = lambda u, args=args: shared_spans(u, *args)
    for args in INVERSIONS:
        rules['inversion %s private %d window %d distance %d' % args] = lambda u, args=args: inversion_spans(u, *args)
    for args in SANDWICHES:
        rules['sandwich %s private %d near %d far %d' % args] = lambda u, args=args: sandwich_spans(u, *args)
    return rules


def cut_intervals(unit):
    """(first gap, last gap, rule, note): at least one limit is inside.

    .lit4 going back is told from .lit4 jumping ahead: one object fills its pool in text order, so the first needs a
    limit, while the second may be a user that the scan misses.
    """
    for low, high, kind, note in unit.forced_cuts():
        if kind == 'lit4_step':
            a, b = (int(x, 16) for x in note.replace('.lit4', '').split('->'))
            if 0 < a - b < LIT4_BACK:
                continue
            kind = 'lit4_back' if b < a else 'lit4_jump'
        yield low, high, kind, note


# ------------------------------------------------------------------ the control on X3 itself

CONTROL = ('x1_homolog', 'file_path', 'dup_string', 'lit4_back')


def homolog_cuts(unit):
    """Ranges between neighbouring functions whose equal bodies sit in two X1 objects."""
    _, homologs = mi.load_annotations(ANNOTATIONS)
    x1 = {}
    for va, row in homologs.items():
        i = unit.index.get(va)
        if row[2] == 'X1' and i is not None and unit.functions[i]['end'] - va >= HOMOLOG_BYTES:
            x1[va] = row
    return mi.homolog_constraints(unit, x1, set(range(len(unit.vas))))[1]


def control_cuts(unit):
    """Ranges of X3 gaps that hold a limit by evidence no join rule reads, shortest first.

    x1_homolog: neighbouring functions whose equal bodies sit in two X1 objects, where symbols give the objects.
    """
    rows = homolog_cuts(unit) + [row for row in cut_intervals(unit) if row[2] in CONTROL]
    return sorted({(low, high, kind) for low, high, kind, _ in rows}, key=lambda row: row[1] - row[0])


def control_rules(unit, scope, rules):
    """name -> (spans, spans that cover a control range whole, the same by chance)."""
    cuts = control_cuts(unit)
    inside = set(scope)
    first_after = [None] * (len(unit.vas) + 1)       # per start: the nearest end of a control range at or after it
    best = None
    ends = collections.defaultdict(list)
    for low, high, _ in cuts:
        ends[low].append(high)
    for i in range(len(unit.vas) - 1, -1, -1):
        for high in ends.get(i, ()):
            best = high if best is None else min(best, high)
        first_after[i] = best

    def covers(first, last):
        end = first_after[first]
        return end is not None and end < last
    chance = {}

    def by_chance(length):
        if length not in chance:
            runs = [covers(g, g + length) for g in scope if g + length - 1 in inside]
            chance[length] = sum(runs) / len(runs) if runs else 0.0
        return chance[length]
    out = {}
    for name, rule in rules.items():
        spans = sorted({(a, b) for a, b, _ in rule(unit) if b > a and a in inside and b - 1 in inside})
        out[name] = (len(spans), sum(covers(a, b) for a, b in spans), round(sum(by_chance(b - a) for a, b in spans), 1))
    return out


# ------------------------------------------------------------------ calibration on X1

def calibrate():
    truth = mi.x1_truth()['main']
    unit = LimitUnit(truth['image'])
    labels = dict(mi.labelled_gaps(unit, truth['owner']))
    middle = sorted(labels)[len(labels) // 2]
    rows = collections.OrderedDict()

    order = sorted(labels)
    total = [0]
    for g in order:
        total.append(total[-1] + labels[g])
    chance = {}

    def holds(length):
        """Share of the runs of `length` gaps that hold a limit."""
        if length not in chance:
            runs = [total[i + length] - total[i] for i in range(len(order) - length + 1)
                    if order[i + length - 1] - order[i] == length - 1]
            chance[length] = sum(v > 0 for v in runs) / len(runs) if runs else 1.0
        return chance[length]

    def judge(expected, wrong):
        if expected >= MIN_CHANCE and wrong == 0:
            return 'hard'
        return 'soft' if expected >= MIN_CHANCE and wrong <= SOFT_SHARE * expected else 'off'

    for name, rule in join_rules().items():
        cases = wrong = 0
        expected = 0.0
        halves = [[0, 0], [0, 0]]
        gaps = set()
        for first, last in sorted({(a, b) for a, b, _ in rule(unit) if b > a}):
            inside = [g for g in range(first, last)]
            if not inside or any(g not in labels for g in inside):
                continue
            bad = any(labels[g] for g in inside)
            cases += 1
            wrong += bad
            expected += holds(len(inside))
            half = halves[first >= middle]
            half[0] += 1
            half[1] += bad
            gaps.update(inside)
        rows[name] = dict(kind='join', cases=cases, wrong=wrong, chance=round(expected, 1), halves=halves, gaps=len(gaps),
                          gaps_wrong=sum(labels[g] for g in gaps), level=judge(expected, wrong))
    tally = collections.defaultdict(lambda: [0, 0, 0.0])
    for low, high, kind, _ in cut_intervals(unit):
        if all(g in labels for g in range(low, high + 1)):
            tally[kind][0] += 1
            tally[kind][1] += not any(labels[g] for g in range(low, high + 1))
            tally[kind][2] += 1 - holds(high - low + 1)
    for kind, (cases, wrong, expected) in sorted(tally.items()):
        # cut rules follow from how cc1 and ld work and hold few cases on X1: without an error they stay in use
        level = judge(expected, wrong)
        rows['cut ' + kind] = dict(kind='cut', cases=cases, wrong=wrong, chance=round(expected, 1),
                                   level='soft' if level == 'off' and wrong == 0 else level)
    # the tool's own recall: how many of the false cuts of X1 would the hard joins refute, and how many true
    # limits would a hard join deny
    used = [rule for name, rule in join_rules().items() if rows[name]['level'] == 'hard']
    joined = {g for rule in used for a, b, _ in rule(unit) for g in range(a, b)}
    true = [g for g, v in labels.items() if v]
    doc = dict(schema='tu-limits-model/1', unit='X1 main', original_sha256=truth['image'].sha256,
               gaps=len(labels), limits=len(true), joined_gaps=sum(g in joined for g in labels),
               joined_limits=sum(g in joined for g in true), min_chance=MIN_CHANCE, soft_share=SOFT_SHARE, rules=rows)
    MODEL.write_text(json.dumps(doc, indent=1) + '\n')
    return doc


# ------------------------------------------------------------------ audit of X3

def load_registry():
    doc = json.loads(REGISTRY.read_text())
    tus = sorted((t for t in doc['tus'] if t['in_scope'] and not t.get('alias_of') and t['functions']),
                 key=lambda t: int(t['functions'][0]['va'], 16))
    return tus


class Audit:
    def __init__(self):
        if not MODEL.is_file():
            raise SystemExit('tu_limits: run `tu_limits.py calibrate` first')
        self.model = json.loads(MODEL.read_text())
        self.tus = load_registry()
        self.names, table = {}, []
        for tu in self.tus:
            for f in tu['functions']:
                va = int(f['va'], 16)
                table.append((va, va + f['size']))
                self.names[va] = f['name']
        table.sort()
        self.image = mi.Image(ORIGINAL, 'main')
        self.unit = LimitUnit(self.image, table)
        self.owner = {self.unit.index[int(f['va'], 16)]: t for t in self.tus for f in t['functions']}
        # a gap counts when both sides are game code and no library zone lies between
        self.scope = [g for g in range(len(self.unit.vas) - 1) if g in self.owner and g + 1 in self.owner]
        inside = set(self.scope)
        self.cuts = [g for g in self.scope if self.owner[g] is not self.owner[g + 1]]
        self.control = control_rules(self.unit, self.scope, join_rules())
        self.level = {}
        for name in join_rules():
            level = self.model['rules'][name]['level']
            _, wrong, expected = self.control[name]
            if level != 'off' and wrong:
                level = 'soft' if wrong <= SOFT_SHARE * expected else 'off'
            self.level[name] = level
        self.joins = {level: collections.defaultdict(list) for level in ('hard', 'soft')}
        self.spans = []
        for name, rule in join_rules().items():
            level = self.level[name]
            if level == 'off':
                continue
            for first, last, note in {(a, b, n) for a, b, n in rule(self.unit) if b > a}:
                if any(g not in inside for g in (first, last - 1)):
                    continue
                self.spans.append((first, last, level, name, note))
                for g in range(first, last):
                    if g in inside:
                        self.joins[level][g].append((last - first, name, note, first, last))
        for table in self.joins.values():
            for rows in table.values():
                rows.sort()
        self.hard = set(self.joins['hard'])
        self.intervals = []
        for low, high, kind, note in list(cut_intervals(self.unit)) + homolog_cuts(self.unit):
            record = self.model['rules'].get('cut ' + kind)
            if not all(g in inside for g in range(low, high + 1)):
                continue
            free = [g for g in range(low, high + 1) if g not in self.hard]
            self.intervals.append(dict(low=low, high=high, rule=kind, note=note, free=free,
                                       level=record['level'] if record else 'soft'))
        self.usable = [i for i in self.intervals if i['level'] in ('hard', 'soft')]
        self.score = self.log_odds()

    def log_odds(self):
        model = json.loads((ROOT / mi.MODEL_FILE).read_text())
        return mi.score(model['model'], self.unit.features())

    def name(self, i):
        return self.names.get(self.unit.vas[i], f'func_{self.unit.vas[i]:08X}')

    def block(self, g):
        """First and last function of the maximal run around gap g whose gaps are all joined."""
        first = last = g
        while first - 1 in self.hard:
            first -= 1
        while last + 1 in self.hard:
            last += 1
        return first, last + 1

    def verdict(self, g):
        exact = [i for i in self.usable if i['free'] == [g]]
        cuts = set(self.cuts)
        # a wide range with several cuts in it says nothing about any one of them
        among = [i for i in self.usable if g in i['free'] and sum(x in cuts for x in i['free']) == 1]
        if g in self.hard:
            return 'refuted', among
        if exact:
            return 'proved', exact
        if among:
            return 'needed', among
        if g in self.joins['soft']:
            return 'doubtful', []
        return 'open', []

    def slack(self, g):
        """Other places this cut could take: free gaps next to g, up to the neighbour cuts."""
        cuts = set(self.cuts)
        left = right = 0
        x = g - 1
        while x in self.owner and x not in self.hard and x not in cuts and x + 1 in self.owner:
            left += 1
            x -= 1
        x = g + 1
        while x + 1 in self.owner and x not in self.hard and x not in cuts:
            right += 1
            x += 1
        return left, right

    def anchored(self, tu):
        for f in tu['functions']:
            i = self.unit.index[int(f['va'], 16)]
            for _, target, kind, _ in self.unit.refs[i]:
                if self.image.section_of(target) in ('.lit4', '.rodata'):
                    return True
        return False

    def merge_groups(self):
        """Runs of current TUs whose shared cuts are all refuted."""
        groups, run = [], []
        for g in self.cuts:
            if g in self.hard:
                if run and run[-1] is self.owner[g]:
                    run.append(self.owner[g + 1])
                else:
                    run = [self.owner[g], self.owner[g + 1]]
                    groups.append(run)
        return groups

    def advice(self, g):
        """For a refuted cut: where the block that denies it could let the cut go instead."""
        first, last = self.block(g)
        left, right = self.owner[g], self.owner[g + 1]
        left_first = self.unit.index[int(left['functions'][0]['va'], 16)]
        right_last = self.unit.index[int(right['functions'][-1]['va'], 16)]
        options = []
        if first > left_first:
            options.append(('move up', first, g - first + 1, self.score[first - 1]))
        if last < right_last:
            options.append(('move down', last + 1, last - g, self.score[last]))
        needed = any(left_first <= x < right_last and x not in self.hard for i in self.usable
                     for x in i['free'] if left_first <= min(i['free']) and max(i['free']) < right_last)
        if not needed:
            options.append(('merge', None, None, None))
        return options

    def missing(self):
        cuts = set(self.cuts)
        rows = []
        for i in self.usable:
            if not i['free']:
                rows.append(dict(i, state='conflict'))
            elif not any(g in cuts for g in i['free']):
                rows.append(dict(i, state='missing', best=max(i['free'], key=lambda g: self.score[g])))
        return rows

    def bounds(self):
        """Fewest and most TUs the evidence allows over the game code."""
        chosen = -1
        least = 0
        for i in sorted((i for i in self.usable if i['free']), key=lambda i: max(i['free'])):
            if chosen < min(i['free']):
                chosen = max(i['free'])
                least += 1
        zones = sum(1 for i in self.owner if i - 1 not in self.owner)
        return least + zones, len([g for g in self.scope if g not in self.hard]) + zones


def span_text(audit, first, last):
    return f'`{audit.name(first)}` .. `{audit.name(last)}`'


def render(a):
    verdicts = {g: a.verdict(g) for g in a.cuts}
    count = collections.Counter(v for v, _ in verdicts.values())
    least, most = a.bounds()
    m = a.model
    out = ['# X3 TU limits', '', 'Generated by `tools/tu/tu_limits.py report`. Do not edit by hand.', '',
           'Every current cut of main, judged by the original data only. Analysis, not proof: each rule has the error',
           'count that it showed on X1 main, where symbols give the true limits. Overlays are not covered: their code and data',
           'share one section, so these rules do not apply.', '',
           f'Registry `{REGISTRY.relative_to(ROOT)}` sha256 `{hashlib.sha256(REGISTRY.read_bytes()).hexdigest()[:16]}`, '
           f'original `{a.image.sha256[:16]}`.', '', '## Summary', '',
           f'- {len(a.tus)} TUs, {len(a.owner)} functions, {len(a.cuts)} cuts between game functions, {len(a.scope)} gaps.',
           f'- The evidence allows from {least} to {most} TUs: it settles {len(a.scope) - (most - 1)} gaps as no limit.', '',
           '| Verdict | Cuts | Meaning |', '| --- | ---: | --- |']
    for key, text in (('proved', 'a cut rule leaves this gap alone'),
                      ('needed', 'the only current cut in a range that must hold one; the place may move inside it'),
                      ('open', 'no rule decides'), ('doubtful', 'a soft join rule speaks against it'),
                      ('refuted', 'a hard join rule puts both sides in one object')):
        out.append(f'| {key} | {count[key]} | {text} |')
    blind = [t for t in a.tus if not a.anchored(t)]
    multi = []
    if CARVES.is_file():
        for tu, sections in json.loads(CARVES.read_text())['tus'].items():
            for section, runs in sections.items():
                if isinstance(runs, list) and len(runs) > 1 and tu.startswith('main/'):
                    multi.append((tu, section, len(runs)))
    out += ['', f'- TUs with no reference to `.lit4` or `.rodata`: {len(blind)} '
            f'({sum(len(t["functions"]) for t in blind)} functions). Their limits rest on the other sections or on code.',
            f'- TUs of main whose C emits a section in more than one run: {len(multi)}'
            + (' (' + ', '.join(f'{t} {s} x{n}' for t, s, n in multi) + ').' if multi else '.')
            + ' Until those runs are contiguous, the gate does not test their order.']

    out += ['', '## How far to trust the rules', '',
            f'X1 main: {m["gaps"]} gaps, {m["limits"]} true limits. The hard joins cover {m["joined_gaps"]} gaps and '
            f'{m["joined_limits"]} true limits. Thresholds were chosen on the same unit; the halves show the spread.', '',
            '| Rule | X1 | In use | X1 cases | Wrong | By chance | Half 1 | Half 2 | X3 cases | Wrong | By chance |', '| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |']
    for name, r in m['rules'].items():
        halves = ['%d/%d' % (w, c) for c, w in r['halves']] if 'halves' in r else ['', '']
        x3 = ' | '.join(str(v) for v in a.control[name]) if name in a.control else ' |  | '
        out.append(f'| {name} | {r["level"]} | {a.level.get(name, r["level"])} | {r["cases"]} | {r["wrong"]} | {r["chance"]} | '
                   f'{halves[0]} | {halves[1]} | {x3} |')

    out += ['', '## Refuted cuts', '', 'The shortest evidence is shown. `move up`/`move down` name the first function after the'
            ' moved cut and how many functions change TU; `merge` appears when no rule needs a limit in the two TUs.', '',
            '| Cut | Left TU | Right TU | Evidence | Options |', '| --- | --- | --- | --- | --- |']
    for g in a.cuts:
        if verdicts[g][0] != 'refuted':
            continue
        _, name, note, first, last = a.joins['hard'][g][0]
        options = '; '.join(f'{k} to `{a.name(i)}` ({n} functions)' if i is not None else k
                            for k, i, n, _ in a.advice(g))
        out.append(f'| `{a.unit.vas[g + 1]:#x}` `{a.name(g + 1)}` | {a.owner[g]["name"]} | {a.owner[g + 1]["name"]} | '
                   f'{note}: {span_text(a, first, last)} ({name}; {len(a.joins["hard"][g])} in all) | {options} |')

    groups = a.merge_groups()
    out += ['', '## Merge groups', '', 'Consecutive TUs joined through refuted cuts. A long chain can also come from one badly placed cut.', '']
    out += [f'- {" + ".join(t["name"] for t in run)} ({sum(len(t["functions"]) for t in run)} functions)' for run in groups] or ['- none']

    out += ['', '## Missing cuts', '', 'A cut rule needs a limit here and no current cut is in the allowed gaps. The suggested place is the'
            ' allowed gap that the map model scores highest: a lead only.', '',
            '| Range | TUs | Rule | Evidence | Allowed gaps | Suggested |', '| --- | --- | --- | --- | ---: | --- |']
    for i in a.missing():
        tus = list(dict.fromkeys(a.owner[x]['name'] for x in (i['low'], i['high'] + 1)))
        where = f'`{a.unit.vas[i["best"] + 1]:#x}` `{a.name(i["best"] + 1)}`' if i['state'] == 'missing' else 'rules conflict'
        out.append(f'| {span_text(a, i["low"], i["high"] + 1)} | {", ".join(tus)} | {i["rule"]} ({i["level"]}) | {i["note"]} | '
                   f'{len(i["free"])} | {where} |')

    for title, key in (('Proved cuts', 'proved'), ('Needed cuts', 'needed')):
        out += ['', f'## {title}', '', '| Cut | Left TU | Right TU | Rule | Evidence | Allowed gaps |', '| --- | --- | --- | --- | --- | ---: |']
        for g in a.cuts:
            if verdicts[g][0] == key:
                i = min(verdicts[g][1], key=lambda i: len(i['free']))
                out.append(f'| `{a.unit.vas[g + 1]:#x}` `{a.name(g + 1)}` | {a.owner[g]["name"]} | {a.owner[g + 1]["name"]} | '
                           f'{i["rule"]} ({i["level"]}) | {i["note"]} | {len(i["free"])} |')

    out += ['', '## Open and doubtful cuts', '', 'Slack: how many gaps before and after are equally free, up to the neighbour cuts. '
            'Score: log-odds of the map model at the gap, and the best score in the slack.', '',
            '| Cut | Left TU | Right TU | Verdict | Slack | Score | Best in slack |', '| --- | --- | --- | --- | --- | ---: | --- |']
    for g in a.cuts:
        if verdicts[g][0] in ('open', 'doubtful'):
            left, right = a.slack(g)
            best = max(range(g - left, g + right + 1), key=lambda x: a.score[x])
            text = 'here' if best == g else f'`{a.unit.vas[best + 1]:#x}` {a.score[best]:.2f}'
            out.append(f'| `{a.unit.vas[g + 1]:#x}` `{a.name(g + 1)}` | {a.owner[g]["name"]} | {a.owner[g + 1]["name"]} | '
                       f'{verdicts[g][0]} | -{left}/+{right} | {a.score[g]:.2f} | {text} |')
    return '\n'.join(out) + '\n', verdicts, count, (least, most), groups


def command_report(args):
    a = Audit()
    text, verdicts, count, bounds, groups = render(a)
    out = Path(args.out) if args.out else REPORT
    out.write_text(text)
    doc = dict(schema='tu-limits/1', registry_sha256=hashlib.sha256(REGISTRY.read_bytes()).hexdigest(),
               original_sha256=a.image.sha256, verdicts=dict(count), tu_bounds=list(bounds),
               merge_groups=[[t['id'] for t in run] for run in groups],
               missing=[dict(low=hex(a.unit.vas[i['low']]), high=hex(a.unit.vas[i['high'] + 1]), rule=i['rule'],
                             level=i['level'], note=i['note'], state=i['state'],
                             free=[hex(a.unit.vas[g + 1]) for g in i['free']]) for i in a.missing()],
               cuts=[dict(va=hex(a.unit.vas[g + 1]), left=a.owner[g]['id'], right=a.owner[g + 1]['id'],
                          verdict=verdicts[g][0], score=round(a.score[g], 3), slack=list(a.slack(g)),
                          joins=[dict(rule=n, note=t, first=hex(a.unit.vas[x]), last=hex(a.unit.vas[y]))
                                 for level in ('hard', 'soft') for _, n, t, x, y in a.joins[level].get(g, [])[:5]],
                          cut_rules=[dict(rule=i['rule'], note=i['note'], free=len(i['free'])) for i in verdicts[g][1]])
                     for g in a.cuts])
    out.with_suffix('.json').write_text(json.dumps(doc, indent=1) + '\n')
    print(json.dumps(dict(report=str(out), tus=len(a.tus), cuts=len(a.cuts), verdicts=dict(count),
                          tu_bounds=list(bounds), merge_groups=len(groups), missing=len(doc['missing']))))


def command_show(args):
    a = Audit()
    va = int(args.va, 16)
    i = bisect.bisect_right(a.unit.vas, va) - 1
    g = i - 1
    print(f'{a.unit.vas[i]:#x} {a.name(i)}: gap after {a.name(g)}')
    if g not in a.owner or i not in a.owner:
        raise SystemExit('outside the game code')
    print('current cut' if g in a.cuts else 'no current cut', '| TUs', a.owner[g]['name'], a.owner[i]['name'])
    print('verdict', a.verdict(g)[0], '| score %.2f' % a.score[g])
    for level in ('hard', 'soft'):
        for _, name, note, first, last in a.joins[level].get(g, [])[:8]:
            print(f'  {level} join: {note}: {a.name(first)} .. {a.name(last)} [{name}]')
    for row in a.intervals:
        if row['low'] <= g <= row['high']:
            print(f'  cut rule {row["rule"]} ({row["level"]}): {row["note"]}, {len(row["free"])} allowed gaps')


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest='command', required=True)
    sub.add_parser('calibrate', help='measure the rules on X1 and write the model')
    p = sub.add_parser('report', help='audit the current cuts of X3 main')
    p.add_argument('--out', help='Markdown path (the JSON goes beside it)')
    p = sub.add_parser('show', help='the evidence at one function start')
    p.add_argument('va')
    args = parser.parse_args(argv)
    if args.command == 'calibrate':
        doc = calibrate()
        levels = collections.Counter(r['level'] for r in doc['rules'].values())
        print(json.dumps(dict(model=str(MODEL), levels=dict(levels), gaps=doc['gaps'], limits=doc['limits'],
                              joined_gaps=doc['joined_gaps'], joined_limits=doc['joined_limits'])))
    elif args.command == 'report':
        command_report(args)
    else:
        command_show(args)


if __name__ == '__main__':
    main()
