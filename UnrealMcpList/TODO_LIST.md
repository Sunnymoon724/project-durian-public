# Unreal MCP VFX TODO 목록

[READ_BEFORE_TASK.md](READ_BEFORE_TASK.md)의 순서를 따른다. 묶음 제목은 작업이 아니다. 위에서부터 첫 번째 `TODO` 하위 항목 하나만 처리한다.

상태: `TODO` / `DONE` / `FAILED`

## 진행 현황

- 완료: `0-1`, `0-3`, `1-1`, `1-2`, `1-3`, `2-1`, `2-2`
- 생성·컴파일·저장 완료 자산: `M_PP_AbilityVision`, `MI_PP_MagnetVision`, `M_PP_MagnetWorldScan`, `MI_PP_MagnetWorldScan`, `M_PP_MagnetObjectHighlight`, `MI_PP_MagnetObjectHighlight`, `NS_MagnetEnterPulse`, `M_IceSpawnPreview`, `MI_IceSpawnPreview`, `M_IceSpawnRing`
- 실패/보류: `0-2`, `2-3` — 각 항목에 원인을 기록했다.
- 다음 작업: `1-4` `NS_MagnetEnterPulse` 실제 재생 검증

## 0. 공용 기반

### 0-1. Project Settings — Custom Depth-Stencil

- 상태: `DONE`
- 작업: `Custom Depth-Stencil Pass`를 `Enabled with Stencil`로 설정한다.
- 완료: 재시작 후 Custom Stencil 값을 읽을 수 있다.

### 0-2. `/Game/Resources/VFX/_Shared/PP/M_PP_AbilityHighlight`

- 상태: `FAILED`
- 작업: `ActiveStencilValue`로 대상을 고르는 공용 필·1~2 px 외곽선 포스트 프로세스 머티리얼을 만든다. 비대상은 `PostProcessInput0`를 유지한다.
- 완료: 선택한 Stencil 대상만 강조된다.
- 실패 사유: Unreal MCP만으로는 포스트 프로세스 적용 상태에서 선택 Stencil 대상의 외곽선 결과를 검증할 수 없음.

### 0-3. `/Game/Resources/VFX/_Shared/PP/M_PP_AbilityVision`

- 상태: `DONE`
- 작업: 틴트, 채도 감소, 대각선 스크린 패턴 파라미터가 있는 공용 포스트 프로세스 머티리얼을 만든다.
- 완료: 인스턴스에서 틴트와 패턴 강도를 조절할 수 있다.

## 1. 자력

자력 모드는 단일 PP가 아니라 `월드 시야 → 후보 표시 → 조준/잡기 표시 → 조작 피드백` 순서의 레이어다.
각 하위 작업은 바로 아래 항목만 완료한 뒤 다음 항목으로 진행한다.

### 1-1. 기존 배경 필터 유지 — `MI_PP_MagnetVision`

- 상태: `DONE`
- 에셋: `/Game/Resources/VFX/Magnet/PP/MI_PP_MagnetVision`
- 작업: 기존 인스턴스는 삭제하거나 교체하지 않고, 자력 모드의 약한 배경 채도 감소·틴트 기반으로 유지한다.
- 범위: 후보·조준 대상 색, 구조물 윤곽, 표면 스캔은 이 에셋에 억지로 추가하지 않는다.
- 확인: 자력 모드 on/off에서 카메라 Blendable 가중치를 조절할 수 있다.

### 1-2. 월드 구조물 스캔 — `M_PP_MagnetWorldScan`, `MI_PP_MagnetWorldScan`

- 상태: `DONE`
- 경로: `/Game/Resources/VFX/Magnet/PP/`
- 작업: 자력 모드에만 적용되는 별도 Post Process 머티리얼과 인스턴스를 만든다.
  - `SceneDepth`와 `WorldNormal` 변화로 벽·기둥·바닥의 모서리에 붉은 윤곽을 만든다.
  - 월드 좌표 기반 대각선 해칭을 만들어 바닥뿐 아니라 벽·기둥·경사면에도 공간에 붙어 보이게 한다.
  - 배경은 약하게 어둡게 하되, 대상 강조 색을 덮지 않게 한다.
- 완료: PIE에서 기둥 윤곽과 표면 해칭이 화면 고정 패턴처럼 보이지 않고 표면을 따라 보인다.
- 구현: 기존 시야 PP를 복제해 `Absolute World Position` 기반 해칭으로 바꾸고, `WorldNormal`의 화면 미분값을 더해 붉은 구조물 에지를 만들었다. 해칭·에지는 Stencil `4` 구조물에만 적용하고, Stencil `3` 플레이어는 원본 화면으로 제외한다. `MI_PP_MagnetWorldScan`에서 해칭·에지 색과 강도를 조절한다.

### 1-3. 자석 후보·조준 대상 강조 — `M_PP_MagnetObjectHighlight`, `MI_PP_MagnetObjectHighlight`

- 상태: `DONE`
- 경로: `/Game/Resources/VFX/Magnet/PP/`
- 작업: Custom Depth/Stencil을 읽는 대상 전용 Post Process 머티리얼과 인스턴스를 만든다.
  - Stencil `1`: 자석 가능한 후보 전체를 마젠타/빨강 필과 얇은 윤곽으로 표시한다.
  - Stencil `2`: 현재 조준하거나 잡은 하나를 노랑 필과 더 밝은 윤곽으로 표시한다.
  - Stencil `0`과 비대상 화면은 원래 색을 유지한다.
- 완료: PIE에서 후보는 마젠타/빨강, 조준 대상 하나만 노랑으로 전환된다.
- 검증: 테스트 액터에서 Stencil `1`의 마젠타 필, Stencil `2`의 노랑 필과 흰색 외곽선 전환을 뷰포트에서 확인했다.

### 1-4. 자력 모드 진입 펄스 — `NS_MagnetEnterPulse`

- 상태: `TODO`
- 경로: `/Game/Resources/VFX/Magnet/Niagara/`
- 작업: 자력 모드 진입 때 플레이어 위치에서 한 번만 재생되는 링 또는 짧은 반구형 스캔을 만든다.
- 기준: 청록·마젠타 계열, `0.6~1.0초`, 루프 없음.
- 완료: 진입 시 1회 재생되고 탐색 중에는 반복되지 않는다.
- 현재: 빈 Niagara System에 System/Emitter/Particle State, 28개 Burst, 원형 Shape Location, 바깥 방향 속도, `0.8초` 수명, 청록 색, Sprite Renderer를 구성하고 컴파일·저장했다. `UAbilityEffectComponent`가 `Normal → MagnetTargeting`에서 이 시스템을 한 번 재생하도록 연결했으며, 실제 화면 확인만 남아 있다.

### 1-5. 잡기 연결선 — `NS_MagnetHoldLink`

- 상태: `TODO`
- 경로: `/Game/Resources/VFX/Magnet/Niagara/`
- 작업: `StylizedToonBeam01`을 참고하거나 재사용해, 손 또는 조준점에서 잡은 대상까지 이어지는 자력 연결선을 만든다.
- 기준: 거리 변화에 맞춰 길이가 갱신되고, `MagnetControl`에서만 보인다.
- 완료: 대상 해제·취소·파괴 시 즉시 사라지고 탐색 상태에는 보이지 않는다.

### 1-6. 자력 VFX 상태 연결 — `UAbilityEffectComponent` 및 자력 대상 코드

- 상태: `TODO`
- 작업: 현재 만든 `UAbilityEffectComponent`를 자력 상태 흐름에 맞춰 완성한다.
  - `Normal → MagnetTargeting`: 배경 필터·월드 스캔·후보 표시를 켜고 진입 펄스를 1회 재생한다.
  - 조준 대상 변경: 이전 대상은 Stencil `1`, 현재 대상은 Stencil `2`로 갱신한다.
  - `MagnetTargeting → MagnetControl`: 잡은 대상의 노랑 표시와 연결선을 켠다.
  - 취소·해제·재시작·대상 파괴: PP, Custom Depth/Stencil, 연결선, 펄스를 모두 정리한다.
- 완료: 상태를 반복 전환해도 이전 효과나 Stencil 값이 남지 않는다.
- 현재: `UAbilityEffectComponent`가 월드 스캔과 대상 강조 PP를 동시에 켜며, `UMagnetTargetComponent` 소유 물체를 Stencil `1` 후보로 초기화한다. 조준한 컴포넌트만 Stencil `2`로 바꾸고, 조준 해제 시 `1`로 복원한다. 게임 내 반복 전환 검증은 남아 있다.
- 구조물 지정: `UMagnetScanStructureComponent`를 바닥·벽·기둥·난간·고정 프레임에 추가한다. 이 컴포넌트는 소유 액터의 Primitive를 Stencil `4`로 지정한다. 캐릭터·적·일반 소품·자력 후보에는 붙이지 않는다.

### 1-7. 자력 모드 통합 확인

- 상태: `TODO`
- 작업: `TestMap`에서 일반·탐색·조준·조작·취소를 순서대로 확인한다.
- 완료: 후보/조준/조작 상태가 색과 연결선으로 구분되고, 벽 뒤·비대상·모드 종료 뒤에 잘못된 효과가 남지 않는다.

## 2. 얼음 생성

### 2-1. `/Game/Resources/VFX/Ice/Materials/M_IceSpawnPreview`

- 상태: `DONE`
- 작업: 청록/적색 둥근 사각형, 대각선 스트라이프, 약한 펄스 프리뷰 머티리얼을 만든다.
- 완료: 생성 가능/불가 상태가 분명히 구분된다.
- 구현: `ValidColor`(청록), `InvalidColor`(적색), `IsValid`, `StripeScale`, `StripeIntensity`, `PulseSpeed` 파라미터로 상태색·대각선 스트라이프·시간 기반 펄스를 구성했다.

### 2-2. `/Game/Resources/VFX/Ice/Materials/MI_IceSpawnPreview`

- 상태: `DONE`
- 작업: 프리뷰 색, 발광, 투명도를 조절하는 인스턴스를 만든다.
- 완료: 인스턴스 파라미터로 가능/불가 상태를 바꾼다.
- 구현: `IsValid`, `EmissionIntensity`, `Opacity`, `StripeIntensity` 오버라이드를 설정했고 각 유효값을 조회해 검증했다.

### 2-3. `/Game/Resources/VFX/Ice/Materials/M_IceSpawnRing`

- 상태: `FAILED`
- 작업: 생성 확정 때 0.4~0.6초 동안 확장·감쇠하는 원형 파동 머티리얼을 만든다.
- 완료: 수면에서 자연스럽게 사라지고 벽 뒤에 보이지 않는다.
- 실패 사유: 원형 링 머티리얼은 생성·컴파일·저장했으나, 시간 제한 확장·감쇠와 실제 수면/벽 깊이 테스트는 연결할 Niagara 또는 테스트 액터가 없어 Unreal MCP만으로 검증할 수 없음.

### 2-4. `/Game/Resources/VFX/Ice/Materials/MI_IceSpawnRing`

- 상태: `TODO`
- 작업: 링의 색, 폭, 속도를 조절하는 인스턴스를 만든다.
- 완료: 인스턴스 값으로 연출을 바꿀 수 있다.

### 2-5. `/Game/Resources/VFX/Ice/Niagara/NS_IceSpawnSplash`

- 상태: `TODO`
- 작업: 기둥 네 모서리의 물 튐·안개 단발 효과를 만든다.
- 완료: 생성 시 한 번 재생한 뒤 종료된다.

### 2-6. `/Game/Resources/VFX/Ice/Blueprints/BP_IcePlacementPreview`

- 상태: `TODO`
- 작업: 수면 Trace 위치와 유효 상태에 프리뷰·링·물 튐을 연결한다.
- 완료: 취소·재시작 시 프리뷰·링·입자가 남지 않는다.

### 2-7. `/Game/Resources/VFX/Ice/Niagara/NS_IcePillarDissolve`

- 상태: `TODO`
- 작업: 얼음 기둥 제거·교체 때 재생할 물안개·결정 파편 해체 효과를 만든다.
- 완료: 제거 위치에서 한 번 재생한 뒤 종료된다.

### 2-8. `/Game/Resources/VFX/Ice/Materials/M_IceOldestPillarIndicator`

- 상태: `TODO`
- 작업: 다음 생성으로 제거될 가장 오래된 얼음 기둥을 약하게 표시하는 머티리얼 또는 강조 효과를 만든다.
- 완료: 최대 개수일 때 가장 오래된 기둥 하나만 표시된다.

### 2-9. `/Game/Resources/VFX/Ice/Blueprints/BP_IcePillarVFX`

- 상태: `TODO`
- 작업: 해체 효과와 가장 오래된 기둥 표시를 얼음 기둥의 제거·교체 상태에 연결한다.
- 완료: 제거 사유와 관계없이 해체 효과가 한 번만 재생되고 강조가 갱신된다.

## 3. 시간 정지

### 3-1. `/Game/Resources/VFX/Stasis/PP/MI_PP_StasisHighlight`

- 상태: `TODO`
- 작업: 공용 강조 머티리얼의 시간 정지 인스턴스를 만든다. Stencil `2`(조준)와 `3`(정지)을 금색 계열로 구분한다.
- 완료: 조준 대상과 정지 대상이 구분된다.

### 3-2. `/Game/Resources/VFX/Stasis/PP/MI_PP_StasisVision`

- 상태: `TODO`
- 작업: 공용 시야 머티리얼의 시간 정지 인스턴스를 만들고 약한 황금 틴트를 설정한다.
- 완료: 자석 필터와 독립적으로 조절된다.

### 3-3. `/Game/Resources/VFX/Stasis/Materials/M_StasisTargetMarker`

- 상태: `TODO`
- 작업: 청록/금색 삼각 마커의 알파 마스크·Emissive 머티리얼을 만든다.
- 완료: 거리와 가림 상태에서도 읽을 수 있다.

### 3-4. `/Game/Resources/VFX/Stasis/Materials/MI_StasisTargetMarker`

- 상태: `TODO`
- 작업: 마커의 색, 발광, 회전값을 조절하는 인스턴스를 만든다.
- 완료: 조준은 청록 회전, 정지는 금색 고정으로 구분된다.

### 3-5. `/Game/Resources/VFX/Stasis/Niagara/NS_StasisLockPulse`

- 상태: `TODO`
- 작업: 정지 확정·해제 때 재생할 금색 링·방사형 입자 단발 효과를 만든다.
- 완료: 이벤트마다 한 번만 재생된다.

### 3-6. `/Game/Resources/VFX/Stasis/Blueprints/BP_StasisTargetVFX`

- 상태: `TODO`
- 작업: 대상 Bounding Box 위의 삼각 마커 3개, 펄스, Stencil 상태를 연결한다.
- 완료: 대상 제거·취소·재시작 때 마커·Stencil·Niagara가 정리된다.

### 3-7. `/Game/Resources/VFX/Stasis/Niagara/NS_StasisChargeFeedback`

- 상태: `TODO`
- 작업: 축적된 힘의 방향·강도를 보여주는 월드 공간 화살표, 누적 링 또는 잔상 효과를 만든다.
- 완료: 축적 벡터에 따라 갱신되고 해제·취소 시 사라진다.

### 3-8. `/Game/Resources/VFX/Stasis/Niagara/NS_StasisHoldAmbient`

- 상태: `TODO`
- 작업: 정지 중 대상 주변에 유지되는 적은 수의 금색 점 입자 효과를 만든다.
- 완료: 정지 상태에서만 유지되고 해제·취소·대상 제거 시 종료된다.

### 3-9. `/Game/Resources/VFX/Stasis/Niagara/NS_StasisExpiryWarning`

- 상태: `TODO`
- 작업: 종료 직전 마커 점멸과 짧은 금색 펄스를 재생하는 경고 효과를 만든다.
- 완료: 종료 직전에만 재생되고 남지 않는다.

## 4. 후보 에셋 검토

### 4-1. 카툰 렌더링 외부 에셋

- 상태: `TODO`
- 작업: 카툰 렌더링 후보의 엔진 버전·라이선스·도입 범위·대체안을 검토한다.
- 완료: 도입 후보 하나 또는 미도입 결정과 근거가 정리된다. 구매·다운로드·프로젝트 반영은 사용자 승인 후에만 한다.

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
