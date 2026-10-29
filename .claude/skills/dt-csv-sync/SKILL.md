---
name: dt-csv-sync
description: ElmChronicles의 DataTable 행 구조체(FTableRowBase를 상속한 USTRUCT, 주로 Source/ElmChronicles/Data/Tables/*TableRow.h)가 바뀌면 DataSource/*.csv의 열을 구조체에 맞게 고친다. 행 구조체에 필드를 추가·삭제·이름 변경·순서 변경했을 때, 부모 구조체(FCharacterTableRow, FAICharacterTableRow)를 고쳐 여러 테이블이 함께 바뀔 때, 새 행 구조체(새 테이블)를 만들었을 때는 사용자가 따로 요청하지 않아도 작업 마지막에 이 스킬을 쓴다. "CSV 맞춰줘", "csv 수정", "DT 열 추가", "테이블 구조 바꿨어", "CSV에 값 넣어줘", "DT 값 확인"처럼 CSV를 읽거나 고치는 모든 요청에도 쓴다. CSV가 UTF-16이라 Write/Edit로 직접 고치면 깨지므로, CSV를 다룰 때는 항상 이 스킬의 스크립트를 거친다.
---

# DataTable CSV 동기화

`DataSource/*.csv`는 DataTable 에셋의 원본이다. DT 에셋은 사람이 이 CSV로 다시 임포트한다. 행 구조체와 CSV의 열이 어긋나면 임포트가 실패하거나 값이 빈다. 구조체를 바꾸는 사람이 CSV를 매번 따로 챙기기 번거로우므로, **행 구조체를 건드린 작업이 끝나면 요청이 없어도 이 절차를 돈다.**

모든 작업은 `scripts/dt_csv.py`로 한다. 이유는 세 가지다.
- CSV가 UTF-16 LE + BOM이다. Write / Edit 도구로 고치면 UTF-8로 저장되거나 글자가 깨진다.
- 열 순서가 UE Export 규칙(자식 필드 → 부모 필드)을 따라야 한다.
- 따옴표 규칙: 첫 열만 따옴표가 없고 나머지는 모두 따옴표로 감싼다.

스크립트는 원래 파일의 인코딩과 줄바꿈을 그대로 유지한다.

```bash
# 프로젝트 루트에서 실행한다(Python 3.9 이상, 표준 라이브러리만 사용).
# 스크립트가 .uproject를 찾아 Source/와 DataSource/ 위치를 정한다.
python .claude/skills/dt-csv-sync/scripts/dt_csv.py <명령>
```

Git Bash(MSYS)는 `/`로 시작하는 인자를 Windows 경로로 바꾼다. `set ... AttackMontage=/Game/...`가 `C:/Program Files/Git/Game/...`으로 저장된다. Git Bash에서는 명령 앞에 `MSYS_NO_PATHCONV=1`을 붙이거나 PowerShell에서 실행한다. `set` 뒤에는 `show`로 값을 확인한다.

| 명령 | 용도 |
|---|---|
| `check` | 모든 CSV를 행 구조체와 비교한다. 빠진 열, 남는 열, 순서, 따옴표, 칸 수를 검사한다. 어긋나면 종료 코드 1. CSV가 없는 테이블 구조체도 알려 준다 |
| `columns <FStruct>` | 구조체가 만들어야 하는 열 목록(순서, 타입, 새 행 기본값, 선언한 구조체) |
| `sync <CSV> [--rename 옛=새 ...] [--struct FStruct] [--dry-run]` | 열을 구조체에 맞춘다. 남는 열은 값을 유지하고, 새 열에는 C++ 기본값을 넣고, 없어진 열은 지운다. CSV가 없으면 헤더만 있는 새 파일을 만든다(`--struct` 필요) |
| `set <CSV> --row <행\|*> 열=값 ... [--dry-run]` | 셀 값을 바꾼다. `--row '*'`는 모든 행 |
| `add-row <CSV> <행이름> [열=값 ...] [--dry-run]` | 행을 추가한다. 적지 않은 열은 C++ 기본값. 열이 구조체와 맞아야 한다(먼저 sync) |
| `show <CSV> [--rows A,B] [--cols X,Y]` | CSV 내용을 읽기 쉽게 출력한다. UTF-16 파일을 직접 열 필요가 없다 |
| `roundtrip` | 읽고 다시 써도 바이트가 같은지 확인한다(스크립트 자체 점검) |

`<CSV>`는 `DT_Monster`, `DT_Monster.csv`, 경로 모두 받는다.

## 절차

1. **무엇이 바뀌었는지 본다.** `git diff -- Source`에서 행 구조체 변경을 읽고, 필드마다 추가 / 삭제 / **이름 변경** / 순서 변경을 구분한다.
   - 이름 변경은 스크립트가 알아낼 수 없다. 같은 자리에서 같은 타입의 필드 이름만 바뀌었다면 이름 변경으로 보고 `--rename`을 쓴다. 그래야 기존 값이 남는다.
   - 타입까지 바뀌었다면 값 형식이 달라지므로 삭제 + 추가로 다룬다.
2. **`check`** 로 영향받는 CSV를 확인한다. 부모 구조체(`FCharacterTableRow` 등)를 바꾸면 그것을 상속하는 테이블의 CSV가 모두 어긋난다(DT_Player, DT_Monster, DT_NPC).
3. **CSV마다 `sync --dry-run`** 으로 계획을 본다.
   - "추가와 삭제가 함께 있다"는 경고가 나오면 1번의 판단과 맞는지 확인한다. 이름 변경이면 `--rename`을 붙인다.
   - "지워지는 값"에 의미 있는 데이터가 있는데 diff로 삭제 의도가 분명하지 않으면, 지우기 전에 사용자에게 묻는다.
4. **`sync`** 를 실제로 실행한다.
5. **새 열의 값을 채운다.** 새 열에는 C++ 기본값이 들어간다. 행마다 달라야 할 근거가 있으면 `set`으로 채운다. 근거는 `Docs/Plan.md`, 대화에서 정한 값, 같은 성격의 다른 열 등이다. 근거가 없으면 기본값으로 두고 보고에 적는다.
6. **※ 메모를 처리한다.** sync가 ※ 메모를 남긴 열(형식을 모르는 타입, 계산하지 못한 기본값)은 아래 형식표를 보고 `set`으로 고친다. 확신이 없으면 보고에 적는다.
7. **`check`를 다시 실행**해서 모두 ✓인지 확인한다.

## 새 테이블

새 행 구조체를 만들었는데 CSV가 없으면, `check`가 `?` 줄로 알려 준다. 이때 `sync DT_<이름> --struct F<이름>TableRow`로 헤더만 있는 CSV를 만든다. 파일 이름은 DT 에셋 이름과 같게 한다.

DT 에셋 생성과 `DA_GameData.mTables` 등록은 에디터 작업이므로 사용자가 한다. 행 데이터는 사용자가 주거나 요청할 때 `add-row`로 넣는다. 기획 수치가 필요하면 `Docs/Plan.md`를 기준으로 한다.

## 값 형식 (UE CSV Export와 같게)

| 타입 | 예 | 비고 |
|---|---|---|
| float / double | `300.000000` | 소수점 6자리 |
| int32 등 | `50` | |
| bool, 비트필드 | `True` / `False` | |
| FString | `해골병사` | 비면 빈 칸 |
| FName | `Tavern_01` | 비면 `None` |
| enum | `Epic` | 열거자 이름만(`EMonsterType::` 없이) |
| TSoftObjectPtr | `/Game/Path/Asset.Asset` | 비면 `None` |
| TSoftClassPtr (BP 클래스) | `/Game/Blueprints/GA_X.GA_X_C` | 끝에 `_C`. 비면 `None` |
| C++ 클래스 참조 | `/Script/ElmChronicles.GameplayAbility_Melee` | |
| FVector | `(X=0.000000,Y=0.000000,Z=0.000000)` | |
| FRotator | `(Pitch=0.000000,Yaw=-90.000000,Roll=0.000000)` | |
| TArray (빈 값) | 빈 칸 | |
| TArray<구조체> | `((Type=PhysicalAttack,Value=10.000000))` | 원소마다 괄호 하나, 원소 사이는 쉼표 |

여기에 없는 타입(FText, FGameplayTag, TMap 등)은 이 프로젝트 CSV에 아직 쓰인 적이 없다. sync가 빈 칸과 ※ 메모를 남기므로 보고에 적고, 사용자가 에디터에서 한 번 Export해 형식을 확인하게 한다.

## 하지 않는 것

- DT `.uasset`의 RowStruct 이름은 믿지 않는다. 구조체 이름을 바꾼 뒤 재임포트하기 전에는 옛 이름이 남아 있다. 스크립트는 열이 가장 많이 겹치는 구조체로 짝을 짓는다. 짝이 틀리면 `--map DT_X=FStruct`로 직접 정한다.
- `[CoreRedirects]` 추가나 에셋 마이그레이션은 안내하지 않는다. 구조가 바뀌면 DT를 다시 임포트하는 것이 이 프로젝트의 방식이다.
- `DT_LevelStat.csv`는 UTF-8 + CRLF다. `check`가 `·` 참고로만 표시하고 sync / set도 형식을 유지하므로, 따로 변환하지 않는다.

## 보고

작업이 끝나면 짧게 알린다. 예:

> CSV 갱신: `DT_Player` / `DT_Monster` / `DT_NPC`에 `AttackDelay` 열 추가(기본값 0.5). `DT_Monster`의 `DetectRange` → `SightRange`(값 유지). DT 세 개를 다시 임포트하세요.

기본값으로 둔 열과 ※ 메모가 남은 열이 있으면 함께 적는다. 사용자가 할 에셋 작업(재임포트, 새 DT 만들기)은 한 줄로만 안내한다.
