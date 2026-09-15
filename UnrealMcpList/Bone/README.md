# 플레이어 애니메이션 리타게팅 작업 안내

이 폴더는 Unreal 프로젝트에서 임의의 **소스 캐릭터** 애니메이션을 임의의 **타겟 캐릭터**에 적용하는 표준 작업 절차다. 특정 캐릭터, 에셋 경로, 완료 상태를 전제하지 않는다.

## 목표

소스 캐릭터의 필요한 애니메이션만 타겟 스켈레톤으로 리타게팅하고, 플레이어 Animation Blueprint(이하 AnimBP)에 안전하게 연결한다. 기본 이동을 먼저 검증한 뒤 점프, 웅크리기, 트래버설을 단계적으로 확장한다.

## 시작 전 사용자에게 받아야 할 정보

작업을 시작하기 전에 아래를 확인한다. 하나라도 불명확하면 추측하여 변경하지 않는다.

| 항목 | 필요한 내용 |
| --- | --- |
| 소스 | 소스 Skeletal Mesh, Skeleton, 애니메이션 루트 또는 사용할 샘플 |
| 타겟 | 타겟 Skeletal Mesh, Skeleton, 적용할 플레이어 Blueprint |
| 적용 범위 | 기본 이동만인지, Jump/Crouch/Traversal까지인지 |
| 기존 자산 | 재사용 가능한 IK Rig, IK Retargeter, AnimBP, Blend Space 유무 |
| 저장 위치 | 리타게팅 결과와 신규 AnimBP를 둘 전용 폴더 |
| 검증 환경 | 실행 맵, GameMode, 입력 방식, PIE 가능 여부 |

## 권장 작업 흐름

```text
소스·타겟 확정
  → 기존 에셋/의존성 조사
  → IK Rig·Retargeter 준비 및 기준 자세 보정
  → 최소 애니메이션 세트 리타게팅
  → 미리보기 검증
  → Blend Space·AnimBP 연결
  → Blueprint/입력 연결
  → PIE 검증
  → Crouch·전환·Traversal 확장
```

각 단계의 구체적 절차는 다음 문서를 따른다.

- [01_asset_selection_and_migration.md](01_asset_selection_and_migration.md): 소스 애셋 선별과 Migrate
- [02_retargeting_workflow.md](02_retargeting_workflow.md): IK Rig, Retarget Pose, 애니메이션 추출
- [03_player_animbp_integration.md](03_player_animbp_integration.md): Blend Space와 플레이어 AnimBP 연결
- [04_validation_checklist.md](04_validation_checklist.md): 미리보기·컴파일·PIE 검증
- [05_troubleshooting.md](05_troubleshooting.md): 증상별 원인 확인, 해결책과 대안

## 공통 원칙

- 원본 에셋, 기존 플레이어 Blueprint, 기존 AnimBP를 덮어쓰지 않는다.
- `.uasset`을 파일 탐색기로 직접 복사하지 않는다. 프로젝트 간 이동은 Unreal Editor의 `Asset Actions → Migrate`를 사용한다.
- Motion Matching, Pose Search, Mover 등 플러그인·엔진 버전에 민감한 전체 그래프는 그대로 복제하지 않는다. 필요한 Animation Sequence부터 사용한다.
- 리타게팅 결과는 타겟 전용 폴더에 저장하고, 확정된 Retarget Pose로 재생성할 수 있게 소스 목록을 기록한다.
- 한 단계가 컴파일과 미리보기 또는 PIE에서 검증되기 전에는 다음 단계로 확장하지 않는다.
