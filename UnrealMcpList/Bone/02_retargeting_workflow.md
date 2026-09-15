# 2. 리타게팅 작업 절차

## 목적

소스 스켈레톤용 Animation Sequence를 타겟 스켈레톤에 맞게 변환한다. Retarget Pose를 확정하기 전의 결과물은 시험 결과로만 취급한다.

## 준비 확인

1. 소스와 타겟의 Skeletal Mesh, Skeleton을 각각 확인한다.
2. 소스·타겟 IK Rig가 있는지 확인한다. 없으면 각 스켈레톤에 맞는 체인을 정의해 만든다.
3. 소스 IK Rig와 타겟 IK Rig를 연결하는 IK Retargeter가 있는지 확인한다.
4. 기존 Retargeter가 있더라도 Source Mesh, Target Mesh, Source IK Rig, Target IK Rig가 이번 작업과 일치하는지 확인한다.
5. 기존 Retargeter에 `Unsaved` 또는 `*` 표시가 있으면 저장하지 말고 변경 내용을 먼저 판별한다.

## 체인 구성 기준

소스와 타겟 사이의 뼈 수가 달라도 의미가 같은 체인을 맞춘다.

| 체인 | 일반적인 범위 |
| --- | --- |
| Spine | pelvis에서 최상단 spine 또는 chest |
| Left/Right Arm | clavicle에서 hand |
| Left/Right Leg | thigh에서 ball 또는 foot |
| Neck/Head | neck에서 head |

손·발 IK Goal 경고는 별도 점검 항목이다. 경고가 있다고 해서 바로 팔·다리 변형의 원인으로 단정하지 말고, 결과 포즈와 체인 매핑을 함께 비교한다.

## Retarget Pose 보정

서로 다른 기준 자세(A-pose/T-pose), 척추 수, 쇄골·상완 회전 차이는 Retarget Pose로 보정한다.

1. Retargeter를 열고 Source와 Target 미리보기를 함께 표시한다.
2. `Default Pose`를 직접 덮어쓰지 말고 새 Target Retarget Pose를 만든다.
3. Idle 애니메이션 하나를 기준 테스트로 선택한다.
4. 다음 순서로 필요한 뼈만 조금씩 조정한다.
   1. `clavicle_l`, `clavicle_r`
   2. `upperarm_l`, `upperarm_r`
   3. 필요 시 `lowerarm_l`, `lowerarm_r`
   4. 필요 시 spine, neck, thigh, calf
5. 매 조정 뒤 어깨 높이, 팔 길이감, 손 위치, 발 접지를 비교한다.
6. 자연스러운 결과가 반복해서 재현되면 새 Pose를 이름을 붙여 저장한다.

자동 정렬 결과가 과도한 회전이나 팔 벌어짐을 만들면 사용하지 않는다. 수동 보정으로 해결되지 않으면 체인 시작·종료 뼈와 Retargeter의 체인 설정을 다시 확인한다.

## 애니메이션 추출

1. 확정한 Retargeter와 Retarget Pose를 활성화한다.
2. 먼저 최소 세트(Idle, Walk, Run, Sprint, Jump Start, Fall, Land)를 선택한다.
3. 타겟 전용 `Retargeted` 폴더로 내보낸다.
4. 각 결과의 Skeleton과 Preview Mesh가 타겟인지 확인한다.
5. Idle과 Walk를 먼저 미리보기한 뒤, Run/Sprint/Jump를 순서대로 확인한다.

## 재생성 원칙

Retarget Pose, 체인, 소스 애니메이션 설정이 바뀌면 영향을 받는 Animation Sequence를 다시 생성한다. 이전 결과를 최종본으로 남겨두지 말고, 검증된 결과만 AnimBP와 Blend Space에 연결한다.
