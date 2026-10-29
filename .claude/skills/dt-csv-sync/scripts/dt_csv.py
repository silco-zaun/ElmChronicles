#!/usr/bin/env python3
"""ElmChronicles DataTable CSV 도구.

C++ 행 구조체(FTableRowBase를 상속한 USTRUCT)를 읽어 DataSource/*.csv의 열과 비교하고,
UE가 Export하는 형식(UTF-16 LE + BOM, LF, 첫 열만 따옴표 없음, 자식 → 부모 열 순서)을 지켜 다시 쓴다.

명령:
  check                         모든 CSV를 행 구조체와 비교하고 형식을 검사한다. 어긋나면 종료 코드 1
  columns <FStruct>             구조체가 만들어야 하는 열 목록(순서, 타입, 새 행 기본값)
  show <csv> [--rows R1,R2] [--cols C1,C2]
                                CSV 내용을 읽기 쉽게 출력한다 (UTF-16 파일을 직접 열지 않아도 된다)
  sync <csv> [--struct FStruct] [--rename Old=New ...] [--dry-run]
                                CSV 열을 구조체에 맞춘다. 남는 열은 값 유지, 새 열은 기본값, 없어진 열은 삭제.
                                CSV가 없으면 헤더만 있는 새 파일을 만든다(--struct 필요)
  set <csv> --row <행이름|*> Col=Value [Col=Value ...] [--dry-run]
                                셀 값을 바꾼다. --row '*'는 모든 행
  add-row <csv> <행이름> [Col=Value ...] [--dry-run]
                                행을 추가한다. 적지 않은 열은 C++ 기본값
  roundtrip                     모든 CSV를 읽고 다시 썼을 때 바이트가 같은지 확인한다 (도구 자체 점검)

<csv>는 "DT_Monster", "DT_Monster.csv" 또는 경로로 쓸 수 있다.
--source / --datasource로 헤더 폴더와 CSV 폴더를 바꿀 수 있다 (기본: <프로젝트>/Source, <프로젝트>/DataSource).
"""

from __future__ import annotations

import argparse
import csv
import io
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")


# ---------------------------------------------------------------- 프로젝트 위치

def find_project_root() -> Path | None:
    for start in (Path(__file__).resolve().parent, Path.cwd().resolve()):
        for p in (start, *start.parents):
            if any(p.glob("*.uproject")):
                return p
    return None


# ---------------------------------------------------------------- C++ 헤더 파싱

def strip_comments(src: str) -> str:
    """주석을 지운다. 문자열 리터럴 안의 // 는 건드리지 않는다."""
    out = []
    i, n = 0, len(src)
    while i < n:
        c = src[i]
        if c == '"':
            j = i + 1
            while j < n and src[j] != '"':
                j += 2 if src[j] == "\\" else 1
            out.append(src[i:j + 1])
            i = j + 1
        elif src.startswith("//", i):
            j = src.find("\n", i)
            i = n if j < 0 else j
        elif src.startswith("/*", i):
            j = src.find("*/", i + 2)
            out.append(" ")
            i = n if j < 0 else j + 2
        else:
            out.append(c)
            i += 1
    return "".join(out)


def match_close(s: str, i: int, open_ch: str, close_ch: str) -> int:
    depth = 0
    for j in range(i, len(s)):
        if s[j] == open_ch:
            depth += 1
        elif s[j] == close_ch:
            depth -= 1
            if depth == 0:
                return j
    return -1


def split_top(s: str, sep: str, brackets: str = "(){}[]<>") -> list[str]:
    opens, closes = brackets[0::2], brackets[1::2]
    parts, depth, cur = [], 0, []
    for ch in s:
        if ch in opens:
            depth += 1
        elif ch in closes:
            depth -= 1
        if ch == sep and depth == 0:
            parts.append("".join(cur))
            cur = []
        else:
            cur.append(ch)
    parts.append("".join(cur))
    return parts


def find_top(s: str, target: str) -> int:
    depth = 0
    for i, ch in enumerate(s):
        if ch in "(<{[":
            if ch == target and depth == 0:
                return i
            depth += 1
        elif ch in ")>}]":
            depth -= 1
        elif ch == target and depth == 0:
            return i
    return -1


def remove_macro(s: str, name: str) -> str:
    while True:
        m = re.search(rf"\b{name}\s*\(", s)
        if not m:
            return s
        end = match_close(s, m.end() - 1, "(", ")")
        s = s[:m.start()] + s[end + 1:] if end >= 0 else s[:m.start()]


@dataclass
class Field:
    name: str
    type: str
    default: str | None


@dataclass
class Struct:
    name: str
    parent: str | None
    fields: list[Field]
    ctor: dict[str, str]
    file: Path


@dataclass
class Column:
    name: str
    type: str
    default: str | None
    owner: str


def normalize_type(t: str) -> str:
    t = re.sub(r"\b(class|struct|const|mutable|enum)\s+", "", t)
    t = re.sub(r"\s+", "", t)
    return t


def parse_decl(decl: str) -> Field | None:
    decl = " ".join(decl.split())
    # 비트필드(uint8 bX : 1)는 UE에서 bool 프로퍼티라 True / False로 Export된다.
    is_bitfield = re.search(r"\s*:\s*\d+\s*$", decl) is not None
    decl = re.sub(r"\s*:\s*\d+\s*$", "", decl)
    default = None
    eq = find_top(decl, "=")
    if eq >= 0:
        left, default = decl[:eq], decl[eq + 1:].strip()
    else:
        br = find_top(decl, "{")
        if br >= 0:
            left, default = decl[:br], decl[br + 1:decl.rfind("}")].strip()
        else:
            left = decl
    m = re.match(r"(.*?)(\w+)\s*$", left.strip())
    if not m or not m.group(1).strip():
        return None
    return Field(m.group(2), "bool" if is_bitfield else normalize_type(m.group(1)), default or None)


def parse_ctor(body: str, name: str) -> dict[str, str]:
    m = re.search(rf"\b{name}\s*\(\s*\)\s*(:[^{{]*)?\{{", body)
    if not m:
        return {}
    out = {}
    if m.group(1):  # 초기화 리스트 : A(1), B(2)
        for im in re.finditer(r"(\w+)\s*[({]([^(){}]*)[)}]", m.group(1)):
            out[im.group(1)] = im.group(2).strip()
    b0 = m.end() - 1
    b1 = match_close(body, b0, "{", "}")
    for am in re.finditer(r"\b(\w+)\s*=\s*([^;]+);", body[b0 + 1:b1]):
        out[am.group(1)] = am.group(2).strip()
    return out


def parse_headers(source: Path) -> tuple[dict[str, Struct], dict[str, list[str]]]:
    structs: dict[str, Struct] = {}
    enums: dict[str, list[str]] = {}
    for path in sorted(source.rglob("*.h")):
        src = strip_comments(path.read_text(encoding="utf-8-sig", errors="replace"))

        for m in re.finditer(r"\bUENUM\s*\(", src):
            p = match_close(src, m.end() - 1, "(", ")")
            hm = re.compile(r"\s*enum\s+(?:class\s+)?(\w+)\s*(?::\s*[\w:]+\s*)?\{").match(src, p + 1)
            if not hm:
                continue
            b0 = hm.end() - 1
            body = src[b0 + 1:match_close(src, b0, "{", "}")]
            names = []
            for part in split_top(body, ",", "(){}"):
                part = remove_macro(part, "UMETA").strip()
                nm = re.match(r"(\w+)", part)
                if nm:
                    names.append(nm.group(1))
            enums[hm.group(1)] = names

        for m in re.finditer(r"\bUSTRUCT\s*\(", src):
            p = match_close(src, m.end() - 1, "(", ")")
            hm = re.compile(
                r"\s*struct\s+(?:\w+_API\s+)?(\w+)\s*(?:final\s*)?(?::\s*(?:public\s+)?([\w:]+))?\s*\{"
            ).match(src, p + 1)
            if not hm:
                continue
            b0 = hm.end() - 1
            body = src[b0 + 1:match_close(src, b0, "{", "}")]
            fields = []
            for fm in re.finditer(r"\bUPROPERTY\s*\(", body):
                fp = match_close(body, fm.end() - 1, "(", ")")
                j, depth = fp + 1, 0
                while j < len(body):
                    ch = body[j]
                    if ch in "({<":
                        depth += 1
                    elif ch in ")}>":
                        depth -= 1
                    elif ch == ";" and depth == 0:
                        break
                    j += 1
                f = parse_decl(body[fp + 1:j])
                if f:
                    fields.append(f)
            name = hm.group(1)
            structs[name] = Struct(name, hm.group(2), fields, parse_ctor(body, name), path)
    return structs, enums


def table_chain(structs: dict[str, Struct], name: str) -> list[Struct] | None:
    """자식 → 부모 순서의 구조체 목록. FTableRowBase까지 이어지지 않으면 None."""
    chain, cur = [], name
    while cur and cur != "FTableRowBase":
        s = structs.get(cur)
        if not s:
            return None
        chain.append(s)
        cur = s.parent
    return chain if cur == "FTableRowBase" else None


def table_columns(structs: dict[str, Struct], name: str) -> list[Column]:
    chain = table_chain(structs, name)
    if chain is None:
        raise SystemExit(f"{name}: FTableRowBase를 상속한 USTRUCT를 찾지 못했습니다.")
    overrides: dict[str, str] = {}
    for s in reversed(chain):  # 자식 생성자의 덮어쓰기가 이긴다
        overrides.update(s.ctor)
    cols = []
    for s in chain:  # UE Export 순서 = TFieldIterator 순서 = 자식 필드 먼저, 그다음 부모
        for f in s.fields:
            cols.append(Column(f.name, f.type, overrides.get(f.name, f.default), s.name))
    return cols


def table_structs(structs: dict[str, Struct]) -> list[str]:
    return [n for n in structs if table_chain(structs, n)]


def leaf_table_structs(structs: dict[str, Struct]) -> list[str]:
    tables = table_structs(structs)
    parents = {structs[n].parent for n in tables}
    return [n for n in tables if n not in parents]


# ---------------------------------------------------------------- 기본값 → CSV 셀

INT_TYPES = {"int8", "int16", "int32", "int64", "uint8", "uint16", "uint32", "uint64", "int"}
OBJ_PREFIXES = ("TSoftObjectPtr<", "TSoftClassPtr<", "TObjectPtr<", "TSubclassOf<",
                "TWeakObjectPtr<", "TLazyObjectPtr<")
CONTAINER_PREFIXES = ("TArray<", "TSet<", "TMap<")
VECTOR_LAYOUTS = {
    "FVector": ("X", "Y", "Z"), "FVector3d": ("X", "Y", "Z"), "FVector3f": ("X", "Y", "Z"),
    "FVector2D": ("X", "Y"), "FRotator": ("Pitch", "Yaw", "Roll"),
    "FLinearColor": ("R", "G", "B", "A"),
}
VECTOR_CONSTANTS = {
    "FVector::ZeroVector": 0.0, "FVector::OneVector": 1.0, "FVector2D::ZeroVector": 0.0,
    "FRotator::ZeroRotator": 0.0, "FLinearColor::Black": None, "FLinearColor::White": 1.0,
}


def to_number(expr: str) -> float | None:
    e = expr.strip()
    e = re.sub(r"^static_cast<\w+>\((.*)\)$", r"\1", e)
    e = e.rstrip("fF")
    try:
        return float(e)
    except ValueError:
        return None


def string_literal(expr: str | None) -> str | None:
    if not expr:
        return None
    m = re.fullmatch(r'\s*(?:TEXT\s*\(\s*)?"((?:[^"\\]|\\.)*)"\s*\)?\s*', expr)
    return m.group(1) if m else None


def default_cell(col_type: str, expr: str | None, structs, enums, nested: bool = False,
                 depth: int = 0) -> tuple[str, str | None]:
    """(CSV 셀 텍스트, 확인이 필요하면 메모)."""
    t = col_type
    if t in ("float", "double"):
        if not expr:
            return "0.000000", None
        v = to_number(expr)
        if v is None:
            return "0.000000", f'기본값 식 "{expr}"을 계산하지 못해 0으로 넣음'
        return f"{v:.6f}", None
    if t in INT_TYPES:
        if not expr:
            return "0", None
        try:
            return str(int(expr.strip().rstrip("uUlL"), 0)), None
        except ValueError:
            return "0", f'기본값 식 "{expr}"을 계산하지 못해 0으로 넣음'
    if t == "bool":
        return ("True" if expr and expr.strip() == "true" else "False"), None
    if t == "FString":
        s = string_literal(expr) or ""
        return (f'"{s}"' if nested else s), None
    if t == "FName":
        s = string_literal(expr)
        if not s:
            return "None", None
        return (f'"{s}"' if nested else s), None
    enum_name = re.sub(r"^TEnumAsByte<(\w+)>$", r"\1", t).split("::")[-1]
    if enum_name in enums:
        if expr:
            return expr.split("::")[-1].strip(), None
        return (enums[enum_name][0] if enums[enum_name] else ""), None
    if t.startswith(OBJ_PREFIXES) or t.endswith("*"):
        return "None", None
    if t.startswith(CONTAINER_PREFIXES):
        if expr and expr.strip() not in ("", "{}"):
            return ("()" if nested else ""), f'컨테이너 기본값 "{expr}"은 옮기지 않고 빈 값으로 넣음'
        return ("()" if nested else ""), None
    if t in VECTOR_LAYOUTS:
        keys = VECTOR_LAYOUTS[t]
        vals = [0.0] * len(keys)
        note = None
        if expr:
            e = expr.strip()
            if e in VECTOR_CONSTANTS and VECTOR_CONSTANTS[e] is not None:
                vals = [VECTOR_CONSTANTS[e]] * len(keys)
            else:
                m = re.fullmatch(rf"(?:{t})?\s*[({{](.*)[)}}]", e)
                nums = [to_number(x) for x in split_top(m.group(1), ",")] if m else [None]
                if nums and all(n is not None for n in nums) and len(nums) in (1, len(keys)):
                    vals = nums * len(keys) if len(nums) == 1 else nums
                else:
                    note = f'기본값 식 "{expr}"을 계산하지 못해 0으로 넣음'
        if t == "FLinearColor" and not expr:
            vals = [0.0, 0.0, 0.0, 0.0]
        return "(" + ",".join(f"{k}={v:.6f}" for k, v in zip(keys, vals)) + ")", note
    if t in structs and depth < 4:
        parts, notes = [], []
        for f in structs[t].fields:
            cell, note = default_cell(f.type, f.default, structs, enums, nested=True, depth=depth + 1)
            parts.append(f"{f.name}={cell}")
            if note:
                notes.append(note)
        note = "중첩 구조체 기본값 — 형식을 한 번 확인할 것" + (f" ({'; '.join(notes)})" if notes else "")
        return "(" + ",".join(parts) + ")", note
    return "", f"타입 {t}의 CSV 형식을 몰라 빈 칸으로 둠 — 값을 직접 채울 것"


# ---------------------------------------------------------------- CSV 읽기/쓰기

@dataclass
class CsvFile:
    path: Path
    header: list[str]
    rows: list[list[str]]
    encoding: str = "utf-16-le"
    bom: bytes = b"\xff\xfe"
    newline: str = "\n"
    trailing: bool = True
    notes: list[str] = field(default_factory=list)

    @property
    def columns(self) -> list[str]:
        return self.header[1:]

    def describe(self) -> str:
        enc = {"utf-16-le": "UTF-16 LE", "utf-16-be": "UTF-16 BE", "utf-8": "UTF-8"}.get(self.encoding, self.encoding)
        enc += "+BOM" if self.bom else " (BOM 없음)"
        nl = "LF" if self.newline == "\n" else "CRLF"
        return f"{enc}, {nl}, {len(self.rows)}행"


def read_csv(path: Path) -> CsvFile:
    raw = path.read_bytes()
    if raw.startswith(b"\xff\xfe"):
        enc, bom = "utf-16-le", b"\xff\xfe"
    elif raw.startswith(b"\xfe\xff"):
        enc, bom = "utf-16-be", b"\xfe\xff"
    elif raw.startswith(b"\xef\xbb\xbf"):
        enc, bom = "utf-8", b"\xef\xbb\xbf"
    else:
        enc, bom = "utf-8", b""
    text = raw[len(bom):].decode(enc)
    newline = "\r\n" if "\r\n" in text else "\n"
    parsed = [r for r in csv.reader(io.StringIO(text, newline="")) if r]
    if not parsed:
        raise SystemExit(f"{path.name}: 비어 있는 CSV입니다.")
    return CsvFile(path, parsed[0], parsed[1:], enc, bom, newline, text.endswith(newline))


def render_csv(f: CsvFile) -> bytes:
    def quote(s: str) -> str:
        return '"' + s.replace('"', '""') + '"'

    lines = [",".join(f.header)]
    lines += [row[0] + "".join("," + quote(c) for c in row[1:]) for row in f.rows]
    text = f.newline.join(lines) + (f.newline if f.trailing else "")
    return f.bom + text.encode(f.encoding)


def resolve_csv(arg: str, datasource: Path) -> Path:
    p = Path(arg)
    if p.suffix.lower() != ".csv":
        p = p.with_suffix(".csv")
    if not p.is_absolute() and not p.exists():
        p = datasource / p.name
    return p


def guess_struct(f: CsvFile, structs: dict[str, Struct]) -> tuple[str | None, float]:
    """CSV 헤더와 열이 가장 많이 겹치는 테이블 구조체. DT 에셋의 RowStruct는 재임포트 전이면 옛 이름이라 믿지 않는다."""
    have = set(f.columns)
    stem = f.path.stem.removeprefix("DT_").lower()
    best, best_score = None, 0.0
    for name in table_structs(structs):
        want = {c.name for c in table_columns(structs, name)}
        score = len(have & want) / max(1, len(have | want))
        score += 0.01 if stem and stem in name.lower() else 0.0  # 동점이면 파일 이름이 들어간 구조체
        if score > best_score:
            best, best_score = name, score
    return best, best_score


# ---------------------------------------------------------------- 명령

def cmd_check(args, structs, enums) -> int:
    problems = 0
    csv_paths = sorted(args.datasource.glob("*.csv"))
    matched = set()
    for path in csv_paths:
        f = read_csv(path)
        issues = []
        # 인코딩과 줄바꿈은 참고만 한다. 다른 형식이어도 임포트는 되고, sync / set은 원래 형식을 그대로 유지한다.
        notes = []
        if f.encoding != "utf-16-le" or not f.bom:
            notes.append("인코딩이 UTF-16 LE+BOM이 아님 — 임포트는 되므로 그대로 둔다")
        if f.newline != "\n":
            notes.append("줄바꿈이 CRLF — 임포트는 되므로 그대로 둔다")
        if f.header[0] != "---":
            issues.append(f'헤더 첫 칸이 "---"가 아님: "{f.header[0]}"')
        if render_csv(f) != path.read_bytes():
            issues.append("따옴표 규칙이 다름(첫 열만 따옴표 없음, 나머지 모두 따옴표) — sync로 다시 쓰면 고쳐진다")
        for row in f.rows:
            if len(row) != len(f.header):
                issues.append(f'행 "{row[0]}"의 칸 수 {len(row) - 1} ≠ 열 수 {len(f.columns)}')

        name = args.map.get(path.stem) if args.map else None
        score = 1.0
        if not name:
            name, score = guess_struct(f, structs)
        if not name or score < 0.5:
            print(f"✗ {path.name}  [{f.describe()}] — 맞는 행 구조체를 찾지 못함")
            problems += 1
            continue
        matched.add(name)
        want = [c.name for c in table_columns(structs, name)]
        have = f.columns
        missing = [c for c in want if c not in have]
        extra = [c for c in have if c not in want]
        if not missing and not extra and want != have:
            issues.append("열 순서가 다름 (기대: 자식 필드 → 부모 필드, 선언 순서)")
        cols = {c.name: c for c in table_columns(structs, name)}
        for c in missing:
            cell, _ = default_cell(cols[c].type, cols[c].default, structs, enums)
            issues.append(f'CSV에 없는 열: {c} ({cols[c].type}, 기본값 "{cell}", {cols[c].owner})')
        for c in extra:
            issues.append(f"구조체에 없는 열: {c}")

        print(f"{'✗' if issues else '✓'} {path.name} ↔ {name}  [{f.describe()}]")
        for i in issues:
            print(f"    - {i}")
        for n in notes:
            print(f"    · {n}")
        problems += bool(issues)

    orphans = [n for n in leaf_table_structs(structs) if n not in matched]
    for n in orphans:
        print(f"? {n}: 이 구조체와 맞는 CSV가 없음 (새 테이블이면 sync DT_<이름> --struct {n}로 만든다)")
    return 1 if problems else 0


def cmd_columns(args, structs, enums) -> int:
    for i, c in enumerate(table_columns(structs, args.struct), 1):
        cell, note = default_cell(c.type, c.default, structs, enums)
        print(f'{i:>2}. {c.name:<22} {c.type:<40} "{cell}"  ← {c.owner}' + (f"  ※ {note}" if note else ""))
    return 0


def cmd_show(args, structs, enums) -> int:
    f = read_csv(resolve_csv(args.csv, args.datasource))
    rows = [r for r in f.rows if not args.rows or r[0] in args.rows.split(",")]
    cols = args.cols.split(",") if args.cols else f.columns
    print(f"{f.path.name}  [{f.describe()}]")
    for row in rows:
        print(f"[{row[0]}]")
        for c in cols:
            if c in f.columns:
                print(f"  {c} = {row[f.header.index(c)]}")
            else:
                print(f"  {c} = (열 없음)")
    return 0


def cmd_sync(args, structs, enums) -> int:
    path = resolve_csv(args.csv, args.datasource)
    renames = dict(r.split("=", 1) for r in args.rename or [])

    if path.exists():
        f = read_csv(path)
        name = args.struct
        if not name:
            name, score = guess_struct(f, structs)
            if not name or score < 0.5:
                raise SystemExit(f"{path.name}: 맞는 구조체를 찾지 못했습니다. --struct로 지정하세요.")
    else:
        if not args.struct:
            raise SystemExit(f"{path.name}이 없습니다. 새로 만들려면 --struct로 구조체를 지정하세요.")
        name = args.struct
        f = CsvFile(path, ["---"], [])
        print(f"새 파일: {path.name} (UTF-16 LE+BOM, LF, 헤더만)")

    cols = table_columns(structs, name)
    want = [c.name for c in cols]
    old_cols = [renames.get(c, c) for c in f.columns]
    for old, new in renames.items():
        if old not in f.columns:
            raise SystemExit(f"--rename {old}={new}: CSV에 {old} 열이 없습니다.")
        if new not in want:
            raise SystemExit(f"--rename {old}={new}: 구조체 {name}에 {new} 필드가 없습니다.")

    added = [c for c in cols if c.name not in old_cols]
    removed = [c for c in old_cols if c not in want]
    defaults = {c.name: default_cell(c.type, c.default, structs, enums) for c in added}

    new_rows = []
    for row in f.rows:
        values = dict(zip(old_cols, row[1:]))
        new_rows.append([row[0]] + [values[c] if c in values else defaults[c][0] for c in want])

    print(f"{path.name} ↔ {name}")
    for old, new in renames.items():
        print(f"  이름 변경: {old} → {new} (값 유지)")
    for c in added:
        cell, note = defaults[c.name]
        print(f'  추가: {c.name} ({c.type}) = "{cell}"' + (f"  ※ {note}" if note else ""))
    for c in removed:
        idx = old_cols.index(c) + 1
        vals = {row[0]: row[idx] for row in f.rows}
        print(f"  삭제: {c}  (지워지는 값: {vals})")
    if [c for c in old_cols if c in want] != [c for c in want if c in old_cols]:
        print("  순서: 구조체 순서(자식 → 부모)로 다시 정렬")
    if added and removed and not renames:
        print("  ⚠ 추가와 삭제가 함께 있습니다. 이름만 바꾼 필드라면 --rename 옛이름=새이름으로 다시 실행해야 값이 남습니다.")

    f.header = ["---"] + want
    f.rows = new_rows
    out = render_csv(f)
    if args.dry_run:
        print("  (dry-run: 파일을 쓰지 않음)")
    elif path.exists() and out == path.read_bytes():
        print("  변경 없음")
    else:
        path.write_bytes(out)
        print("  저장함")
    return 0


def cmd_set(args, structs, enums) -> int:
    path = resolve_csv(args.csv, args.datasource)
    f = read_csv(path)
    targets = [r for r in f.rows if args.row == "*" or r[0] == args.row]
    if not targets:
        raise SystemExit(f'{path.name}: 행 "{args.row}"이 없습니다. 행: {[r[0] for r in f.rows]}')
    for assign in args.values:
        col, value = assign.split("=", 1)
        if col not in f.columns:
            raise SystemExit(f"{path.name}: {col} 열이 없습니다.")
        idx = f.header.index(col)
        for row in targets:
            print(f'  [{row[0]}] {col}: "{row[idx]}" → "{value}"')
            row[idx] = value
    if args.dry_run:
        print("  (dry-run: 파일을 쓰지 않음)")
    else:
        path.write_bytes(render_csv(f))
        print("  저장함")
    return 0


def cmd_add_row(args, structs, enums) -> int:
    path = resolve_csv(args.csv, args.datasource)
    f = read_csv(path)
    name = args.struct or guess_struct(f, structs)[0]
    cols = table_columns(structs, name)
    if [c.name for c in cols] != f.columns:
        raise SystemExit(f"{path.name}의 열이 {name}와 다릅니다. 먼저 sync로 맞추세요.")
    if any(r[0] == args.row for r in f.rows):
        raise SystemExit(f'{path.name}: 행 "{args.row}"이 이미 있습니다. 값을 바꾸려면 set을 쓰세요.')
    values = dict(v.split("=", 1) for v in args.values or [])
    for col in values:
        if col not in f.columns:
            raise SystemExit(f"{path.name}: {col} 열이 없습니다.")
    row = [args.row]
    print(f'{path.name}: 행 "{args.row}" 추가')
    for c in cols:
        if c.name in values:
            row.append(values[c.name])
        else:
            cell, note = default_cell(c.type, c.default, structs, enums)
            row.append(cell)
            if note:
                print(f"  ※ {c.name}: {note}")
    for c, v in zip(f.columns, row[1:]):
        print(f"  {c} = {v}" + ("" if c in values else "  (기본값)"))
    f.rows.append(row)
    if args.dry_run:
        print("  (dry-run: 파일을 쓰지 않음)")
    else:
        path.write_bytes(render_csv(f))
        print("  저장함")
    return 0


def cmd_roundtrip(args, structs, enums) -> int:
    bad = 0
    for path in sorted(args.datasource.glob("*.csv")):
        same = render_csv(read_csv(path)) == path.read_bytes()
        print(f"{'✓' if same else '✗'} {path.name}")
        bad += not same
    return 1 if bad else 0


def main() -> int:
    root = find_project_root()
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--source", type=Path, default=root / "Source" if root else None)
    ap.add_argument("--datasource", type=Path, default=root / "DataSource" if root else None)
    ap.add_argument("--map", action="append", help="CSV와 구조체를 직접 짝지음: DT_Monster=FMonsterCharacterTableRow")
    sub = ap.add_subparsers(dest="cmd", required=True)

    sub.add_parser("check")
    p = sub.add_parser("columns")
    p.add_argument("struct")
    p = sub.add_parser("show")
    p.add_argument("csv")
    p.add_argument("--rows")
    p.add_argument("--cols")
    p = sub.add_parser("sync")
    p.add_argument("csv")
    p.add_argument("--struct")
    p.add_argument("--rename", action="append", metavar="Old=New")
    p.add_argument("--dry-run", action="store_true")
    p = sub.add_parser("set")
    p.add_argument("csv")
    p.add_argument("--row", required=True)
    p.add_argument("values", nargs="+", metavar="Col=Value")
    p.add_argument("--dry-run", action="store_true")
    p = sub.add_parser("add-row")
    p.add_argument("csv")
    p.add_argument("row")
    p.add_argument("values", nargs="*", metavar="Col=Value")
    p.add_argument("--struct")
    p.add_argument("--dry-run", action="store_true")
    sub.add_parser("roundtrip")

    args = ap.parse_args()
    if not args.source or not args.datasource:
        raise SystemExit(".uproject를 찾지 못했습니다. --source와 --datasource를 지정하세요.")
    args.map = dict(m.split("=", 1) for m in args.map) if args.map else {}
    structs, enums = parse_headers(args.source)
    handler = {"check": cmd_check, "columns": cmd_columns, "show": cmd_show, "sync": cmd_sync,
               "set": cmd_set, "add-row": cmd_add_row, "roundtrip": cmd_roundtrip}[args.cmd]
    return handler(args, structs, enums)


if __name__ == "__main__":
    sys.exit(main())
