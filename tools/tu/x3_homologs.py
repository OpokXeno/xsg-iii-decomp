#!/usr/bin/env python3
"""Which X3 game functions have a homolog in Xenosaga I or II? Leads only, never identity.

    python3 -B tools/tu/x3_homologs.py        # both sources: annotations/leads/x{1,2}-homologs.{tsv,json},
                                              # reports/x3-x{1,2}-homologs.md and reports/x3-homologs.md

tools/tu/x3_annotate.py only finds copies: bodies equal once relocated fields are masked, unique
on both sides, of 8 words or more. This tool looks further, and says for each pair how it was found:

  exact      the body is equal once relocated fields are masked (j/jal targets, lui immediates,
             $gp-relative immediates, the low half after a lui)
  offsets    equal once every immediate of an arithmetic, load or store instruction is masked too:
             the same instructions and registers, with other constants, struct offsets or frame sizes
  opcodes    the same sequence of operations, with other registers as well
  similar    the operation sequences are alike (ratio in the row), and context puts the pair together
  role       context puts the pair together but the code is not alike: the same place in the
             program, rewritten or too short to compare. An assumption, the weakest class

A key that is unique in both games pairs two functions by itself (8 words or more for `exact`,
12 for `offsets`, 20 for `opcodes`). Everything else needs context, and each row lists it:

  calls      two paired functions call these two at the same call site. In an equal shape the call
             sites correspond one to one; in a `similar` pair the two call lists are aligned around
             the callees that are already paired
  callees    both call the same set of already paired functions (two or more), weighted so that a
             callee everything calls counts little; each is the other's best by a margin
  callers    the same, for the set of already paired functions that call them
  string     both reference a text that only one function of each game references
  order      both sit between the same two paired neighbours, in link order, and are alike
  shape      no context: among all unpaired functions of 24 operations or more, each is the other's
             most alike by a clear margin (ratio 0.85 or more, 0.10 above the runner-up on both sides)
  gap        after everything else: two paired neighbours have the same small number of unpaired
             functions between them in both games, taken in order. Proximity alone
  calls      after everything else: call sites with no rival on either side, whatever the code
             (in the rounds, one call site pairs two functions only when their code is alike)

Context only proposes a pair; the code decides its class. Rounds repeat until nothing is added; the
last pass runs once at the end and never serves as an anchor. The report measures
both rules on pairs that were made without them. Rows made by the last pass carry `late`.

Library functions (SDK, runtime, middleware) take part as anchors only: they pair by equal shapes
alone, they give `calls`, `callees` and `callers` more to hold on to, and no table counts them.

X1 has a symbol table and real object files; X2's names and files are recovered labels and its
function table is an estimate. Both trees are only read.
"""
import bisect
import collections
import difflib
import json
import math
import sys
from pathlib import Path

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))
import map_infer  # noqa: E402
from x3_annotate import masked  # noqa: E402

X1_ROOT = Path('/home/pc/xenosaga1-port/xenosaga-i-decomp')
X2_ROOT = Path('/home/pc/xenosaga2-port/xenosga-ii-decomp')
X1_FILES = dict(main='SLUS_204.69', ov01='OV01.OVL', ov02='OV02.OVL', ov10='OV10.OVL', ov11='OV11.OVL', ov12='OV12.OVL')
X2_FILES = dict(main='SLUS_208.92', ov01='OV01.OVL', ov02='OV02.OVL')
X2_GAME = ((0x194638, 0x29f000),)                       # X2 tree, tools/tu/map_infer.py MAIN_ZONES
X3_FILES = dict(main='SLUS_213.89', ov01='OV01.OVL', ov02='OV02.OVL', ov04='OV04.OVL')
X3_GAME = tuple((z['start'], z['end']) for z in map_infer.MAIN_ZONES if z['cls'] == 'game')
UNION_TSV = 'annotations/leads/homologs.tsv'
UNION_REPORT = 'reports/x3-homologs.md'
SIGNATURE_SCORE = 0.6           # weighted overlap of two sets of paired callees or callers
SIGNATURE_MARGIN = 0.15
SIGNATURE_SHARED = 2
SIGNATURE_USERS = 200           # a function this many others call or are called by is no signature
CONTEXT_GAP = 2                 # unpaired functions between two anchors that proximity alone may pair (3: 18 of 24 right)
UNIQUE_KEY_WORDS = dict(exact=8, offsets=12, opcodes=20)
SIMILAR = 0.6                   # operation-sequence ratio below which a context pair is only a `role`
ORDER_SIMILAR = 0.7             # an order pair has no other evidence, so it needs more
MAX_GAP = 60                    # functions between two anchors that order alignment will look at
SHAPE_WORDS = 24                # shorter functions are alike to too many others to pair without context
SHAPE_RATIO = 0.85
SHAPE_MARGIN = 0.10
SHINGLE = 6                     # operations per fingerprint
SHINGLE_USERS = 6               # a fingerprint more functions share says nothing
SHAPE_CANDIDATES = 6
STRING_BYTES = 6
CLASSES = ('exact', 'offsets', 'opcodes', 'similar', 'role')
BRANCH_OPS = {1, 4, 5, 6, 7, 0x14, 0x15, 0x16, 0x17}
IMMEDIATE_OPS = ({0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x18, 0x19, 0x1a, 0x1b, 0x1e, 0x1f, 0x2f}
                 | set(range(0x20, 0x2f)) | {0x31, 0x35, 0x36, 0x37, 0x39, 0x3d, 0x3e, 0x3f})


def offsets_key(words):
    """masked(), with the immediate of every arithmetic, load and store instruction set to zero."""
    return tuple(word & 0xffff0000 if word >> 26 in IMMEDIATE_OPS else word for word in masked(words))


def operation(word):
    """What the instruction does, without registers, immediates or targets."""
    op = word >> 26
    if op == 0:
        return 0x100 | (word & 0x3f)
    if op == 1:
        return 0x200 | ((word >> 16) & 31)
    if op in (0x10, 0x11, 0x12):
        rs = (word >> 21) & 31
        return (op << 12) | (rs << 6) | ((word & 0x3f) if rs >= 16 else 0)
    if op == 0x1c:
        return 0x400000 | ((word & 0x3f) << 5) | ((word >> 6) & 31 if word & 0x3f in (8, 9, 0x28, 0x29) else 0)
    return op


class Function:
    __slots__ = ('game', 'unit', 'va', 'size', 'name', 'words', 'exact', 'offsets', 'opcodes', 'calls',
                 'strings', 'index', 'recovered', 'file', 'library')

    @property
    def key(self):
        return (self.unit, self.va)


def estimated_rows(root, zones):
    """unit -> [(name, va, size, library)] from a stripped game's estimated table; outside the game zones is library."""
    table = json.loads((root / 'config/symbols/estimated-functions.json').read_text())['units']

    def rows(unit, image):
        index = image.code_section['index']
        return [(name, va, size, not (image.overlay or any(low <= va < high for low, high in zones)))
                for name, va, size, section in table[unit]['functions'] if section == index]
    return rows


def manifest_rows(root):
    """unit -> [(name, va, size, library)] from a build manifest (X1: real symbols); out of scope is library."""
    by_unit = collections.defaultdict(list)
    for tu in json.loads((root / 'config/tu-build.json').read_text())['tus']:
        by_unit[tu['unit']] += [(f['name'], int(f['va'], 16), f['size'], not tu.get('in_scope')) for f in tu['functions']]
    return lambda unit, image: [row for row in by_unit[unit] if image.text_start <= row[1] < image.limit_va]


def load_ledger(root):
    """(unit, va) -> (category, source path) from a project's per-TU ledgers."""
    out = {}
    for path in sorted((root / 'config/tu').glob('*/tu*.json')):
        record = json.loads(path.read_text())
        for function in record.get('functions', []):
            out[record['unit'], int(function['va'], 16)] = (function.get('category'), record.get('path'))
    return out


def load_game(game, root, files, rows_of, ledger=None):
    functions = []
    images = {unit: map_infer.Image(root / 'game/extracted-iso' / name, unit) for unit, name in files.items()}
    overlay_base = min(image.text_start for image in images.values() if image.overlay)
    for unit, image in images.items():
        for name, va, size, library in sorted(rows_of(unit, image), key=lambda row: row[1]):
            function = Function()
            function.game, function.unit, function.va, function.size, function.name = game, unit, va, size, name
            function.library = library
            words = [image.word(at) for at in range(va, va + size - size % 4, 4)]
            while words and words[-1] == 0:
                words.pop()
            function.words = words
            function.exact = masked(words)
            function.offsets = offsets_key(words)
            function.opcodes = tuple(operation(word) for word in words)
            calls = []
            for at in range(va, va + 4 * len(words), 4):
                word = image.word(at)
                if word >> 26 == 3:
                    target = ((at + 4) & 0xf0000000) | ((word & 0x03ffffff) << 2)
                    in_overlay = target >= overlay_base
                    # A call from main into the overlay address range names no unit: several load there.
                    calls.append(None if in_overlay and not image.overlay else
                                 (unit if in_overlay else 'main', target))
            function.calls = calls
            record = dict(va=va, end=va + 4 * len(words))
            texts = set()
            for _, target, _, _ in map_infer.scan_refs(image, record):
                if image.section_of(target) is None:
                    continue
                text = map_infer.string_at(image, target)
                if text is not None and len(text) >= STRING_BYTES:
                    texts.add(text)
            function.strings = texts
            function.recovered, function.file = (ledger or {}).get((unit, va), (None, None))
            functions.append(function)
    for index, function in enumerate(functions):
        function.index = index
    return functions


class Matcher:
    def __init__(self, x2, x3):
        self.x2, self.x3 = x2, x3
        self.by2 = {f.key: f for f in x2}
        self.by3 = {f.key: f for f in x3}
        self.game2 = [f for f in x2 if not f.library]
        self.game3 = [f for f in x3 if not f.library]
        self.pairs = {}                 # x2 key -> dict(x3=key, cls, ratio, evidence)
        self.taken = {}                 # x3 key -> x2 key
        self.ratios = {}
        self.log = collections.Counter()

    def ratio(self, a, b):
        key = (a.key, b.key)
        if key not in self.ratios:
            short, long = sorted((len(a.opcodes), len(b.opcodes)))
            if not short or short / long < 0.4:
                value = 0.0
            else:
                matcher = difflib.SequenceMatcher(None, a.opcodes, b.opcodes, autojunk=False)
                value = matcher.ratio() if matcher.quick_ratio() >= 0.3 else matcher.quick_ratio()
            self.ratios[key] = value
        return self.ratios[key]

    def code_class(self, a, b):
        if a.exact == b.exact:
            return 'exact', 1.0
        if a.offsets == b.offsets:
            return 'offsets', 1.0
        if a.opcodes == b.opcodes:
            return 'opcodes', 1.0
        value = self.ratio(a, b)
        return ('similar' if value >= SIMILAR else 'role'), value

    def add(self, a, b, evidence, minimum=None):
        if a.key in self.pairs or b.key in self.taken:
            return False
        cls, value = self.code_class(a, b)
        if a.library != b.library or (a.library and cls not in ('exact', 'offsets', 'opcodes')):
            return False                                    # library functions are anchors: equal shapes only
        if minimum is not None and CLASSES.index(cls) > CLASSES.index(minimum[0]):
            return False
        if minimum is not None and cls == minimum[0] and value < minimum[1]:
            return False
        self.pairs[a.key] = dict(x3=b.key, cls=cls, ratio=round(value, 3), evidence=[evidence])
        self.taken[b.key] = a.key
        if not a.library:
            self.log[f'{evidence.split(":")[0]} -> {cls}'] += 1
        return True

    def note(self, a, b, evidence):
        pair = self.pairs.get(a.key)
        if pair and pair['x3'] == b.key and evidence not in pair['evidence']:
            pair['evidence'].append(evidence)

    # ------------------------------------------------------------------ rounds

    def unique_keys(self):
        added = 0
        for cls, words in UNIQUE_KEY_WORDS.items():
            left, right = collections.defaultdict(list), collections.defaultdict(list)
            for f in self.x2:
                if f.key not in self.pairs and len(f.words) >= words:
                    left[getattr(f, cls)].append(f)
            for f in self.x3:
                if f.key not in self.taken and len(f.words) >= words:
                    right[getattr(f, cls)].append(f)
            for key, mine in left.items():
                theirs = right.get(key)
                if theirs and len(mine) == 1 and len(theirs) == 1:
                    added += self.add(mine[0], theirs[0], 'unique-key')
        return added

    def shapes(self):
        """Pairs by code alone: mutual most-alike functions, each ahead of its runner-up by a margin."""
        left = [f for f in self.game2 if f.key not in self.pairs and len(f.opcodes) >= SHAPE_WORDS]
        right = [f for f in self.game3 if f.key not in self.taken and len(f.opcodes) >= SHAPE_WORDS]

        def shingles(f):
            return {f.opcodes[i:i + SHINGLE] for i in range(len(f.opcodes) - SHINGLE + 1)}

        def candidates(sources, targets):
            index = collections.defaultdict(list)
            for position, f in enumerate(targets):
                for shingle in shingles(f):
                    index[shingle].append(position)
            out = {}
            for f in sources:
                shared = collections.Counter()
                for shingle in shingles(f):
                    users = index.get(shingle, ())
                    if len(users) <= SHINGLE_USERS:
                        shared.update(users)
                out[f.key] = [targets[position] for position, _ in shared.most_common(SHAPE_CANDIDATES)]
            return out

        def clear_best(f, options, swap):
            scored = sorted(((self.ratio(o, f) if swap else self.ratio(f, o)), o.key) for o in options)
            if not scored or scored[-1][0] < SHAPE_RATIO:
                return None
            if len(scored) > 1 and scored[-1][0] - scored[-2][0] < SHAPE_MARGIN:
                return None
            return scored[-1][1]

        forward, backward = candidates(left, right), candidates(right, left)
        added = 0
        for f in left:
            k3 = clear_best(f, forward[f.key], False)
            if k3 is None:
                continue
            g = self.by3[k3]
            if clear_best(g, backward[k3], True) == f.key:
                added += self.add(f, g, 'shape', ('similar', SHAPE_RATIO))
        return added

    def accept_votes(self, votes, label, minimum=None):
        """Mutual best candidates of a vote table {(x2 key, x3 key): count}."""
        best2, best3 = collections.defaultdict(list), collections.defaultdict(list)
        for (k2, k3), count in votes.items():
            best2[k2].append((count, k3))
            best3[k3].append((count, k2))
        added = 0
        for k2, options in best2.items():
            options.sort(reverse=True)
            count, k3 = options[0]
            if len(options) > 1 and options[1][0] * 2 > count:
                continue                                    # a rival with more than half the votes
            back = sorted(best3[k3], reverse=True)
            if back[0][1] != k2 or (len(back) > 1 and back[1][0] * 2 > count):
                continue
            a, b = self.by2[k2], self.by3[k3]
            if k2 in self.pairs or k3 in self.taken:
                self.note(a, b, f'{label}:{count}')
                continue
            added += self.add(a, b, f'{label}:{count}', minimum)
        return added

    def calls(self):
        votes = self.call_votes()
        # One call site is thin evidence for a pair whose code is not alike.
        strong = {pair: count for pair, count in votes.items() if count >= 2}
        return self.accept_votes(votes, 'calls', ('similar', SIMILAR)) + self.accept_votes(strong, 'calls')

    def strings(self):
        users2, users3 = collections.defaultdict(set), collections.defaultdict(set)
        for f in self.x2:
            for text in f.strings:
                users2[text].add(f.key)
        for f in self.x3:
            for text in f.strings:
                users3[text].add(f.key)
        votes = collections.Counter()
        for text, who in users2.items():
            other = users3.get(text)
            if other and len(who) == 1 and len(other) == 1:
                votes[next(iter(who)), next(iter(other))] += 1
        self.string_votes = votes
        strong = {pair: count for pair, count in votes.items() if count >= 2}
        return self.accept_votes(votes, 'string', ('similar', SIMILAR)) + self.accept_votes(strong, 'string')

    def order(self):
        """Unpaired functions between two paired neighbours, aligned in link order."""
        added = 0
        units2 = collections.defaultdict(list)
        for f in self.x2:
            units2[f.unit].append(f)
        index3 = {f.key: f.index for f in self.x3}
        for unit, functions in units2.items():
            anchors = [(f.index, index3[self.pairs[f.key]['x3']]) for f in functions
                       if f.key in self.pairs and self.pairs[f.key]['cls'] != 'role']
            for (a2, a3), (b2, b3) in zip(anchors, anchors[1:]):
                if self.x3[a3].unit != self.x3[b3].unit or not a3 < b3:
                    continue
                gap2 = [f for f in self.x2[a2 + 1:b2] if f.key not in self.pairs]
                gap3 = [f for f in self.x3[a3 + 1:b3] if f.key not in self.taken]
                if not gap2 or not gap3 or len(gap2) > MAX_GAP or len(gap3) > MAX_GAP:
                    continue
                added += self.align(gap2, gap3)
        return added

    def align(self, gap2, gap3):
        def score(a, b):
            cls, value = self.code_class(a, b)
            if cls in ('exact', 'offsets', 'opcodes'):
                return 1.0 + 0.01 * (3 - CLASSES.index(cls))
            # A short function is alike to many; only an equal shape pairs it by position.
            return value if value >= ORDER_SIMILAR and min(len(a.words), len(b.words)) >= 8 else None
        rows, columns = len(gap2), len(gap3)
        best = [[0.0] * (columns + 1) for _ in range(rows + 1)]
        for i in range(rows - 1, -1, -1):
            for j in range(columns - 1, -1, -1):
                value = score(gap2[i], gap3[j])
                take = best[i + 1][j + 1] + value if value is not None else -1.0
                best[i][j] = max(best[i + 1][j], best[i][j + 1], take)
        added, i, j = 0, 0, 0
        while i < rows and j < columns:
            value = score(gap2[i], gap3[j])
            if value is not None and best[i][j] == best[i + 1][j + 1] + value:
                added += self.add(gap2[i], gap3[j], 'order')
                i, j = i + 1, j + 1
            elif best[i][j] == best[i + 1][j]:
                i += 1
            else:
                j += 1
        return added

    def anchor_runs(self):
        """Consecutive paired neighbours of one X2 unit whose X3 sides sit in one unit, in order."""
        units = collections.defaultdict(list)
        for f in self.x2:
            units[f.unit].append(f)
        index3 = {f.key: f.index for f in self.x3}
        for functions in units.values():
            anchors = [(f.index, index3[self.pairs[f.key]['x3']]) for f in functions
                       if f.key in self.pairs and self.pairs[f.key]['cls'] != 'role']
            for (a2, a3), (b2, b3) in zip(anchors, anchors[1:]):
                if self.x3[a3].unit == self.x3[b3].unit and a3 < b3:
                    yield a2, a3, b2, b3

    def call_votes(self, kind='all'):
        """{(x2 callee, x3 callee): call sites} from paired callers. `kind`: 'shape', 'aligned' or 'all'."""
        votes = collections.Counter()
        for k2, pair in self.pairs.items():
            a, b = self.by2[k2], self.by3[pair['x3']]
            if pair['cls'] in ('exact', 'offsets', 'opcodes'):
                if kind == 'aligned' or len(a.calls) != len(b.calls):
                    continue
                for c2, c3 in zip(a.calls, b.calls):        # equal shapes: the call sites correspond
                    if c2 in self.by2 and c3 in self.by3:
                        votes[c2, c3] += 1
            elif pair['cls'] == 'similar' and kind != 'shape':
                # Alike bodies: align the two call lists around the callees that are already paired.
                left = [('known', self.pairs[c]['x3']) if c in self.pairs and self.pairs[c]['cls'] != 'role'
                        else ('unit',) if c is None else ('open',) for c in a.calls]
                right = [('known', c) if c in self.taken and self.pairs[self.taken[c]]['cls'] != 'role'
                         else ('unit',) if c is None else ('open',) for c in b.calls]
                blocks = difflib.SequenceMatcher(None, left, right, autojunk=False).get_matching_blocks()
                for i, j, size in blocks:
                    if not any(left[i + k][0] == 'known' for k in range(size)):
                        continue                            # nothing known holds this run in place
                    for k in range(size):
                        c2, c3 = a.calls[i + k], b.calls[j + k]
                        if left[i + k] == ('open',) and c2 in self.by2 and c3 in self.by3:
                            votes[c2, c3] += 1
        return votes

    def signature_scores(self, direction, everyone=False):
        """(score, shared, x2 key, x3 key) for functions with alike sets of paired callees or callers.

        `everyone` scores paired functions too, to measure the rule on pairs made without it.
        """
        def neighbours(functions, by):
            out = collections.defaultdict(set)
            for f in functions:
                for c in f.calls:
                    if c in by:
                        if direction == 'callees':
                            out[f.key].add(c)
                        else:
                            out[c].add(f.key)
            return out
        near2, near3 = neighbours(self.x2, self.by2), neighbours(self.x3, self.by3)
        solid2 = {k for k, pair in self.pairs.items() if pair['cls'] != 'role'}
        solid3 = {self.pairs[k]['x3'] for k in solid2}
        users = collections.defaultdict(list)               # paired x3 function -> x3 functions that have it near
        for k3, near in near3.items():
            for c in near & solid3:
                users[c].append(k3)
        weight = {c: 1.0 / math.log2(2 + len(who)) for c, who in users.items()}
        out = []
        for f in self.game2:
            if f.key in self.pairs and not everyone:
                continue
            mapped = {self.pairs[c]['x3'] for c in near2.get(f.key, set()) & solid2}
            if len(mapped) < SIGNATURE_SHARED:
                continue
            candidates = collections.Counter()
            for c in mapped:
                if len(users.get(c, ())) <= SIGNATURE_USERS:
                    candidates.update(users.get(c, ()))
            mine = sum(weight.get(c, 1.0) for c in mapped)
            for k3, shared in candidates.items():
                g = self.by3[k3]
                if shared < SIGNATURE_SHARED or g.library or (k3 in self.taken and not everyone):
                    continue
                theirs = near3[k3] & solid3
                both = sum(weight[c] for c in mapped & theirs)
                out.append((both / (mine + sum(weight[c] for c in theirs) - both), shared, f.key, k3))
        return out

    @staticmethod
    def clear_signatures(scores):
        """Mutual best pairs of signature_scores(), each ahead of its runner-up on both sides."""
        best2, best3 = collections.defaultdict(list), collections.defaultdict(list)
        for score, shared, k2, k3 in scores:
            best2[k2].append((score, k3, shared))
            best3[k3].append((score, k2))
        out = []
        for k2, options in best2.items():
            options.sort(reverse=True)
            score, k3, shared = options[0]
            if score < SIGNATURE_SCORE or (len(options) > 1 and score - options[1][0] < SIGNATURE_MARGIN):
                continue
            back = sorted(best3[k3], reverse=True)
            if back[0][1] != k2 or (len(back) > 1 and back[0][0] - back[1][0] < SIGNATURE_MARGIN):
                continue
            out.append((k2, k3, shared))
        return out

    def signatures(self, minimum=('similar', SIMILAR)):
        added = 0
        for direction in ('callees', 'callers'):
            for k2, k3, shared in self.clear_signatures(self.signature_scores(direction)):
                a, b = self.by2[k2], self.by3[k3]
                if k2 in self.pairs or k3 in self.taken:
                    self.note(a, b, f'{direction}:{shared}')
                    continue
                added += self.add(a, b, f'{direction}:{shared}', minimum)
        return added

    def measure_context(self):
        """How often would proximity or one call site alone have found a pair that other evidence made?"""
        gap = collections.Counter()
        runs = list(self.anchor_runs())
        for (a2, a3, m2, m3), (n2, n3, b2, b3) in zip(runs, runs[1:]):
            if (m2, m3) != (n2, n3):
                continue                                    # the two runs do not share their middle anchor
            left2 = [f.index for f in self.x2[a2 + 1:b2] if f.key not in self.pairs or f.index == m2]
            left3 = [f.index for f in self.x3[a3 + 1:b3] if f.key not in self.taken or f.index == m3]
            others = len(left2) - 1
            if len(left2) != len(left3) or not 1 <= others <= 8:
                continue                                    # hiding the middle anchor leaves `others` + 1 to place
            gap[others, left3[left2.index(m2)] == m3] += 1
        aligned = collections.Counter()
        for k2, pair in self.pairs.items():
            if pair['cls'] != 'similar':
                continue
            a, b = self.by2[k2], self.by3[pair['x3']]
            left = [('known', self.pairs[c]['x3']) if c in self.pairs and self.pairs[c]['cls'] != 'role'
                    else ('unit',) if c is None else ('open',) for c in a.calls]
            right = [('known', c) if c in self.taken and self.pairs[self.taken[c]]['cls'] != 'role'
                     else ('unit',) if c is None else ('open',) for c in b.calls]
            for hidden in {token for token in left if token[0] == 'known' and not self.by3[token[1]].library}:
                hide = lambda tokens: [('open',) if token == hidden else token for token in tokens]
                one, two = hide(left), hide(right)
                found = False
                for i, j, size in difflib.SequenceMatcher(None, one, two, autojunk=False).get_matching_blocks():
                    if not any(one[i + k][0] == 'known' for k in range(size)):
                        continue
                    for k in range(size):
                        if left[i + k] == hidden:
                            aligned[b.calls[j + k] == hidden[1]] += 1
                            found = True
                aligned['not aligned'] += not found
        calls = [dict(callers='callers that are alike, call lists aligned (one paired callee hidden at a time)',
                      votes='each call site', correct=aligned[True], wrong=aligned[False])]
        for kind, label in (('shape', 'callers of equal shape'),):
            single = collections.Counter()
            best2, best3 = collections.defaultdict(list), collections.defaultdict(list)
            for (k2, k3), count in self.call_votes(kind).items():
                best2[k2].append((count, k3))
                best3[k3].append((count, k2))
            for k2, options in best2.items():
                pair = self.pairs.get(k2)
                if pair is None or self.by2[k2].library or pair['evidence'][0].startswith('calls'):
                    continue
                if len(options) == 1 and len(best3[options[0][1]]) == 1:
                    single['one site' if options[0][0] == 1 else 'two sites or more', pair['x3'] == options[0][1]] += 1
            calls += [dict(callers=label, votes=name, correct=single[name, True], wrong=single[name, False])
                      for name in ('one site', 'two sites or more')]
        signatures = []
        for direction in ('callees', 'callers'):
            tally = collections.Counter()
            for k2, k3, shared in self.clear_signatures(self.signature_scores(direction, everyone=True)):
                pair = self.pairs.get(k2)
                if pair is None or pair['cls'] == 'role' or pair['evidence'][0].startswith(('callees', 'callers')):
                    continue
                tally[pair['x3'] == k3] += 1
            signatures.append(dict(rule=direction, correct=tally[True], wrong=tally[False]))
        self.context_measure = dict(
            gap=[dict(unpaired_between=n, correct=gap[n, True], wrong=gap[n, False]) for n in range(1, 9)],
            calls=calls, signatures=signatures)

    def context(self):
        """Proximity and single call sites. One pass, after the rounds; its pairs never anchor others."""
        added = collections.Counter()
        for a2, a3, b2, b3 in list(self.anchor_runs()):
            gap2 = [f for f in self.x2[a2 + 1:b2] if f.key not in self.pairs]
            gap3 = [f for f in self.x3[a3 + 1:b3] if f.key not in self.taken]
            if len(gap2) == len(gap3) and 1 <= len(gap2) <= CONTEXT_GAP:
                for a, b in zip(gap2, gap3):
                    added['gap'] += self.add(a, b, f'gap:{len(gap2)}')
        added['calls'] += self.accept_votes(dict(self.call_votes()), 'calls')
        added['signatures'] += self.signatures(minimum=None)
        return dict(added)

    def run(self):
        rounds = []
        while True:
            counts = dict(unique_keys=self.unique_keys(), shapes=self.shapes(), calls=self.calls(), strings=self.strings(),
                          signatures=self.signatures(), order=self.order())
            rounds.append(counts)
            if not any(counts.values()) or len(rounds) > 30:
                break
        self.before_context = set(self.pairs)
        self.measure_context()
        self.context_added = self.context()
        return rounds


# --------------------------------------------------------------------- checks

def cross_checks(matcher, x3_names):
    """Evidence a pair did not use, compared with the pair."""
    out = {}
    agree, made = collections.Counter(), collections.Counter()
    for (k2, k3), count in matcher.string_votes.items():
        pair = matcher.pairs.get(k2)
        if pair is None or matcher.by2[k2].library or pair['evidence'][0].startswith('string'):
            continue                                        # the string made the pair: not independent
        agree[pair['cls'], pair['x3'] == k3] += 1
        made[pair['evidence'][0].split(':')[0], pair['x3'] == k3] += 1
    out['unique_string_agrees'] = {cls: dict(agree=agree[cls, True], disagree=agree[cls, False]) for cls in CLASSES}
    out['unique_string_agrees_by_evidence'] = {kind: dict(agree=made[kind, True], disagree=made[kind, False])
                                               for kind in sorted({kind for kind, _ in made})}
    names = collections.Counter()
    for k2, pair in matcher.pairs.items():
        if matcher.by2[k2].library:
            continue
        mine = x3_names.get(pair['x3'])
        if mine:
            names[pair['cls'], mine == matcher.by2[k2].name] += 1
    out['x1_name_of_the_x3_function_agrees'] = {cls: dict(agree=names[cls, True], disagree=names[cls, False]) for cls in CLASSES}
    order = {}
    by_units = collections.defaultdict(list)
    for k2, pair in matcher.pairs.items():
        if pair['cls'] != 'role' and not matcher.by2[k2].library:
            by_units[k2[0], pair['x3'][0]].append((k2[1], pair['x3'][1]))
    for units, rows in sorted(by_units.items()):
        rows.sort()
        tails = []
        for _, va in rows:                                   # longest increasing run of X3 addresses
            position = bisect.bisect_left(tails, va)
            tails[position:position + 1] = [va]
        order[f'{units[0]} -> {units[1]}'] = dict(pairs=len(rows), in_one_increasing_order=len(tails))
    out['link_order'] = order
    out['context_rules_on_known_pairs'] = matcher.context_measure
    return out


LEFT_BUCKETS = ((0.95, '0.95 or more'), (0.85, '0.85 to 0.95'), (0.7, '0.70 to 0.85'), (0.5, '0.50 to 0.70'), (0.0, 'below 0.50'))


def left_over(matcher):
    """For each source function left unpaired: how alike is the most alike X3 function, paired or not?"""
    index = collections.defaultdict(list)
    for position, f in enumerate(matcher.game3):
        for i in range(len(f.opcodes) - SHINGLE + 1):
            index[f.opcodes[i:i + SHINGLE]].append(position)
    counts, sizes = collections.Counter(), collections.Counter()
    short = short_bytes = 0
    for f in matcher.game2:
        if f.key in matcher.pairs:
            continue
        if len(f.opcodes) < SHAPE_WORDS:
            short += 1
            short_bytes += 4 * len(f.words)
            continue
        shared = collections.Counter()
        for i in range(len(f.opcodes) - SHINGLE + 1):
            users = index.get(f.opcodes[i:i + SHINGLE], ())
            if len(users) <= 30:
                shared.update(users)
        best = max((matcher.ratio(f, matcher.game3[position]) for position, _ in shared.most_common(8)), default=0.0)
        label = next(name for low, name in LEFT_BUCKETS if best >= low)
        counts[label] += 1
        sizes[label] += 4 * len(f.words)
    unpaired3 = [f for f in matcher.game3 if f.key not in matcher.taken]
    return dict(source_unpaired=sum(1 for f in matcher.game2 if f.key not in matcher.pairs), source_short=short,
                source_short_bytes=short_bytes,
                best_ratio=[dict(ratio=name, functions=counts[name], source_bytes=sizes[name]) for _, name in LEFT_BUCKETS],
                x3_unpaired=len(unpaired3), x3_unpaired_bytes=sum(4 * len(f.words) for f in unpaired3),
                x3_unpaired_short=sum(1 for f in unpaired3 if len(f.opcodes) < SHAPE_WORDS))


def x1_names():
    """X3 (unit, va) -> name, for the annotations that came from X1's symbols."""
    out = {}
    for unit, _, path in (('main', None, 'annotations/slus_213.89_annotations.csv'),) + tuple(map_infer.OVERLAYS):
        file = ROOT / path
        if not file.is_file():
            continue
        for line in file.read_text().splitlines():
            if 'same code as X1' in line:
                va, rest = line.split(',', 1)
                out[unit, int(va, 16)] = rest.split(' - ')[0].strip()
    return out


# --------------------------------------------------------------------- output

def context_only(row):
    """Made by the last pass: proximity, or call sites the code did not back."""
    return row['late']


def write_outputs(source, matcher, rounds, checks, left):
    x2, x3 = matcher.game2, matcher.game3
    rows = []
    for k2, pair in matcher.pairs.items():
        a, b = matcher.by2[k2], matcher.by3[pair['x3']]
        if a.library:
            continue
        rows.append(dict(x3_unit=b.unit, x3_va=f'0x{b.va:08x}', x3_size=4 * len(b.words), x3_name=b.name,
                         source=source, src_unit=a.unit, src_va=f'0x{a.va:08x}', src_size=4 * len(a.words), src_name=a.name,
                         src_file=a.file, src_state=a.recovered, cls=pair['cls'], ratio=pair['ratio'],
                         evidence=pair['evidence'], late=k2 not in matcher.before_context))
    rows.sort(key=lambda row: (row['x3_unit'], row['x3_va']))
    tsv = ROOT / f'annotations/leads/{source.lower()}-homologs.tsv'
    tsv.parent.mkdir(parents=True, exist_ok=True)
    columns = ('x3_unit', 'x3_va', 'x3_size', 'x3_name', 'cls', 'ratio', 'source', 'src_unit', 'src_va', 'src_size',
               'src_name', 'src_state', 'src_file', 'evidence')
    lines = ['\t'.join(columns)]
    for row in rows:
        lines.append('\t'.join(str(';'.join(row[c]) if c == 'evidence' else row[c]) for c in columns))
    tsv.write_text('\n'.join(lines) + '\n')
    bytes3 = sum(4 * len(f.words) for f in x3)
    summary = collections.OrderedDict()
    for cls in CLASSES:
        mine = [row for row in rows if row['cls'] == cls]
        late = [row for row in mine if row['late']]
        summary[cls] = dict(pairs=len(mine), x3_bytes=sum(row['x3_size'] for row in mine),
                            source_is_c=sum(1 for row in mine if row['src_state'] == 'exact_c'),
                            source_named=sum(1 for row in mine if not row['src_name'].startswith('func_')),
                            by_proximity_or_one_call=len(late), by_proximity_or_one_call_bytes=sum(row['x3_size'] for row in late))
    by_unit = collections.OrderedDict()
    for unit in X3_FILES:
        total = [f for f in x3 if f.unit == unit]
        mine = [row for row in rows if row['x3_unit'] == unit]
        by_unit[unit] = dict(functions=len(total), bytes=sum(4 * len(f.words) for f in total),
                             by_class={cls: sum(1 for row in mine if row['cls'] == cls) for cls in CLASSES},
                             bytes_by_class={cls: sum(row['x3_size'] for row in mine if row['cls'] == cls) for cls in CLASSES},
                             from_source_unit=dict(collections.Counter(row['src_unit'] for row in mine if row['cls'] != 'role')))
    similar = []
    for low, high, label in ((0.9, 2.0, '0.90 or more'), (0.8, 0.9, '0.80 to 0.90'), (0.7, 0.8, '0.70 to 0.80'), (0.0, 0.7, '0.60 to 0.70')):
        mine = [row for row in rows if row['cls'] == 'similar' and low <= row['ratio'] < high]
        similar.append(dict(ratio=label, pairs=len(mine), x3_bytes=sum(row['x3_size'] for row in mine),
                            by_evidence=dict(collections.Counter(row['evidence'][0].split(':')[0] for row in mine))))
    report = dict(source=source, source_game_functions=len(x2), x3_game_functions=len(x3),
                  source_game_bytes=sum(4 * len(f.words) for f in x2), x3_game_bytes=bytes3, classes=summary,
                  by_x3_unit=by_unit, rounds=rounds, context_pass=matcher.context_added,
                  library_anchors=sum(1 for k2 in matcher.pairs if matcher.by2[k2].library),
                  how_pairs_were_made=dict(sorted(matcher.log.items())), cross_checks=checks, left_over=left,
                  similar_by_ratio=similar)
    (ROOT / f'annotations/leads/{source.lower()}-homologs.json').write_text(json.dumps(dict(summary=report, pairs=rows), indent=1) + '\n')
    return report, rows


MEANING = dict(exact='the body, relocated fields masked', offsets='instructions and registers; constants or offsets differ',
               opcodes='the operation sequence; registers differ', similar='alike, and context agrees',
               role='context only; the code is not alike')
SOURCE_NOTE = dict(X1='X1 has a symbol table, so its names and files are real.',
                   X2='X2\'s names and files are recovered labels, and its function table is an estimate.')


def context_lines(measure):
    lines = ['The last pass pairs functions with no evidence in their own code: `gap` (the same small number of unpaired',
             'functions between two paired neighbours, taken in order), call sites whose two callees are not alike, and sets',
             'of paired callees or callers. Each rule was measured on pairs that other evidence had already made, by asking',
             'the rule about them:', '',
             '| Functions to place between two neighbours | Hidden pair placed right | Placed wrong |', '| ---: | ---: | ---: |']
    lines += [f'| {row["unpaired_between"] + 1} | {row["correct"]} | {row["wrong"]} |' for row in measure['gap']
              if row['correct'] + row['wrong']]
    lines += ['', f'`gap` is applied up to {CONTEXT_GAP} functions. A gap of one cannot be measured this way.', '',
              '| Call sites that name the pair, no rival | From | Agrees with the pair | Disagrees |', '| --- | --- | ---: | ---: |']
    lines += [f'| {row["votes"]} | {row["callers"]} | {row["correct"]} | {row["wrong"]} |' for row in measure['calls']]
    lines += ['', '| Set of paired neighbours, clear best on both sides | Agrees with the pair | Disagrees |', '| --- | ---: | ---: |']
    lines += [f'| {row["rule"]} | {row["correct"]} | {row["wrong"]} |' for row in measure['signatures']]
    lines += ['', 'These numbers are an upper bound for the last pass. The pairs they were measured on are functions whose code',
              'survived; the functions the last pass pairs are the ones whose code did not, and there a removed function and',
              'an added one look the same as one changed function.']
    return lines


def write_report(report, rows):
    source = report['source']
    total3, bytes3 = report['x3_game_functions'], report['x3_game_bytes']
    lines = [f'# {source} homologs of X3 functions', '',
             'Generated by `tools/tu/x3_homologs.py`. Do not edit by hand. The pairs are in',
             f'`annotations/leads/{source.lower()}-homologs.tsv` and `.json`; [x3-homologs.md](x3-homologs.md) joins both sources.', '',
             f'Every pair is a lead. {SOURCE_NOTE[source]} X3\'s function table is an estimate,',
             'and only the whole-file gate says that C reproduces X3\'s bytes.', '',
             '## Result', '',
             f'{source} has {report["source_game_functions"]:,} game functions and X3 {total3:,} ({bytes3:,} bytes of code).', '',
             f'| Class | What is equal | Pairs | Share of X3 functions | X3 bytes | Share of X3 code | {source} side is published C | '
             'Of the pairs, made by the last pass |',
             '| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |']
    running = running_bytes = 0
    for cls, row in report['classes'].items():
        running += row['pairs']
        running_bytes += row['x3_bytes']
        lines.append(f'| `{cls}` | {MEANING[cls]} | {row["pairs"]:,} | {100 * row["pairs"] / total3:.1f}% | {row["x3_bytes"]:,} | '
                     f'{100 * row["x3_bytes"] / bytes3:.1f}% | {row["source_is_c"]:,} | {row["by_proximity_or_one_call"]:,} |')
    lines += [f'| all | | {running:,} | {100 * running / total3:.1f}% | {running_bytes:,} | {100 * running_bytes / bytes3:.1f}% | | '
              f'{sum(row["by_proximity_or_one_call"] for row in report["classes"].values()):,} |', '',
              f'{total3 - running:,} X3 game functions have no {source} pair of any class.', '',
              f'| X3 unit | Functions | exact | offsets | opcodes | similar | role | Unpaired | Paired code, without `role` | From {source} units |',
              '| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |']
    for unit, row in report['by_x3_unit'].items():
        counts = row['by_class']
        paired = sum(row['bytes_by_class'][cls] for cls in CLASSES if cls != 'role')
        lines.append(f'| `{unit}` | {row["functions"]:,} | ' + ' | '.join(f'{counts[cls]:,}' for cls in CLASSES)
                     + f' | {row["functions"] - sum(counts.values()):,} | {paired:,} of {row["bytes"]:,} bytes ({100 * paired / max(1, row["bytes"]):.1f}%) | '
                     + ', '.join(f'{name} {count}' for name, count in sorted(row['from_source_unit'].items())) + ' |')
    lines += ['', '`similar` by how alike the two operation sequences are:', '',
              '| Ratio | Pairs | X3 bytes | Made by |', '| --- | ---: | ---: | --- |']
    lines += [f'| {row["ratio"]} | {row["pairs"]:,} | {row["x3_bytes"]:,} | '
              + ', '.join(f'{kind} {count}' for kind, count in sorted(row['by_evidence'].items())) + ' |'
              for row in report['similar_by_ratio']]
    lines += ['', '## Pairs by context alone', ''] + context_lines(report['cross_checks']['context_rules_on_known_pairs'])
    left = report['left_over']
    lines += ['', '## What is left', '',
              f'{left["source_unpaired"]:,} {source} functions have no pair. {left["source_short"]:,} of them ({left["source_short_bytes"]:,} bytes) are '
              f'shorter than {SHAPE_WORDS} operations, too short to judge by code alone. For the others, this is how alike the',
              'most alike X3 function is, paired or not:', '',
              f'| Best ratio against any X3 function | {source} functions | {source} bytes |', '| --- | ---: | ---: |']
    lines += [f'| {row["ratio"]} | {row["functions"]:,} | {row["source_bytes"]:,} |' for row in left['best_ratio']]
    lines += ['', 'The search looks at the 8 X3 functions that share the most 6-operation runs, so a function rewritten throughout',
              'reads as "below 0.50" whether or not it is the same function in the source.', '',
              f'{left["x3_unpaired"]:,} X3 game functions ({left["x3_unpaired_bytes"]:,} bytes) have no {source} pair; {left["x3_unpaired_short"]:,} of them '
              f'are shorter than {SHAPE_WORDS} operations.',
              '', '## How the pairs were made', '',
              '| Evidence that proposed the pair -> class the code gave it | Pairs |', '| --- | ---: |']
    lines += [f'| {key} | {count:,} |' for key, count in report['how_pairs_were_made'].items()]
    lines += ['', f'Rounds until nothing was added: {len(report["rounds"])}, then the last pass. {report["library_anchors"]:,} library functions '
              'were paired as anchors and are counted nowhere.', '',
              '## How far to trust it', '',
              'Nothing here was reviewed by hand, and there is no ground truth. These checks use evidence a pair did not use:', '',
              '**A text only one function of each game references.** Where a pair was made without it, does it agree?', '',
              '| Class | Agrees | Disagrees |', '| --- | ---: | ---: |']
    lines += [f'| `{cls}` | {row["agree"]} | {row["disagree"]} |' for cls, row in report['cross_checks']['unique_string_agrees'].items()]
    lines += ['', '| Made by | Agrees | Disagrees |', '| --- | ---: | ---: |']
    lines += [f'| {kind} | {row["agree"]} | {row["disagree"]} |'
              for kind, row in report['cross_checks']['unique_string_agrees_by_evidence'].items()]
    if source == 'X2':
        lines += ['', '**X1\'s symbol name.** Where `tools/tu/x3_annotate.py` named the X3 function from X1\'s symbol table, is it the',
                  'name X2 gave its side of the pair? X2 took many names from X1, so a disagreement can also be an X2 label.', '',
                  '| Class | Same name | Other name |', '| --- | ---: | ---: |']
        lines += [f'| `{cls}` | {row["agree"]} | {row["disagree"]} |'
                  for cls, row in report['cross_checks']['x1_name_of_the_x3_function_agrees'].items()]
    lines += ['', '**Link order.** Pairs of one source unit and one X3 unit, without `role`: how many fit one increasing order?',
              'Whole files moved between the games, so this is never all of them; the `order` evidence makes it higher than chance.', '',
              f'| {source} unit -> X3 unit | Pairs | In one increasing order |', '| --- | ---: | ---: |']
    lines += [f'| {units} | {row["pairs"]:,} | {row["in_one_increasing_order"]:,} |'
              for units, row in report['cross_checks']['link_order'].items()]
    files, file_bytes = collections.Counter(), collections.Counter()
    for row in rows:
        if row['cls'] != 'role' and row['src_file']:
            files[row['src_file']] += 1
            file_bytes[row['src_file']] += row['x3_size']
    lines += ['', f'## {source} files with the most homologs', '', f'| {source} file | Pairs | X3 bytes |', '| --- | ---: | ---: |']
    lines += [f'| `{name}` | {count} | {file_bytes[name]:,} |' for name, count in files.most_common(25)]
    lines += ['', '## Limits', '',
              '- `exact` hides relocated values, so it is not byte identity.',
              '- `offsets` also hides every constant: two functions that differ in a number, not a layout, land here.',
              '- `similar` compares operations only. A ratio is not a match percentage.',
              '- `role` says where a function sits, not what it does.',
              '- A call from main into the overlay address range names no unit and is not followed.',
              '- Only game zones are compared; library functions are left out on both sides.', '']
    (ROOT / f'reports/x3-{source.lower()}-homologs.md').write_text('\n'.join(lines))


def write_union(reports, all_rows, x3):
    """Each X3 function once, with its best pair over both sources."""
    best = {}
    for row in all_rows:
        key = (row['x3_unit'], row['x3_va'])
        rank = (CLASSES.index(row['cls']), context_only(row), row['source'] != 'X1', -row['ratio'])
        if key not in best or rank < best[key][0]:
            best[key] = (rank, row)
    sources = collections.defaultdict(set)
    names = collections.defaultdict(dict)
    for row in all_rows:
        sources[row['x3_unit'], row['x3_va']].add(row['source'])
        if row['cls'] in ('exact', 'offsets', 'opcodes') and not row['src_name'].startswith('func_'):
            names[row['x3_unit'], row['x3_va']][row['source']] = row['src_name']
    rows = [row for _, row in sorted(best.values(), key=lambda item: (item[1]['x3_unit'], item[1]['x3_va']))]
    columns = ('x3_unit', 'x3_va', 'x3_size', 'cls', 'ratio', 'source', 'src_unit', 'src_va', 'src_name', 'src_state',
               'src_file', 'evidence', 'late', 'also_in')
    lines = ['\t'.join(columns)]
    for row in rows:
        other = ','.join(sorted(sources[row['x3_unit'], row['x3_va']] - {row['source']}))
        lines.append('\t'.join(str(';'.join(row[c]) if c == 'evidence' else other if c == 'also_in' else row[c]) for c in columns))
    (ROOT / UNION_TSV).write_text('\n'.join(lines) + '\n')
    total3 = len(x3)
    bytes3 = sum(4 * len(f.words) for f in x3)
    out = ['# X1 and X2 homologs of X3 functions', '',
           'Generated by `tools/tu/x3_homologs.py`. Do not edit by hand. Each X3 game function counts once, with its best pair',
           f'over both sources (`{UNION_TSV}`). The pairs of each source, how they were made and how far to trust them are in',
           '[x3-x1-homologs.md](x3-x1-homologs.md) and [x3-x2-homologs.md](x3-x2-homologs.md). Every pair is a lead.', '',
           f'X3 has {total3:,} game functions and {bytes3:,} bytes of game code.', '',
           '| Best pair | X3 functions | Share | X3 bytes | Share of code | Source side is published C |', '| --- | ---: | ---: | ---: | ---: | ---: |']
    groups = [('`exact`', lambda r: r['cls'] == 'exact' and not context_only(r)),
              ('`offsets`', lambda r: r['cls'] == 'offsets' and not context_only(r)),
              ('`opcodes`', lambda r: r['cls'] == 'opcodes' and not context_only(r)),
              ('`similar`, ratio 0.90 or more', lambda r: r['cls'] == 'similar' and not context_only(r) and r['ratio'] >= 0.9),
              ('`similar`, ratio 0.80 to 0.90', lambda r: r['cls'] == 'similar' and not context_only(r) and 0.8 <= r['ratio'] < 0.9),
              ('`similar`, ratio below 0.80', lambda r: r['cls'] == 'similar' and not context_only(r) and r['ratio'] < 0.8),
              ('`role` from two call sites or more', lambda r: r['cls'] == 'role' and not context_only(r)),
              ('last pass only: proximity, call sites or neighbour sets; code not alike or not compared', context_only)]
    count = size = 0
    for label, test in groups:
        mine = [row for row in rows if test(row)]
        count += len(mine)
        size += sum(row['x3_size'] for row in mine)
        out.append(f'| {label} | {len(mine):,} | {100 * len(mine) / total3:.1f}% | {sum(row["x3_size"] for row in mine):,} | '
                   f'{100 * sum(row["x3_size"] for row in mine) / bytes3:.2f}% | {sum(1 for row in mine if row["src_state"] == "exact_c"):,} |')
    out += [f'| all | {count:,} | {100 * count / total3:.1f}% | {size:,} | {100 * size / bytes3:.2f}% | |', '',
            f'{total3 - count:,} X3 game functions have no pair in either game.', '']
    where = collections.Counter(tuple(sorted(v)) for v in sources.values())
    out += ['| Paired with | X3 functions |', '| --- | ---: |']
    out += [f'| {" and ".join(key)} | {value:,} |' for key, value in sorted(where.items())]
    both = [v for v in names.values() if len(v) == 2]
    out += ['', f'Where both games give the X3 function a name through an `exact`, `offsets` or `opcodes` pair ({len(both)} functions), '
            f'{sum(1 for v in both if v["X1"] == v["X2"])} names are equal and {sum(1 for v in both if v["X1"] != v["X2"])} differ. '
            'X1\'s is the symbol; X2\'s is a label.', '',
            'By X3 unit, best pair of any class, with and without the last row of the table above:', '',
            '| X3 unit | Functions | Paired | Paired without the last pass | Bytes | Paired bytes | Paired bytes without the last pass |',
            '| --- | ---: | ---: | ---: | ---: | ---: | ---: |']
    for unit in X3_FILES:
        every = [row for row in rows if row['x3_unit'] == unit]
        mine = [row for row in every if not context_only(row)]
        total = [f for f in x3 if f.unit == unit]
        unit_bytes = sum(4 * len(f.words) for f in total)
        paired, solid = sum(row['x3_size'] for row in every), sum(row['x3_size'] for row in mine)
        out.append(f'| `{unit}` | {len(total):,} | {len(every):,} ({100 * len(every) / len(total):.1f}%) | {len(mine):,} ({100 * len(mine) / len(total):.1f}%) | '
                   f'{unit_bytes:,} | {paired:,} ({100 * paired / unit_bytes:.1f}%) | {solid:,} ({100 * solid / unit_bytes:.1f}%) |')
    out.append('')
    (ROOT / UNION_REPORT).write_text('\n'.join(out))
    return dict(functions=count, bytes=size, share_of_functions=round(100 * count / total3, 2), share_of_code=round(100 * size / bytes3, 2))


def main():
    x3 = load_game('X3', ROOT, X3_FILES, estimated_rows(ROOT, X3_GAME))
    game3 = [f for f in x3 if not f.library]
    sources = (('X1', X1_ROOT, X1_FILES, manifest_rows(X1_ROOT)),
               ('X2', X2_ROOT, X2_FILES, estimated_rows(X2_ROOT, X2_GAME)))
    reports, all_rows, printed = {}, [], {}
    for source, root, files, rows_of in sources:
        functions = load_game(source, root, files, rows_of, load_ledger(root))
        matcher = Matcher(functions, x3)
        rounds = matcher.run()
        checks = cross_checks(matcher, x1_names() if source == 'X2' else {})
        report, rows = write_outputs(source, matcher, rounds, checks, left_over(matcher))
        write_report(report, rows)
        reports[source] = report
        all_rows += rows
        printed[source] = dict(classes={cls: (row['pairs'], row['x3_bytes'], row['by_proximity_or_one_call'])
                                        for cls, row in report['classes'].items()},
                               context_pass=report['context_pass'], measured=checks['context_rules_on_known_pairs'])
    printed['union'] = write_union(reports, all_rows, game3)
    print(json.dumps(printed, indent=1))
    return 0


if __name__ == '__main__':
    sys.exit(main())
