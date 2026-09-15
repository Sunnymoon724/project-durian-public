# 4. 검증 체크리스트

## 원칙

리타게팅, AnimBP, 플레이어 Blueprint, PIE를 분리해 검증한다. PIE가 실패했다고 AnimBP가 실패한 것으로 단정하지 않고, Output Log의 최초 컴파일 오류 에셋을 확인한다.

## A. 리타게팅 결과 검증

- [ ] 결과 Animation Sequence의 Skeleton이 타겟 Skeleton이다.
- [ ] Preview Mesh가 타겟 Skeletal Mesh다.
- [ ] Idle에서 어깨·팔·손의 기준 자세가 자연스럽다.
- [ ] Walk/Run/Sprint에서 발이 비정상적으로 회전하거나 크게 미끄러지지 않는다.
- [ ] Jump Start, Fall, Land에서 관절이 찌그러지거나 튀지 않는다.
- [ ] Root Bone과 Root Motion 설정이 의도와 일치한다.

## B. 에셋 참조 검증

- [ ] Blend Space의 모든 샘플이 유효한 타겟 애니메이션을 참조한다.
- [ ] Anim Notify, Curve, Control Rig, Pose Asset 참조가 누락되지 않았다.
- [ ] 머티리얼, 텍스처, Physics Asset 참조가 정상이다.
- [ ] Reference Viewer에 원본 프로젝트 전용 경로나 불필요한 데모 의존성이 남지 않았다.

## C. 컴파일 검증

- [ ] Retargeter, Blend Space, AnimBP, 플레이어 Blueprint를 각각 Compile한다.
- [ ] 새 경고·오류의 에셋과 메시지를 기록한다.
- [ ] 기존 오류와 새 오류를 구분한다.
- [ ] 변경한 에셋만 의도적으로 저장한다.

## D. PIE 검증

- [ ] PIE가 시작된다.
- [ ] 플레이어가 올바른 Pawn으로 Spawn/Possess된다.
- [ ] 입력이 플레이어 컨트롤러와 캐릭터까지 전달된다.
- [ ] 정지, 걷기, 달리기, 전력질주가 실제 속도에 맞게 전환된다.
- [ ] 점프 시작, 공중, 착지가 게임플레이 상태와 일치한다.
- [ ] 충돌, 캡슐, Physics Asset이 의도대로 동작한다.

## 기록 형식

문제 재현 시 다음만 짧게 남긴다.

```text
날짜:
소스 / 타겟:
변경한 에셋:
재현 절차:
관찰 결과:
Output Log의 최초 관련 오류:
시도한 해결:
결과:
```

이 기록이 있으면 다음 작업자나 AI가 문제의 원인 후보와 이미 시도한 조치를 구분할 수 있다.
