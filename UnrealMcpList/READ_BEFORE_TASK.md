# Unreal MCP 작업 절차

이 파일을 읽은 뒤 [TODO_LIST.md](TODO_LIST.md)를 처음부터 끝까지 읽는다.

## 이 목록을 읽는 방법

- `0. 공용 기반`, `1. 자력` 같은 번호는 **작업 묶음 제목**이다. 상태를 갖지 않으며, 직접 완료 처리하지 않는다.
- 실제 작업은 `0-1`, `0-2`, `1-1`처럼 하위 번호가 붙은 항목이다.
- 작업 순서는 파일의 위에서 아래 순서로 고정한다.
- 새 작업을 시작할 때는 파일에서 가장 먼저 나오는 `TODO` 하위 항목 **하나만** 수행한다. 뒤의 `TODO`를 건너뛰지 않는다.
- 상태는 `TODO`, `KEEP`, `DONE`, `FAILED`만 사용한다. `KEEP`은 현재 상태를 유지하고 명시적으로 `TODO`로 되돌리기 전까지 작업하지 않는 항목이다. `IN_PROGRESS`는 사용하지 않는다.

## 세션 시작

1. 사용자가 명시적으로 **“Unreal MCP 작업 시작”**이라고 지시한 경우에만 시작한다.
2. 현재 Git 상태를 확인한다.
3. `codex` 브랜치가 없으면 생성·전환하고, 이미 있으면 해당 브랜치로 전환한다.
4. `TODO_LIST.md`를 읽고, 아래의 재개 규칙을 먼저 적용한다.

## 도구와 진행 원칙

- Unreal 에디터 작업은 Unreal MCP만 사용한다.
- Discord 알림은 `SendDiscordText.bat`을 사용한다. 인자를 메시지로 전달하며, webhook URL은 `Webhook.env`에서 읽는다. URL이나 인증 정보는 문서·로그에 남기지 않는다.
- Discord에는 각 하위 작업의 시작, 의미 있는 검증 단계 도달, 완료 또는 실패를 즉시 영어로 알린다. 사용자가 중간에 확인하지 않아도 현재 진행 항목과 다음 행동을 알 수 있게 짧게 작성한다.
- Discord 전송 도구가 없거나 전송이 차단되면 그 사실만 결과에 기록하고, 에셋 작업·커밋·다음 TODO 진행은 중단하지 않는다.
- 사용자 확인을 기다리지 않고 이 절차에 따라 다음 작업을 계속 처리한다.

## 중단된 세션 재개

1. `DONE`인 항목은 완료된 작업이다. 다시 만들거나 수정하지 않는다.
2. `DONE` 항목의 에셋·TODO 변경이 아직 커밋되지 않았다면, 그 변경만 먼저 검토·커밋하고 `SendDiscordText.bat`으로 Discord 완료 알림을 보낸다. 그 뒤 다음 `TODO`를 찾는다.
3. 가장 앞선 미완료 항목이 `TODO`라면, 이전 세션이 그 작업 도중 끝난 것이다. 현재 에셋과 Git 변경을 확인한 뒤, 그 항목의 완료 기준을 만족하도록 이어서 작업한다.
4. `FAILED` 항목은 사유와 커밋을 확인한 뒤 건너뛰고, 다음 `TODO` 하위 항목을 이어서 처리한다.
5. `KEEP` 항목은 에셋·코드를 수정하거나 검증하지 않고 건너뛴다. 사용자가 명시적으로 재개를 지시한 경우에만 `TODO`로 되돌린다.

## 하위 작업 하나의 처리

1. 위에서부터 첫 번째 `TODO` 하위 항목을 선택한다.
2. 그 항목에 적힌 작업과 완료 기준만 수행한다.
3. 완료 기준을 확인한 뒤 해당 항목의 상태를 `DONE`으로 바꾼다.
4. 해당 작업의 에셋·관련 코드·TODO 변경만 명시적으로 stage하여 하나의 커밋으로 만든다.
5. 커밋 후 `SendDiscordText.bat`으로 해당 작업의 완료 알림을 보내고 다음 첫 `TODO` 하위 항목을 처리한다.

## 실패·전체 완료

- 작업을 끝낼 수 없거나 Unreal MCP만으로 완료 기준을 검증할 수 없으면 해당 항목의 상태를 `FAILED`로 바꾸고 실패 사유를 남긴다. 이 변경을 커밋하고 `SendDiscordText.bat`으로 실패 알림을 보낸 뒤 다음 첫 `TODO` 하위 항목을 처리한다.
- 모든 하위 항목이 `DONE`, `FAILED`, `KEEP` 중 하나이면 세션을 종료한다.

## Discord 알림 형식

- 시작: `SendDiscordText.bat "[Unreal MCP Progress] 1-4 - <short current step>"`
- 검증: `SendDiscordText.bat "[Unreal MCP Progress] 1-4 - <result and next step>"`
- 완료: `SendDiscordText.bat "[Unreal MCP Complete] 0-1 - <short description> (commit: <hash>)"`
- 실패: `SendDiscordText.bat "[Unreal MCP Failed] 0-2 - <short failure reason> (commit: <hash>)"`

## 커밋 형식

`<type>(vfx): <작업 번호> <짧은 설명>`

예시: `feat(vfx): 1-3 add magnet scan pulse`
