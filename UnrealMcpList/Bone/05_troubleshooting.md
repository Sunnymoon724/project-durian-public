# 5. 이슈 대응 가이드

## 1. 어깨가 올라가거나 팔이 벌어짐·비틀림

**우선 원인 후보**: 소스·타겟의 A/T 기준 자세 차이, 쇄골·상완 회전 차이, 척추 체인 길이 차이, Retarget Pose 부정확.

**확인 및 해결**:

1. 타겟 원본 기준 자세와 Retargeter 미리보기를 나란히 비교한다.
2. 새 Target Retarget Pose를 만들고 clavicle → upperarm → lowerarm 순서로 소폭 보정한다.
3. Idle로 확인한 뒤 Walk/Run에서도 같은 문제가 없는지 확인한다.
4. 개선되지 않으면 arm/spine 체인의 시작·끝 뼈와 Rotation 설정을 검토한다.

**대안**: 정상 결과가 나오는 다른 Retargeter가 있다면 체인 구성과 Pose를 비교해 차이만 이식한다. 무작정 `Run IK Rig`를 끄는 것은 해결책으로 확정하지 않는다.

## 2. 손·발 IK Goal 없음 경고

**의미**: Retarget Chain이 존재하지 않는 IK Goal을 참조할 수 있다.

**대응**: 같은 경고가 정상 결과에도 나타나는지 먼저 비교한다. 정상 결과가 있다면 단독 원인으로 보지 않는다. 실제 손·발 변형이 있거나 경고를 제거해야 할 때만 IK Rig의 Goal과 Chain 설정을 일치시킨다.

## 3. Animation Sequence가 재생되지 않음 또는 참조 누락

**원인 후보**: Skeleton 불일치, Migrate 의존성 누락, Blend Space 샘플 오류, Notify/Curve 누락.

**대응**:

1. Sequence의 Skeleton과 Preview Mesh를 확인한다.
2. Blend Space 내부 모든 샘플 참조를 확인한다.
3. Reference Viewer로 빠진 Notify, Curve, Control Rig, Pose Asset을 찾는다.
4. 필요한 원본 기준 에셋에서 Migrate를 다시 실행한다.

## 4. AnimBP 컴파일 실패

**원인 후보**: Target Skeleton 불일치, 유효하지 않은 노드 참조, 플러그인·엔진 버전이 다른 샘플 그래프 복제.

**대응**: AnimBP의 Target Skeleton부터 확인한다. Motion Matching, Pose Search, Blend Stack, Mover처럼 의존성이 큰 샘플 전체 그래프는 복사하지 말고, 단순 Blend Space 그래프로 최소 구현을 만든다. 필요한 기능은 플러그인·프로젝트 설정이 준비된 뒤 별도로 도입한다.

## 5. PIE가 시작하지 않음

**원인 후보**: 현재 플레이어와 관계없는 Blueprint·GameMode·샘플 에셋의 컴파일 오류, 엔진 버전 불일치 API.

**대응**:

1. Output Log에서 첫 번째 컴파일 실패 에셋을 찾는다.
2. 플레이어 AnimBP/Blueprint를 개별 컴파일해 오류가 같은지 비교한다.
3. 샘플 에셋이라면 프로젝트에서 실제로 사용되는지 참조를 조사한다.
4. 사용하지 않는 에셋을 삭제하거나 변경하려면 먼저 영향 범위와 복구 방법을 확인하고 별도 승인을 받는다.

## 6. PIE에서 이동·시점 입력이 동작하지 않음

**원인 후보**: Pawn Possess 실패, GameMode/PlayerController 설정 오류, Enhanced Input Mapping Context 미등록, Input Action 타입 불일치, 축 바인딩 누락.

**대응**:

1. 실행 맵의 GameMode, Default Pawn Class, PlayerController Class를 확인한다.
2. PIE에서 실제 Possessed Pawn과 Controller를 확인한다.
3. Enhanced Input 사용 시 Local Player Subsystem에 Mapping Context가 추가되는지 확인한다.
4. Input Action의 Axis 타입과 콜백을 확인한다.
5. 프로젝트가 Legacy Input을 쓰는 경우 Axis Mapping과 컨트롤러 바인딩을 같은 경로로 통일한다.

## 7. 에디터가 모듈 DLL을 잠가 빌드 실패

**대응**: 실행 중인 Unreal Editor, PIE, Live Coding 상태가 바이너리를 점유하는지 확인한다. 작업을 저장한 뒤 에디터를 재시작하거나 프로젝트 정책에 맞는 Live Coding 빌드로 갱신한다. 빌드 성공 전에는 런타임 동작을 검증 완료로 보고하지 않는다.

## 8. Crouch·Traversal 애니메이션은 있지만 재생되지 않음

**원인**: 애니메이션 에셋만 있고 실제 게임플레이 상태·입력·트리거가 없는 경우.

**대응**: Crouch는 Character Movement의 상태와 AnimBP 변수를 연결한다. Slide/Vault/Mantle/Hurdle/Climb은 트리거 조건, 이동 제어, 충돌 처리, Montage 또는 상태 전환을 하나의 기능 단위로 구현하고 검증한다.

## 판단이 어려울 때

- 바꾸기 전에 현재 에셋과 참조 관계를 캡처·기록한다.
- 최소 재현 에셋(Idle 하나, Retargeter 하나)으로 문제를 줄인다.
- 변경 하나당 결과 하나를 비교한다.
- 원인을 확정할 수 없으면 추측으로 대량 변경하지 말고, 증상·로그·비교 결과를 남겨 다음 점검 가설을 제시한다.
