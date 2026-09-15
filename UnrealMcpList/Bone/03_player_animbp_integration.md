# 3. 플레이어 AnimBP 통합

## 목적

리타게팅된 최소 이동 세트를 타겟 플레이어의 AnimBP에 연결한다. 엔진·플러그인 의존성이 큰 샘플 AnimBP를 복제하지 않고, 단순한 Blend Space 기반으로 시작한다.

## 사전 확인

- 플레이어 Blueprint의 Skeletal Mesh가 타겟 메시인지 확인한다.
- AnimBP의 Target Skeleton이 타겟 메시의 Skeleton과 같은지 확인한다.
- 플레이어 Blueprint가 Animation Blueprint 모드를 사용하고 올바른 Anim Class를 지정하는지 확인한다.
- 기존 AnimBP를 보존해야 하면 복사본 또는 신규 AnimBP에서 작업한다.

## 기본 변수

Event Graph에서 아래 값을 계산한다.

| 변수 | 계산 방식 | 용도 |
| --- | --- | --- |
| `Speed` | `Get Velocity`의 XY 길이 | Idle/Walk/Run/Sprint 블렌딩 |
| `IsInAir` | Character Movement의 Falling 상태 | Jump/Fall/Land 전환 |
| `IsCrouching` | Pawn 또는 Character Movement의 Crouch 상태 | Crouch 상태 전환 |

`Try Get Pawn Owner` 결과가 유효한지 확인한 뒤 값을 읽는다. 소유자가 없을 때 이전 값이나 잘못된 참조를 사용하지 않도록 한다.

## 기본 Blend Space

1D Blend Space를 새로 만들고 축은 `Speed`로 둔다. 축 범위와 샘플 속도는 실제 Character Movement 설정에 맞춘다.

```text
Speed 0        : Idle
Speed Walk     : Walk
Speed Run      : Run
Speed Sprint   : Sprint
```

Anim Graph에서 `Speed → Blend Space → Output Pose`를 먼저 연결한다. 이 단계가 정상 동작한 뒤에만 상태 머신을 추가한다.

## 확장 순서

1. 기본 이동: Idle, Walk, Run, Sprint
2. 공중: Jump Start, Fall Loop, Land
3. 웅크리기: Crouch Idle, Crouch Walk, Stand↔Crouch
4. 이동 품질: Start, Stop, 방향 전환, Turn
5. 특수 이동: Slide, Vault, Mantle, Hurdle, Climb

Jump·Crouch·Traversal은 애니메이션만 있다고 연결하지 않는다. 해당 게임플레이 상태, 입력, 이동 코드의 트리거가 준비된 경우에만 State Machine 또는 Montage로 추가한다.

## Montage와 상태 머신 선택

| 상황 | 권장 방식 |
| --- | --- |
| 지속 이동·공중·웅크리기 | State Machine |
| 짧고 독립적인 동작 | Montage |
| 이동 제어와 밀접한 Vault/Mantle 등 | 게임플레이 트리거 + Montage 또는 전용 Traversal 상태 |

Root Motion 사용 여부는 프로젝트의 Character Movement 설계와 일치해야 한다. 애니메이션만 보고 Root Motion을 임의로 켜거나 끄지 않는다.
