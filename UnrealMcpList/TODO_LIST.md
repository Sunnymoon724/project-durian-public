# Unreal MCP VFX TODO 목록

[READ_BEFORE_TASK.md](READ_BEFORE_TASK.md)의 순서를 따른다. 묶음 제목은 작업이 아니다. 위에서부터 첫 번째 `TODO` 하위 항목 하나만 처리한다.

상태: `TODO` / `KEEP` / `DONE` / `FAILED`

## 진행 현황 (2026-09-19 기준)

- 유지 항목: `0-2` `M_PP_AbilityHighlight` — 현재 구현을 유지하며, 명시적 재개 전까지 검증·수정하지 않는다.
- TODO 재개 대상: `1-7`, `2-3`~`2-6`, `2-8`~`2-9`, `3-1`, `3-3`~`3-9` — 아래 재시도 메모와 검증 한계를 참고해 보강 후 진행한다.
- 다음 작업: `1-7` 자력 모드 통합 확인
- 자력 기능 코드 상태: `UPhysicsHandleComponent` 기반 잡기·놓기, 카메라 중앙 `ECC_Visibility` Trace, `UMagnetTargetComponent`와 물리 시뮬레이션 검사, 장애물 Sweep, 거리 조절, 취소·재시작 초기화가 구현되어 있다.
- 자력 기능 미검증: 실제 플레이에서 조준 대상 판별, 범위·시야·비대상 제외, 이전 대상 정보 정리, 잡기→이동→놓기 반복 동작을 아직 확인하지 않았다.

## 0. 공용 기반

### 0-2. `/Game/Resources/VFX/_Shared/PP/M_PP_AbilityHighlight`

- 상태: `KEEP`
- 작업: `ActiveStencilValue`로 대상을 고르는 공용 필·1~2 px 외곽선 포스트 프로세스 머티리얼을 만든다. 비대상은 `PostProcessInput0`를 유지한다.
- 완료: 선택한 Stencil 대상만 강조된다.
- 재시도 메모: 공용 Post Process 머티리얼을 생성·컴파일·저장했으나, `ActiveStencilValue` 파라미터가 Stencil 비교값에 연결되지 않았고 선택 대상의 1~2 px 외곽선 결과를 화면에서 검증할 수 없었음.
- 재시도 보강: `CustomStencil == ActiveStencilValue` 비교용 `If` 그래프를 추가하고, 비대상은 `PostProcessInput0`, 일치 대상은 강조 색을 출력하도록 연결했다. MCP 재컴파일·저장까지 완료했으며 PIE에서 Stencil 대상과 외곽선 결과를 확인해야 `DONE`으로 전환한다.
- 재검증 결과: MCP로 대상 블록이 보이는 위치에서 `TestMap` PIE를 재실행하고 `E` 입력까지 수행했다. 자력 대상 강조 화면은 확인했지만, 게임에서 직접 사용하는 인스턴스가 `MI_PP_MagnetObjectHighlight`이므로 공용 `M_PP_AbilityHighlight`의 단독 Stencil 강조·외곽선 결과는 아직 판정하지 못했다.

## 1. 자력

자력 모드는 단일 PP가 아니라 `월드 시야 → 후보 표시 → 조준/잡기 표시 → 조작 피드백` 순서의 레이어다.
각 하위 작업은 바로 아래 항목만 완료한 뒤 다음 항목으로 진행한다.

### 1-7. 자력 모드 통합 확인

- 상태: `TODO`
- 작업: `TestMap`에서 일반·탐색·조준·조작·취소를 순서대로 확인한다.
- 완료: 후보/조준/조작 상태가 색과 연결선으로 구분되고, 벽 뒤·비대상·모드 종료 뒤에 잘못된 효과가 남지 않는다.
- 재시도 메모: Unreal MCP PIE에서 후보(마젠타)·조준/잡기(노랑)·취소 후 정리는 확인했으나, 현재 고정 3인칭 카메라에서 연결선이 플레이어와 대상에 가려 독립적으로 보이는지 검증할 수 없었다. 런타임 Niagara 컴포넌트 조회 도구도 제공되지 않는다.

## 2. 얼음 생성

### 2-3. `/Game/Resources/VFX/Ice/Materials/M_IceSpawnRing`

- 상태: `TODO`
- 작업: 생성 확정 때 0.4~0.6초 동안 확장·감쇠하는 원형 파동 머티리얼을 만든다.
- 완료: 수면에서 자연스럽게 사라지고 벽 뒤에 보이지 않는다.
- 선행 확인: `Content/Resources/VFX/Cryonis/...`의 실제 에셋 위치와 C++의 `/Game/Resources/VFX/Ice/...` 로드 경로가 일치하는지 확인한다. 리디렉터가 없으면 에셋 경로 또는 C++ 참조를 한 경로로 통일한 뒤 진행한다.
- 진행 방향: 기존 `M_IceSpawnRing`은 유지하고 재생성하지 않는다. `AIcePlacementPreview::PlaySpawnEffect`의 기존 링 표시·0.5초 정리 경로를 활용해, 머티리얼 파라미터와 확장·감쇠 표현만 보강한 뒤 PIE에서 깊이 가림을 검증한다.

### 2-4. `/Game/Resources/VFX/Ice/Materials/MI_IceSpawnRing`

- 상태: `TODO`
- 작업: 링의 색, 폭, 속도를 조절하는 인스턴스를 만든다.
- 완료: 인스턴스 값으로 연출을 바꿀 수 있다.
- 진행 방향: 기존 부모 `M_IceSpawnRing`을 보강하고, 새 `MI_IceSpawnRing`만 생성한다. 색·폭·속도 파라미터를 인스턴스에 노출한 뒤 `AIcePlacementPreview`가 부모 대신 인스턴스를 사용하도록 연결한다.

### 2-5. `/Game/Resources/VFX/Ice/Niagara/NS_IceSpawnSplash`

- 상태: `TODO`
- 작업: 기둥 네 모서리의 물 튐·안개 단발 효과를 만든다.
- 완료: 생성 시 한 번 재생한 뒤 종료된다.
- 보강: 현재 중심에서 한 번 재생하는 경로를 기둥 반폭 기준의 네 모서리 오프셋 재생으로 확장한다. 네 효과 모두 단발 종료하는지 확인한다.
- 진행 방향: 기존 `NS_IceSpawnSplash`를 유지하고 재생성하지 않는다. `AIcePlacementPreview::PlaySpawnEffect`의 기존 단발 재생 경로를 이용해 기둥 네 모서리 배치 파라미터를 보강하고, 생성·종료를 PIE에서 검증한다.

### 2-6. `/Game/Resources/VFX/Ice/Blueprints/BP_IcePlacementPreview`

- 상태: `TODO`
- 작업: 수면 Trace 위치와 유효 상태에 프리뷰·링·물 튐을 연결한다.
- 완료: 취소·재시작 시 프리뷰·링·입자가 남지 않는다.
- 진행 방향: 새 블루프린트를 재생성하지 않는다. 기존 `AIcePlacementPreview`와 `FCryonisAbility`에 수면 대상 Trace, 유효성 판정, 프리뷰·링·스플래시 재생 및 취소 정리 경로가 구현돼 있으므로, `MI_IceSpawnRing` 연결을 보강하고 실제 플레이에서 상태 전환을 검증한다.

### 2-8. `/Game/Resources/VFX/Ice/Materials/M_IceOldestPillarIndicator`

- 상태: `TODO`
- 작업: 다음 생성으로 제거될 가장 오래된 얼음 기둥을 약하게 표시하는 머티리얼 또는 강조 효과를 만든다.
- 완료: 최대 개수일 때 가장 오래된 기둥 하나만 표시된다.
- 갱신 시점: 새 기둥 생성 직전, 최대 개수 초과에 따른 교체 직후, 대상 기둥 수동 제거 직후에 표시 대상을 다시 계산한다. 표시가 필요 없는 상태에서는 이전 강조를 즉시 해제한다.
- 진행 방향: 기존 `FCryonisAbility::SpawnedPillars`의 생성 순서와 `MaxPillarCount` 처리 경로를 사용한다. 새 기둥 생성으로 최대 개수를 초과하기 직전의 첫 번째 기둥 하나에만 기존/신규 강조 머티리얼을 적용하고, 교체 후 다음 첫 번째 기둥으로 표시를 갱신한다.

### 2-9. `/Game/Resources/VFX/Ice/Blueprints/BP_IcePillarVFX`

- 상태: `TODO`
- 작업: 해체 효과와 가장 오래된 기둥 표시를 얼음 기둥의 제거·교체 상태에 연결한다.
- 완료: 제거 사유와 관계없이 해체 효과가 한 번만 재생되고 강조가 갱신된다.
- 검증: 자동 교체와 수동 제거 각각에서 해체 효과의 단발 재생, 이전 강조 해제, 다음 가장 오래된 기둥 강조 갱신을 확인한다.
- 진행 방향: 기존 `BP_IcePillarVFX`와 `NS_IcePillarDissolve`를 유지하고 재생성하지 않는다. `FCryonisAbility::DestroyPillar`의 제거·교체 경로 및 `SpawnedPillars` 순서에 연결해, 제거 사유와 무관하게 해체 효과가 한 번만 재생되고 가장 오래된 기둥 표시가 갱신되는지 PIE에서 검증한다.

## 3. 시간 정지

### 3-1. `/Game/Resources/VFX/Stasis/PP/MI_PP_StasisHighlight`

- 상태: `TODO`
- 작업: 공용 강조 머티리얼의 시간 정지 인스턴스를 만든다. Stencil `2`(조준)와 `3`(정지)을 금색 계열로 구분한다.
- 완료: 조준 대상과 정지 대상이 구분된다.
- 재시도 메모: 금색 `MI_PP_StasisHighlight` 인스턴스는 생성·저장했으나, 부모 `M_PP_AbilityVision`에는 Stencil 2/3 분기 파라미터가 없고 시간 정지 대상 상태도 없어 두 대상을 구분해 연결·검증할 수 없음.

### 3-3. `/Game/Resources/VFX/Stasis/Materials/M_StasisTargetMarker`

- 상태: `TODO`
- 작업: 청록/금색 삼각 마커의 알파 마스크·Emissive 머티리얼을 만든다.
- 완료: 거리와 가림 상태에서도 읽을 수 있다.
- 재시도 메모: 청록 Emissive·투명도와 UV 기반 삼각 알파 마스크 머티리얼은 생성·재컴파일했으나, 프로젝트에 시간 정지 대상/마커 배치 구현이 없어 거리·가림 상태에서의 가독성을 Unreal MCP로 검증할 수 없음.

### 3-4. `/Game/Resources/VFX/Stasis/Materials/MI_StasisTargetMarker`

- 상태: `TODO`
- 작업: 마커의 색, 발광, 회전값을 조절하는 인스턴스를 만든다.
- 완료: 조준은 청록 회전, 정지는 금색 고정으로 구분된다.
- 재시도 메모: 금색 `MI_StasisTargetMarker` 인스턴스는 생성·저장했으나, 부모 머티리얼에는 색·투명도만 노출되어 있고 발광 강도·회전 파라미터 및 조준/정지 상태 연결이 없어 요구한 두 상태를 구분·검증할 수 없음.

### 3-5. `/Game/Resources/VFX/Stasis/Niagara/NS_StasisLockPulse`

- 상태: `TODO`
- 작업: 정지 확정·해제 때 재생할 금색 링·방사형 입자 단발 효과를 만든다.
- 완료: 이벤트마다 한 번만 재생된다.
- 재시도 메모: 금색 단발 Niagara 시스템은 생성·컴파일했으나, 현재 시스템에는 정지 확정·해제 이벤트와 링 메시/렌더러 구성이 없어 이벤트별 링·방사형 효과를 연결·검증할 수 없음.

### 3-6. `/Game/Resources/VFX/Stasis/Blueprints/BP_StasisTargetVFX`

- 상태: `TODO`
- 작업: 대상 Bounding Box 위의 삼각 마커 3개, 펄스, Stencil 상태를 연결한다.
- 완료: 대상 제거·취소·재시작 때 마커·Stencil·Niagara가 정리된다.
- 재시도 메모: `BP_StasisTargetVFX` Niagara 래퍼는 생성·컴파일했으나, 프로젝트에 시간 정지 대상, Bounding Box, Stencil 2/3 상태 및 취소·재시작 이벤트가 없어 마커·펄스·정리 동작을 연결·검증할 수 없음.

### 3-7. `/Game/Resources/VFX/Stasis/Niagara/NS_StasisChargeFeedback`

- 상태: `TODO`
- 작업: 축적된 힘의 방향·강도를 보여주는 월드 공간 화살표, 누적 링 또는 잔상 효과를 만든다.
- 완료: 축적 벡터에 따라 갱신되고 해제·취소 시 사라진다.
- 재시도 메모: 프로젝트에 시간 정지 축적 벡터·강도 데이터와 해제·취소 이벤트가 없어 Niagara 사용자 변수를 갱신하거나 해제 시 정리하는 런타임 경로를 만들고 Unreal MCP로 검증할 수 없음.

### 3-8. `/Game/Resources/VFX/Stasis/Niagara/NS_StasisHoldAmbient`

- 상태: `TODO`
- 작업: 정지 중 대상 주변에 유지되는 적은 수의 금색 점 입자 효과를 만든다.
- 완료: 정지 상태에서만 유지되고 해제·취소·대상 제거 시 종료된다.
- 재시도 메모: 저밀도 금색 유지 Niagara 시스템은 생성·컴파일했으나, 프로젝트에 시간 정지 시작·해제·취소·대상 제거 이벤트가 없어 상태 한정 재생과 종료를 연결·검증할 수 없음.

### 3-9. `/Game/Resources/VFX/Stasis/Niagara/NS_StasisExpiryWarning`

- 상태: `TODO`
- 작업: 종료 직전 마커 점멸과 짧은 금색 펄스를 재생하는 경고 효과를 만든다.
- 완료: 종료 직전에만 재생되고 남지 않는다.
- 재시도 메모: 단발 금색 경고 Niagara 시스템은 생성·컴파일했으나, 시간 정지 종료 시점·마커 점멸 상태가 없어 종료 직전에만 실행되는지 연결·검증할 수 없음.

## 4. 후보 에셋 검토

### 4-2. 추가 빔·전류 에셋

- 상태: `TODO`
- 작업: `StylizedToonBeam01` 외의 빔·전류 연출 후보를 검토하고 자석 잡기 상태에 맞는 에셋을 선택·연결한다.
- 완료: 선택한 에셋이 잡기 상태에서만 재생되고 종료 시 정리된다.

## 5. 최종 VFX 테스트

### 5-1. 능력 전환·가림 테스트

- 상태: `TODO`
- 작업: 자력·얼음·시간 정지를 연속 전환하고 취소·재시작·벽 가림을 확인한다.
- 완료: 이전 PP·마커·파티클·Stencil이 남지 않고, 가려진 효과가 보이지 않는다.

### 5-2. 렌더링 우선순위 테스트

- 상태: `TODO`
- 작업: 카툰 기본 PP와 능력 PP의 Blendable Location·Priority를 확인한다.
- 완료: 화면 필터와 대상 강조가 서로 덮어쓰지 않는다.

### 5-3. 생성 에셋·Git 변경 검토

- 상태: `TODO`
- 작업: 이번 목록으로 생성한 에셋·관련 코드·문서만 검토한다.
- 완료: 캐시, 빌드 산출물, 개인 설정이 커밋에 포함되지 않는다.
