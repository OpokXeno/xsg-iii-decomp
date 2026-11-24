#!/usr/bin/env python3
"""Best translation-unit map of X3 main from every kind of evidence, with a measured confidence per limit.

    tu_map.py calibrate     train on X1 (symbols give the truth), measure by cross-validation, write the model
    tu_map.py solve         write reports/x3-tu-map-solved.md and .json
    tu_map.py show VA       what decides the place before one function

A gap is the place between two consecutive functions. Four independent sources are combined:

  1. scored signals of the original binary (calls, data, strings, pools, delay-slot annulment): a boosted-tree and
     logistic ensemble trained on X1 main, whose symbols name the objects;
  2. hard rules of tu_limits.py: data that one object must cover, and ranges that must hold a limit;
  3. the sibling games: X1's real objects through equal or similar functions, X2's data through the same;
  4. tu_compile_evidence.py: C that gives the original bytes only with a callee inside (or outside) its file.

The map is analysis. It edits nothing: registry, sources and cuts stay as they are.
"""
import argparse
import bisect
import collections
import csv
import json
import math
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import map_infer as mi  # noqa: E402
import tu_limits as tl  # noqa: E402
from x3_homologs import X2_ROOT  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
MODEL = ROOT / 'config/tu-map-solved-model.json'
COMPILE_EVIDENCE = ROOT / 'config/tu-compile-evidence.json'
REPORT = ROOT / 'reports/x3-tu-map-solved.md'
HOMOLOGS = ROOT / 'annotations/leads'
FOLDS = 6
SEEDS = 4
HOMOLOG_SPAN = 12        # functions between two neighbours with homologs
LONGEST_SPAN = 128       # functions: a longer span comes from a stray reference
RADIUS = 1               # scored cuts keep this many gaps apart
GAME_SECTIONS = ('.rodata', '.sdata', '.sbss', '.bss', '.data', '.lit4')


# ------------------------------------------------------------------ signals

CALLER_SAVED = {2, 3, *range(4, 16), 24, 25}
ALU_OPS = {8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x18, 0x19}
ALU_FUNCTS = {0, 2, 3, 4, 6, 7, 0x21, 0x23, 0x24, 0x25, 0x26, 0x27, 0x2a, 0x2b, 0x2d, 0x2f, 0x38, 0x3a, 0x3b, 0x3c, 0x3e, 0x3f}


def reads_writes(word):
    op, rs, rt, rd, fn = word >> 26, (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31, word & 63
    if op == 0:
        if fn in (0, 2, 3, 0x38, 0x3a, 0x3b, 0x3c, 0x3e, 0x3f):
            return {rt}, {rd}
        if fn == 8:
            return {rs}, set()
        if fn == 9:
            return {rs}, {rd}
        if fn in (0x10, 0x12):
            return set(), {rd}
        return {rs, rt}, {rd}
    if op == 0x0f:
        return set(), {rt}
    if op in ALU_OPS or op in mi.LOAD_GPR:
        return {rs}, {rt}
    if op in (6, 7, 0x16, 0x17, 1):
        return {rs}, set()
    if op == 0x1c:
        return {rs, rt}, {rd}
    if op == 0x11:
        return (set(), {rt}) if rs in (0, 2) else ({rt}, set()) if rs in (4, 6) else (set(), set())
    if op == 0x12:
        return {rt}, {rt}
    return {rs, rt}, set()


def annulment_sites(unit):
    """(caller, callee va, annulled): a delay slot taken from the branch target, before a call on the other path.

    cc1's reorg annuls when that call may throw. A callee that the same file defined before is known not to.
    """
    image = unit.image
    for i, f in enumerate(unit.functions):
        for pc in range(f['va'], f['end'] - 8, 4):
            kind, _ = mi.branch_kind(image.word(pc), pc)
            if kind not in ('cond', 'likely'):
                continue
            slot = image.word(pc + 4)
            if slot == 0 or not (slot >> 26 in ALU_OPS or (slot >> 26 == 0 and slot & 63 in ALU_FUNCTS)):
                continue
            register = next(iter(reads_writes(slot)[1]))
            if register not in (2, 3):
                continue
            for q in range(pc + 8, f['end'] - 4, 4):
                word = image.word(q)
                if word >> 26 == 3:
                    reads, writes = reads_writes(image.word(q + 4))
                    if register not in reads and register not in writes:
                        yield i, ((word & 0x3ffffff) << 2) | (q & 0xf0000000), kind == 'likely'
                    break
                if mi.branch_kind(word, q)[0] in ('cond', 'likely', 'always', 'jr', 'j', 'jr_ra') or \
                        (word >> 26 == 0 and word & 63 in (8, 9)):
                    break
                reads, writes = reads_writes(word)
                if register in reads or register in writes:
                    break


def signals(unit):
    """name -> one value per gap, beyond map_infer's features."""
    import numpy
    n = len(unit.vas) - 1
    out = collections.defaultdict(lambda: numpy.zeros(n))

    def span(name, a, b, weight=1.0):
        if b > a:
            out[name][a:b] += weight
    for caller, targets in enumerate(unit.calls):
        for target in set(targets):
            callee = unit.index.get(target)
            if callee is None or callee == caller or abs(callee - caller) > 200:
                continue
            a, b = min(caller, callee), max(caller, callee)
            side = 'back' if callee < caller else 'fwd'
            span(f'call_{side}', a, b, 1 / (b - a))
            callers = len(unit.callers.get(callee, ()))
            if callers <= 3:
                span(f'call_{"only" if callers == 1 else "few"}_{side}', a, b, 1 / (b - a))
    for target, who in unit.users.items():
        a, b = min(who), max(who)
        if b > a:
            section = unit.image.section_of(target)
            if b - a <= 256:
                span('datum_' + (section.strip('.') if section in GAME_SECTIONS else 'other'), a, b, 1 / (b - a))
            users = sorted(who)
            for p, q in zip(users, users[1:]):
                if q - p <= 64:
                    span('datum_next', p, q, 1 / (q - p))
    for name, rule in tl.join_rules().items():
        if name.startswith('sandwich') and '.bss' not in name:
            continue
        for a, b, _ in set(rule(unit)):
            span('join ' + name, a, b)
            span('weight ' + name, a, b, 1 / max(1, b - a))
    for section in GAME_SECTIONS:
        items = tl.private_items(unit, section, 8)
        for (a, a1, a2), (b, b1, b2) in zip(items, items[1:]):
            low, high = min(a1, b1), max(a2, b2)
            if b - a > 512 or not 0 < high - low <= 64:
                continue
            same = a // 16 == b // 16
            span(('next16 ' if same else 'next ') + section, low, high, 1 / (high - low))
            if b1 < a2:
                span('next_back ' + section, low, high, 1 / (high - low))
            elif not same:
                span(('step16 ' if b % 16 == 0 else 'step ') + section, a2, b1, 1 / max(1, b1 - a2))
    for caller, target, annulled in annulment_sites(unit):
        callee = unit.index.get(target)
        if callee is not None and 0 < caller - callee <= 200:
            span('annulled' if annulled else 'not_annulled', callee, caller, 1 / (caller - callee))
    total = sum([out[k] for k in sorted(out) if k.startswith(('call_back', 'call_fwd', 'datum', 'next'))], numpy.zeros(n))
    out['total'] = total
    for radius in (1, 2, 4, 8):
        padded = numpy.pad(total, (radius, radius), mode='edge')
        window = numpy.stack([padded[i:i + n] for i in range(2 * radius + 1)])
        out[f'total_min_{radius}'] = total - window.min(0)
        out[f'total_rank_{radius}'] = (window < total).mean(0)
        out[f'total_mean_{radius}'] = total - window.mean(0)
    return out


def matrix(unit, names):
    import numpy
    extra = signals(unit)
    zero = numpy.zeros(len(unit.vas) - 1)
    return numpy.column_stack([numpy.array(unit.features())] + [extra.get(name, zero) for name in names])


# ------------------------------------------------------------------ model

class Boost:
    """Gradient-boosted trees for a binary label. Written out because no learning library is pinned."""

    def __init__(self, trees=300, rate=0.04, depth=4, rows=0.7, columns=0.2, ridge=5.0, least=3.0, bins=24, seed=0):
        self.__dict__.update({k: v for k, v in locals().items() if k != 'self'})

    def fit(self, x, y):
        import numpy
        rng = numpy.random.default_rng(self.seed)
        count, width = x.shape
        self.edges = [numpy.unique(numpy.quantile(x[:, j], numpy.linspace(0, 1, self.bins + 1)[1:-1])) for j in range(width)]
        binned = self._bin(x)
        mean = y.mean()
        self.base = float(math.log(mean / (1 - mean)))
        value = numpy.full(count, self.base)
        self.forest = []
        for _ in range(self.trees):
            p = 1 / (1 + numpy.exp(-value))
            pick = numpy.where(rng.random(count) < self.rows)[0]
            columns = rng.choice(width, max(1, int(width * self.columns)), replace=False)
            tree = []
            self._grow(binned, p - y, p * (1 - p), pick, columns, 0, tree)
            self.forest.append(tree)
            value += self.rate * self._apply(tree, binned)
        return self

    def _bin(self, x):
        import numpy
        return numpy.column_stack([numpy.searchsorted(self.edges[j], x[:, j], side='left') for j in range(x.shape[1])])

    def _grow(self, binned, g, h, pick, columns, depth, tree):
        import numpy
        me = len(tree)
        tree.append(None)
        total_g, total_h = g[pick].sum(), h[pick].sum()
        best = (1e-6, None, None)
        if depth < self.depth:
            here = total_g ** 2 / (total_h + self.ridge)
            for j in columns:
                b = binned[pick, j]
                left_g = numpy.cumsum(numpy.bincount(b, g[pick], self.bins + 1))[:-1]
                left_h = numpy.cumsum(numpy.bincount(b, h[pick], self.bins + 1))[:-1]
                allowed = (left_h >= self.least) & (total_h - left_h >= self.least)
                if allowed.any():
                    gain = numpy.where(allowed, left_g ** 2 / (left_h + self.ridge)
                                       + (total_g - left_g) ** 2 / (total_h - left_h + self.ridge) - here, -1)
                    k = int(gain.argmax())
                    if gain[k] > best[0]:
                        best = (gain[k], int(j), k)
        if best[1] is None:
            tree[me] = [round(float(-total_g / (total_h + self.ridge)), 5)]
            return me
        mask = binned[pick, best[1]] <= best[2]
        left = self._grow(binned, g, h, pick[mask], columns, depth + 1, tree)
        right = self._grow(binned, g, h, pick[~mask], columns, depth + 1, tree)
        tree[me] = [best[1], best[2], left, right]
        return me

    @staticmethod
    def _apply(tree, binned):
        import numpy
        out = numpy.zeros(len(binned))
        stack = [(0, numpy.arange(len(binned)))]
        while stack:
            node, rows = stack.pop()
            if len(tree[node]) == 1:
                out[rows] = tree[node][0]
            else:
                j, k, left, right = tree[node]
                mask = binned[rows, j] <= k
                stack += [(left, rows[mask]), (right, rows[~mask])]
        return out

    def predict(self, x):
        binned = self._bin(x)
        return self.base + self.rate * sum(self._apply(tree, binned) for tree in self.forest)

    def dump(self):
        return dict(base=self.base, rate=self.rate, edges=[e.tolist() for e in self.edges], forest=self.forest)

    @classmethod
    def load(cls, doc):
        import numpy
        model = cls(rate=doc['rate'])
        model.base, model.forest, model.edges = doc['base'], doc['forest'], [numpy.array(e) for e in doc['edges']]
        return model


def train_linear(x, y, ridge=1e-2, steps=600, rate=0.3):
    import numpy
    mean, deviation = x.mean(0), x.std(0)
    deviation[deviation == 0] = 1
    z = numpy.c_[numpy.clip((x - mean) / deviation, -6, 6), numpy.ones(len(x))]
    w = numpy.zeros(z.shape[1])
    w[-1] = -3
    for _ in range(steps):
        gradient = z.T @ (1 / (1 + numpy.exp(-z @ w)) - y) / len(y)
        gradient[:-1] += ridge * w[:-1]
        w -= rate * gradient
    return dict(mean=mean.tolist(), deviation=deviation.tolist(), weights=w.tolist())


def apply_linear(doc, x):
    import numpy
    z = numpy.clip((x - numpy.array(doc['mean'])) / numpy.array(doc['deviation']), -6, 6)
    return numpy.c_[z, numpy.ones(len(x))] @ numpy.array(doc['weights'])


def fit_ensemble(x, y):
    return [Boost(seed=s).fit(x, y) for s in range(SEEDS)], train_linear(x, y)


def ensemble_score(models, linear, x):
    return (sum(m.predict(x) for m in models) / len(models) + apply_linear(linear, x)) / 2


def platt(score, y):
    """Scale and shift that turn the score into a log-odds."""
    import numpy
    a, b = 1.0, 0.0
    for _ in range(4000):
        p = 1 / (1 + numpy.exp(-(a * score + b)))
        a -= 0.05 * float(((p - y) * score).mean())
        b -= 0.05 * float((p - y).mean())
    return a, b


def sigmoid(v):
    return 1 / (1 + math.exp(-max(-30.0, min(30.0, v))))


# ------------------------------------------------------------------ decoding

GRADES = ('proved', 'high', 'medium', 'low')


def decode(odds, scope, blocked, intervals, target):
    """Cuts as gap -> (grade, interval or None): scored gaps up to target, plus one for each uncovered interval."""
    cuts = {}
    for g in sorted((g for g in scope if g not in blocked), key=lambda g: -odds[g]):
        if len(cuts) >= target:
            break
        if all(abs(g - other) > RADIUS for other in cuts):
            cuts[g] = None
    for interval in sorted(intervals, key=lambda i: len(i['free'])):
        if interval['free'] and not any(g in cuts for g in interval['free']):
            cuts[max(interval['free'], key=lambda g: odds[g])] = None
    out = {}
    for g in cuts:
        p = sigmoid(odds[g])
        mine = [i for i in intervals if g in i['free']]
        alone = [i for i in mine if sum(x in cuts for x in i['free']) == 1]
        tight = min(alone, key=lambda i: len(i['free']), default=None)
        if tight and tight['level'] != 'weak':
            # the interval holds a limit: share its certainty among the free gaps by their odds
            weight = sum(math.exp(min(20, odds[x])) for x in tight['free'])
            p = max(p, math.exp(min(20, odds[g])) / weight)
        grade = 'proved' if tight and tight['level'] != 'weak' and len(tight['free']) == 1 else \
            'high' if p >= 0.65 else 'medium' if p >= 0.4 else 'low'
        out[g] = dict(grade=grade, p=p, interval=tight)
    return out


def hard_joins(unit, levels):
    joined = {}
    for name, rule in tl.join_rules().items():
        if levels[name] == 'hard':
            for a, b, note in sorted(set(rule(unit))):
                if b - a > LONGEST_SPAN:
                    continue
                for g in range(a, b):
                    joined.setdefault(g, ('data', note))
    return joined


def make_intervals(rows, scope, joined):
    inside, out = set(scope), []
    for low, high, kind, note in rows:
        if high >= low and all(g in inside for g in range(low, high + 1)):
            out.append(dict(low=low, high=high, rule=kind, note=note, level='strong',
                            free=[g for g in range(low, high + 1) if g not in joined]))
    return out


# ------------------------------------------------------------------ calibration

def calibrate():
    import numpy
    truth = mi.x1_truth()['main']
    unit = tl.LimitUnit(truth['image'])
    labels = dict(mi.labelled_gaps(unit, truth['owner']))
    names = sorted(signals(unit))
    gaps = numpy.array(sorted(labels))
    x, y = matrix(unit, names)[gaps], numpy.array([labels[g] for g in sorted(labels)])
    fold = numpy.minimum(numpy.arange(len(y)) * FOLDS // len(y), FOLDS - 1)
    held = numpy.zeros(len(y))
    for k in range(FOLDS):
        held[fold == k] = ensemble_score(*fit_ensemble(x[fold != k], y[fold != k]), x[fold == k])
    levels = {name: r['level'] for name, r in json.loads(tl.MODEL.read_text())['rules'].items()}
    joined = hard_joins(unit, levels)
    free = numpy.array([g not in joined for g in gaps])
    scale, shift = platt(held[free], y[free])
    odds = {int(g): scale * s + shift for g, s in zip(gaps, held)}
    expected = float(sum(sigmoid(v) for g, v in odds.items() if g not in joined))
    limits = {int(g) for g in gaps if labels[g]}
    intervals = make_intervals(tl.cut_intervals(unit), odds, joined)
    cuts = decode(odds, list(odds), joined, intervals, round(expected))
    grades = {}
    for grade in GRADES:
        mine = [g for g, c in cuts.items() if c['grade'] == grade]
        grades[grade] = dict(cuts=len(mine), exact=sum(g in limits for g in mine),
                             within_2=sum(any(g + d in limits for d in range(-2, 3)) for g in mine))
    bands = []
    for low, high in ((0, .02), (.02, .05), (.05, .1), (.1, .2), (.2, .4), (.4, .65), (.65, 1.01)):
        mine = [g for g, v in odds.items() if g not in joined and low <= sigmoid(v) < high]
        bands.append(dict(low=low, high=high, gaps=len(mine), limits=sum(g in limits for g in mine)))
    # whole TUs: both ends right
    edges = sorted(cuts)
    runs = [(a, b) for a, b in zip(edges, edges[1:]) if all(g in odds for g in range(a, b + 1))]
    exact_tus = sum(a in limits and b in limits and not any(g in limits for g in range(a + 1, b)) for a, b in runs)
    metrics = {f'within_{t}': mi.boundary_metrics(edges, limits, t) for t in (0, 1, 2)}
    old = json.loads((ROOT / mi.MODEL_FILE).read_text())['folds']['main']['exact']
    models, linear = fit_ensemble(x, y)
    doc = dict(schema='tu-map-solved-model/1', names=names, scale=scale, shift=shift, gaps=len(y), limits=len(limits),
               expected=round(expected, 1), metrics=metrics, previous_model=old, grades=grades, bands=bands,
               tus=dict(solved=len(runs), exact=exact_tus), joined=len([g for g in odds if g in joined]),
               joined_limits=sum(g in joined for g in limits),
               original_sha256=truth['image'].sha256, linear=linear, boost=[m.dump() for m in models])
    MODEL.write_text(json.dumps(doc, separators=(',', ':')) + '\n')
    return doc


# ------------------------------------------------------------------ the siblings

def homolog_rows(game):
    path = HOMOLOGS / f'{game}-homologs.tsv'
    if not path.is_file():
        return []
    with path.open() as stream:
        return [r for r in csv.DictReader(stream, delimiter='\t') if r['x3_unit'] == 'main' and r['src_unit'] == 'main'
                and r['cls'] != 'role' and int(r['x3_size']) >= tl.HOMOLOG_BYTES]


def x1_evidence(unit, owners):
    """Joins and cuts between neighbouring functions whose X1 twins sit in the same or in two real objects."""
    record = mi.x1_truth()['main']
    starts = sorted(record['owner'])

    def object_of(va):
        return record['owner'][starts[bisect.bisect_right(starts, va) - 1]][0]
    twin = {}
    for row in homolog_rows('x1'):
        i = unit.index.get(int(row['x3_va'], 16))
        if i in owners:
            twin[i] = (object_of(int(row['src_va'], 16)), row['src_name'], int(row['src_va'], 16), row['src_file'])
    joins, cuts = [], []
    order = sorted(twin)
    for a, b in zip(order, order[1:]):
        if b - a > HOMOLOG_SPAN:
            continue
        note = f'X1 {twin[a][1]}, {twin[b][1]}'
        if twin[a][0] == twin[b][0]:
            joins.append((a, b, 'same X1 object: ' + note))
        elif not any(twin[q][0] == twin[a][0] for q in order if b < q <= b + HOMOLOG_SPAN):
            # a lone moved function between two of one object states nothing
            if not any(twin[q][0] == twin[b][0] for q in order if a - HOMOLOG_SPAN <= q < a):
                cuts.append((a, b - 1, 'x1_object', 'two X1 objects: ' + note))
    return joins, cuts, twin


def x2_evidence(unit, owners, levels, model):
    """Joins that X2's data prove, and X2's own odds, carried over neighbouring functions of both games."""
    path = X2_ROOT / 'game/extracted-iso/SLUS_208.92'
    if not path.is_file():
        return [], []
    doc = json.loads((X2_ROOT / 'config/objects/main.objects.json').read_text())
    table = sorted((int(f['va'], 16), int(f['va'], 16) + f['size']) for t in doc['tus']
                   if t['in_scope'] and not t.get('alias_of') for f in t['functions'])
    other = tl.LimitUnit(mi.Image(path, 'main'), table)
    joined = hard_joins(other, levels)
    odds = scored_odds(other, model)
    twin = {}
    for row in homolog_rows('x2'):
        i, j = unit.index.get(int(row['x3_va'], 16)), other.index.get(int(row['src_va'], 16))
        if i in owners and j is not None:
            twin[i] = j
    joins, opinions = [], []
    order = sorted(twin)
    for a, b in zip(order, order[1:]):
        x, y = twin[a], twin[b]
        if b - a > 8 or not 0 < y - x <= HOMOLOG_SPAN:
            continue
        if all(g in joined for g in range(x, y)):
            joins.append((a, b, 'X2 data joins their twins'))
        else:
            none = 1.0
            for g in range(x, y):
                none *= 1 - (0 if g in joined else sigmoid(odds[g]))
            opinions.append((a, b, 1 - none))
    return joins, opinions


def scored_odds(unit, model):
    raw = ensemble_score([Boost.load(d) for d in model['boost']], model['linear'], matrix(unit, model['names']))
    return [model['scale'] * float(v) + model['shift'] for v in raw]


# ------------------------------------------------------------------ the solved map

class Solved:
    def __init__(self):
        self.model = json.loads(MODEL.read_text())
        self.audit = a = tl.Audit()
        unit, scope, inside = a.unit, a.scope, set(a.scope)
        self.odds = scored_odds(unit, self.model)
        x1_joins, x1_cuts, self.twin = x1_evidence(unit, a.owner)
        x2_joins, opinions = x2_evidence(unit, a.owner, a.level, self.model)
        compiled_joins, compiled_cuts = [], []
        if COMPILE_EVIDENCE.is_file():
            doc = json.loads(COMPILE_EVIDENCE.read_text())
            for callee, caller in doc['joins']:
                compiled_joins.append((unit.index[int(callee, 16)], unit.index[int(caller, 16)], 'C matches only with both in one file'))
            for callee, caller in doc['cuts']:
                i, j = unit.index[int(callee, 16)], unit.index[int(caller, 16)]
                compiled_cuts.append((i, j - 1, 'compiled', 'C matches only with the callee in another file'))
        cut_rows = [r for r in tl.cut_intervals(unit)] + x1_cuts + compiled_cuts
        # strongest first: a join that would leave a cut range without a free gap yields to the range
        self.joined = {}
        self.conflicts = []
        sources = [('data', [(i, i + 1, n[1]) for i, n in hard_joins(unit, a.level).items()]), ('compiled', compiled_joins),
                   ('x1', x1_joins), ('x2', x2_joins)]
        for source, rows in sources:
            for first, last, note in rows:
                if any(g not in inside for g in (first, last - 1)):
                    continue
                trial = {g for g in range(first, last) if g not in self.joined}
                bad = [] if source == 'data' else [
                    c for c in cut_rows if any(g in trial for g in range(c[0], c[1] + 1))
                    and all(g in self.joined or g in trial for g in range(c[0], c[1] + 1))]
                if bad:
                    self.conflicts.append((first, last, source, note, bad[0]))
                    continue
                for g in trial:
                    self.joined[g] = (source, note)
        for first, last, p in opinions:
            gaps = [g for g in range(first, last) if g not in self.joined and g in inside]
            if not gaps:
                continue
            none = 1.0
            for g in gaps:
                none *= 1 - sigmoid(self.odds[g])
            mine = min(max(1 - none, 1e-4), 1 - 1e-4)
            shift = 0.5 * (math.log(min(max(p, 1e-4), 1 - 1e-4) / (1 - min(max(p, 1e-4), 1 - 1e-4))) - math.log(mine / (1 - mine)))
            for g in gaps:
                self.odds[g] += shift
        self.intervals = make_intervals(cut_rows, scope, self.joined)
        free = [g for g in scope if g not in self.joined]
        self.expected = sum(sigmoid(self.odds[g]) for g in free)
        self.cuts = decode(self.odds, scope, self.joined, self.intervals, round(self.expected))
        self.tus = self.build()

    def build(self):
        a = self.audit
        edges = sorted(self.cuts)
        out, first = [], min(a.owner)
        breaks = sorted(set(edges) | {i for i in a.owner if i + 1 not in a.owner})
        for end in breaks:
            if end < first:
                continue
            members = list(range(first, end + 1))
            names = collections.Counter(a.owner[i]['name'] for i in members if i in a.owner)
            inner = [g for g in members[:-1] if g not in self.joined]
            worst = max(inner, key=lambda g: self.odds[g], default=None)
            out.append(dict(first=first, last=end, names=names, start=self.cuts.get(first - 1), end=self.cuts.get(end),
                            joined=len(members) - 1 - len(inner), worst=worst,
                            risk=sigmoid(self.odds[worst]) if worst is not None else 0.0))
            following = [i for i in a.owner if i > end]
            if not following:
                break
            first = min(following)
        for tu in out:
            tu['name'] = self.title(tu)
            tu['action'] = self.action(tu)
        return out

    def title(self, tu):
        files = self.audit.unit.source_files()
        paths = {next(iter(files[i])).decode('latin1').replace('\\', '/').rsplit('/', 1)[-1]
                 for i in range(tu['first'], tu['last'] + 1) if i in files and len(files[i]) == 1}
        if len(paths) == 1:
            return paths.pop().replace('.euc.c', '.c')
        objects = collections.Counter(self.twin[i][3].rsplit('/', 1)[-1] for i in range(tu['first'], tu['last'] + 1)
                                      if i in self.twin and self.twin[i][3])
        named = [n for n, _ in tu['names'].most_common() if not n.startswith('tu')]
        if named and tu['names'][named[0]] * 2 >= sum(tu['names'].values()):
            return named[0] + '.c'
        if objects and objects.most_common(1)[0][1] >= 2:
            return objects.most_common(1)[0][0] + ' (X1)'
        return (named[0] if named else tu['names'].most_common(1)[0][0]) + '.c'

    def action(self, tu):
        a, cuts = self.audit, set(self.audit.cuts)
        starts = tu['first'] - 1 in cuts or tu['first'] - 1 not in a.owner
        ends = tu['last'] in cuts or tu['last'] + 1 not in a.owner
        whole = len(tu['names'])
        if starts and ends:
            return 'same' if whole == 1 else 'merge'
        return 'split' if whole == 1 else 'recut'


def certainty(tu):
    good = all(e is None or e['grade'] in ('proved', 'high') for e in (tu['start'], tu['end']))
    if good and tu['risk'] < 0.2:
        return 'firm'
    if tu['risk'] < 0.4 and all(e is None or e['grade'] != 'low' for e in (tu['start'], tu['end'])):
        return 'likely'
    return 'tentative'


def why(s, g):
    cut = s.cuts.get(g)
    if cut is None:
        return 'edge of the game code'
    if cut['interval']:
        return f'{cut["interval"]["rule"]}: {cut["interval"]["note"]}'
    return 'scored signals'


def write_report(s):
    a, m = s.audit, s.model
    tu_grade = collections.Counter(certainty(t) for t in s.tus)
    functions = collections.Counter()
    for t in s.tus:
        functions[certainty(t)] += t['last'] - t['first'] + 1
    action = collections.Counter(t['action'] for t in s.tus)
    cut_grade = collections.Counter(c['grade'] for c in s.cuts.values())
    kept = sum(g in set(a.cuts) for g in s.cuts)
    sources = collections.Counter(v[0] for g, v in s.joined.items() if g in set(a.scope))
    vas = a.unit.vas
    out = ['# X3 main TU map, solved', '', 'Generated by `tools/tu/tu_map.py solve`. Do not edit by hand. Analysis: the registry keeps its own cuts.', '',
           f'{len(a.owner)} game functions. Current map: {len(a.tus)} TUs. Solved map: **{len(s.tus)} TUs**. '
           f'{len(s.joined)} of {len(a.scope)} gaps are proved to be no limit '
           f'({", ".join(f"{k} {v}" for k, v in sources.most_common())}).', '',
           '| TU certainty | TUs | Functions |', '| --- | ---: | ---: |']
    out += [f'| {k} | {tu_grade[k]} | {functions[k]} |' for k in ('firm', 'likely', 'tentative')]
    out += ['', '`firm`: both ends proved or high, and no place inside above 20%. `likely`: no low end, nothing inside above 40%.', '',
            '| Against the current map | TUs |', '| --- | ---: |']
    out += [f'| {k} | {action[k]} |' for k in ('same', 'merge', 'split', 'recut')]
    out += ['', f'Of {len(s.cuts)} solved limits, {kept} are current cuts; {len(a.cuts) - kept} current cuts disappear.', '',
            '## Measured on X1', '', f'Cross-validated on X1 main ({m["limits"]} true limits in {m["gaps"]} gaps; signals and data rules only, '
            'without the sibling games or the compiler test, which X1 cannot check). The data rules were tuned on X1, so this is a little optimistic.', '',
            '| | Cuts | Precision | Recall | F1 |', '| --- | ---: | ---: | ---: | ---: |']
    old = m['previous_model']
    out.append(f'| previous model, exact | {old["cuts"]} | {old["precision"]} | {old["recall"]} | {old["f1"]} |')
    for t, label in ((0, 'exact'), (1, 'within 1 function'), (2, 'within 2 functions')):
        r = m['metrics'][f'within_{t}']
        out.append(f'| this model, {label} | {r["cuts"]} | {r["precision"]} | {r["recall"]} | {r["f1"]} |')
    out += ['', f'Whole TUs exact at both ends: {m["tus"]["exact"]} of {m["tus"]["solved"]}.', '',
            '| Limit grade | X1 cuts | Exact | Within 2 | X3 cuts |', '| --- | ---: | ---: | ---: | ---: |']
    out += [f'| {k} | {v["cuts"]} | {v["exact"]} | {v["within_2"]} | {cut_grade[k]} |' for k, v in m['grades'].items()]
    out += ['', '## The map', '', 'Start: grade of the first limit and its probability. Inside: the likeliest missed limit, with its probability. '
            '`same`, `merge`, `split` and `recut` compare with the current TUs, listed with their function counts.', '',
            '| # | Range | Functions | Name | Certainty | Start | Why | Inside | Against current | Current TUs |',
            '| ---: | --- | ---: | --- | --- | --- | --- | --- | --- | --- |']
    for k, t in enumerate(s.tus, 1):
        start = '-' if t['start'] is None else f'{t["start"]["grade"]} {t["start"]["p"]:.2f}'
        inside = '-' if t['worst'] is None or t['risk'] < 0.1 else f'`{vas[t["worst"] + 1]:#x}` {t["risk"]:.2f}'
        names = ', '.join(f'{n} {c}' for n, c in t['names'].items())
        out.append(f'| {k} | `{vas[t["first"]]:#x}`-`{a.unit.functions[t["last"]]["end"]:#x}` | {t["last"] - t["first"] + 1} | {t["name"]} | '
                   f'{certainty(t)} | {start} | {why(s, t["first"] - 1)} | {inside} | {t["action"]} | {names} |')
    out += ['', '## Conflicts', '', 'Joins that lost against a range that must hold a limit. Each one is a wrong lead on one side.', '',
            '| Join | Source | Evidence | Against |', '| --- | --- | --- | --- |']
    for first, last, source, note, cut in s.conflicts:
        out.append(f'| `{a.name(first)}` .. `{a.name(last)}` | {source} | {note} | {cut[2]}: {cut[3]} |')
    for i in s.intervals:
        if not i['free']:
            out.append(f'| `{a.name(i["low"])}` .. `{a.name(i["high"] + 1)}` | data | covered by data rules | {i["rule"]}: {i["note"]} |')
    REPORT.write_text('\n'.join(out) + '\n')
    doc = dict(schema='tu-map-solved/1', original_sha256=a.image.sha256, tus=[
        dict(index=k, start=hex(vas[t['first']]), end=hex(a.unit.functions[t['last']]['end']), functions=t['last'] - t['first'] + 1,
             name=t['name'], certainty=certainty(t), start_grade=t['start'] and t['start']['grade'],
             start_probability=t['start'] and round(t['start']['p'], 3), why=why(s, t['first'] - 1), risk=round(t['risk'], 3),
             risk_at=hex(vas[t['worst'] + 1]) if t['worst'] is not None else None, action=t['action'], current=dict(t['names']))
        for k, t in enumerate(s.tus, 1)])
    REPORT.with_suffix('.json').write_text(json.dumps(doc, indent=1) + '\n')
    return dict(report=str(REPORT.relative_to(ROOT)), tus=len(s.tus), certainty=dict(tu_grade), functions=dict(functions), action=dict(action),
                cuts=dict(cut_grade), current_cuts_kept=kept, joined=dict(sources), conflicts=len(s.conflicts), expected=round(s.expected))


def command_show(va):
    s = Solved()
    unit = s.audit.unit
    i = bisect.bisect_right(unit.vas, va) - 1
    print(f'{unit.vas[i]:#x} {s.audit.name(i)}: gap after {s.audit.name(i - 1)}')
    print('solved limit' if i - 1 in s.cuts else 'no limit', s.cuts.get(i - 1, {}).get('grade', ''), '| probability %.3f' % sigmoid(s.odds[i - 1]),
          '| current cut' if i - 1 in s.audit.cuts else '| no current cut')
    if i - 1 in s.joined:
        print('joined by', *s.joined[i - 1])
    for row in s.intervals:
        if row['low'] <= i - 1 <= row['high'] and len(row['free']) <= 40:
            print(f'cut range {row["rule"]}: {row["note"]}, {len(row["free"])} free gaps')


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest='command', required=True)
    sub.add_parser('calibrate', help='train and measure on X1, write the model')
    sub.add_parser('solve', help='write the solved map of X3 main')
    p = sub.add_parser('show', help='what decides the place before one function')
    p.add_argument('va')
    args = parser.parse_args(argv)
    if args.command == 'calibrate':
        doc = calibrate()
        print(json.dumps({k: doc[k] for k in ('gaps', 'limits', 'expected', 'metrics', 'previous_model', 'grades', 'bands', 'tus', 'joined',
                                              'joined_limits')}))
    elif args.command == 'solve':
        print(json.dumps(write_report(Solved())))
    else:
        command_show(int(args.va, 16))


if __name__ == '__main__':
    main()
