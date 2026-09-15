# Unreal MCP VFX TODO 목록

[READ_BEFORE_TASK.md](READ_BEFORE_TASK.md)의 순서를 따른다. 묶음 제목은 작업이 아니다. 위에서부터 첫 번째 `TODO` 하위 항목 하나만 처리한다.

상태: `TODO` / `DONE` / `FAILED`

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

- 상태: `TODO`
- 작업: 틴트, 채도 감소, 대각선 스크린 패턴 파라미터가 있는 공용 포스트 프로세스 머티리얼을 만든다.
- 완료: 인스턴스에서 틴트와 패턴 강도를 조절할 수 있다.

## 1. 자력

### 1-1. `/Game/Resources/VFX/Magnet/PP/MI_PP_MagnetHighlight`

- 상태: `TODO`
- 작업: 공용 강조 머티리얼의 자석 인스턴스를 만들고 Stencil `1`을 노란색으로 설정한다.
- 완료: 자석 가능한 대상만 노란 필·외곽선으로 보인다.

### 1-2. `/Game/Resources/VFX/Magnet/PP/MI_PP_MagnetVision`

- 상태: `TODO`
- 작업: 공용 시야 머티리얼의 자석 인스턴스를 만들고 청록/마젠타 틴트와 대각선 패턴을 설정한다.
- 완료: 배경 필터와 노란 강조가 독립적으로 조절된다.

### 1-3. `/Game/Resources/VFX/Magnet/Niagara/NS_MagnetScanPulse`

- 상태: `TODO`
- 작업: 자력 탐지 시작 때 한 번 퍼지는 청록/마젠타 스캔 펄스를 만든다.
- 완료: 0.6~1.0초 안에 한 번 재생하고 종료된다.

### 1-4. `/Game/Resources/VFX/Magnet/Blueprints/BP_MagnetVFXController`

- 상태: `TODO`
- 작업: 자석 상태에 맞춰 Stencil `1`, 자석 PP 인스턴스, 스캔 펄스를 전환하는 VFX Controller를 만든다.
- 완료: 탐지·조작·취소·재시작 뒤 효과가 남지 않는다.

### 1-5. `/Game/Resources/VFX/Magnet/Beams/StylizedToonBeam01/*`

- 상태: `TODO`
- 작업: 기존 `/Game/StylizedToonBeam01`의 빔 에셋을 위 경로로 옮긴 뒤, 자석을 잡은 상태에서만 사용하도록 연결한다.
- 완료: `MagnetControl`에서만 재생되고 종료 시 꺼진다.

## 2. 얼음 생성

### 2-1. `/Game/Resources/VFX/Ice/Materials/M_IceSpawnPreview`

- 상태: `TODO`
- 작업: 청록/적색 둥근 사각형, 대각선 스트라이프, 약한 펄스 프리뷰 머티리얼을 만든다.
- 완료: 생성 가능/불가 상태가 분명히 구분된다.

### 2-2. `/Game/Resources/VFX/Ice/Materials/MI_IceSpawnPreview`

- 상태: `TODO`
- 작업: 프리뷰 색, 발광, 투명도를 조절하는 인스턴스를 만든다.
- 완료: 인스턴스 파라미터로 가능/불가 상태를 바꾼다.

### 2-3. `/Game/Resources/VFX/Ice/Materials/M_IceSpawnRing`

- 상태: `TODO`
- 작업: 생성 확정 때 0.4~0.6초 동안 확장·감쇠하는 원형 파동 머티리얼을 만든다.
- 완료: 수면에서 자연스럽게 사라지고 벽 뒤에 보이지 않는다.

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
