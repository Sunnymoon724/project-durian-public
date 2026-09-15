# 1. 애셋 선별 및 마이그레이션

## 목적

소스 프로젝트 또는 샘플에서 필요한 캐릭터·애니메이션과 의존성만 안전하게 가져온다. 샘플 전체, 데모 맵, 사용하지 않는 Blueprint를 무차별적으로 가져오지 않는다.

## 선별 순서

1. 소스 캐릭터에서 사용할 동작 목록을 확정한다.
2. 소스 Skeletal Mesh와 Skeleton을 확인한다.
3. 각 동작이 Animation Sequence인지, Blend Space인지, Montage인지 확인한다.
4. 소스 AnimBP의 Reference Viewer로 Animation Sequence, Curve, Notify, Control Rig, Pose Asset 의존성을 확인한다.
5. 새 리소스가 필요한 경우에만 다음 항목을 함께 선별한다.
   - Skeletal Mesh, Skeleton, Physics Asset
   - IK Rig, IK Retargeter
   - Animation Sequence, Blend Space, Montage
   - 필요한 Anim Notify Class, Curve, Control Rig, Pose Asset

## 권장 최소 애니메이션 세트

첫 검증에는 아래만 사용한다.

| 분류 | 동작 |
| --- | --- |
| 기본 이동 | Idle, Walk, Run, Sprint |
| 공중 | Jump Start, Fall Loop, Land |

그 다음 Crouch, Start/Stop, 방향 전환, Turn, Slide, Vault, Mantle, Hurdle, Climb 순으로 확장한다.

## Migrate 절차

1. 소스 프로젝트의 Content Browser에서 기준 에셋을 선택한다. 보통 Skeletal Mesh, IK Retargeter 또는 AnimBP가 적합하다.
2. `Asset Actions → Migrate`를 실행한다.
3. 의존성 목록을 읽고, 의도하지 않은 데모 맵·샘플 Blueprint·대형 콘텐츠가 포함되는지 확인한다.
4. 타겟 프로젝트의 `Content` 폴더를 목적지로 선택한다.
5. Content Browser에서 이동된 파일의 참조 오류와 저장 위치를 확인한다.

## 권장 폴더 구조

프로젝트의 기존 규칙이 우선이다. 별도 규칙이 없다면 다음처럼 소스와 타겟을 분리한다.

```text
Content/
├─ Characters/<TargetCharacter>/
├─ Animations/<TargetCharacter>/SourceReference/
├─ Animations/<TargetCharacter>/Retargeted/
├─ Retarget/<Source>_To_<Target>/
└─ _BP/Characters/
```

## 하지 않을 일

- 운영체제 파일 탐색기에서 `.uasset`을 복사하거나 이름을 바꾸지 않는다.
- 타겟과 스켈레톤이 다른 소스 AnimBP를 그대로 타겟에 지정하지 않는다.
- 의존성을 확인하지 않은 단일 애니메이션만 가져와 해결하려 하지 않는다.
- 같은 이름의 기존 타겟 에셋을 덮어쓰지 않는다.
