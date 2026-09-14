# Unreal Engine 프로젝트의 VS Code 로컬 `.vscode` 설정 가이드

## 목적

새 Unreal Engine C++ 프로젝트를 다른 PC에서 열거나 새로 만들면 `.vscode` 폴더가 Git으로 공유되지 않아 VS Code가 Unreal 헤더, `*.generated.h`, 매크로를 제대로 해석하지 못할 수 있다.

이 문서는 각 작업환경에서 VS Code의 C++ 분석 환경을 다시 만들고, `#include` 오탐과 자동완성 문제를 해결하는 방법을 설명한다. 대상은 **Unreal Engine 5 + Windows + VS Code + Microsoft C/C++ 확장**이다.

> 중요한 구분: 이 가이드는 게임 코드나 Unreal 빌드를 수정하는 방법이 아니다. VS Code가 표시하는 빨간 밑줄, 자동완성, 정의로 이동을 정상화하는 로컬 개발환경 설정이다.

---

## 0. 사용하는 VS Code 확장

- **C/C++** (`ms-vscode.cpptools`): Unreal C++ 코드 분석과 자동완성
- **TriggerMate** (`TobiasWust.triggermate`): 새 `.cpp` 파일 생성 시 compile database 자동 갱신

---

## 1. Unreal에서 처음 한 번 빌드

1. Unreal Editor에서 프로젝트를 연다.
2. C++ 프로젝트가 처음이라면 Editor의 **Compile** 또는 Live Coding을 한 번 실행한다.
3. 빌드가 성공했는지 확인한다.

이 단계가 필요한 이유는 Unreal Header Tool이 `Intermediate/Build/.../UHT` 아래에 `MyClass.generated.h` 같은 생성 헤더를 만들기 때문이다. 이 파일이 없으면 VS Code는 `#include "MyClass.generated.h"` 오류를 표시할 수 있다.

---

## 2. Unreal로 VS Code 프로젝트 파일 생성

Unreal Editor에서 다음 중 현재 버전에 보이는 명령을 실행한다.

- **Tools → Generate Visual Studio Code Project**
- 또는 **Tools → Refresh Visual Studio Code Project**

그러면 `MyGame/.vscode`가 생성되며, 일반적으로 다음 파일/폴더가 포함된다.

```text
MyGame/.vscode/
├─ c_cpp_properties.json
├─ compileCommands_MyGame.json
├─ compileCommands_Default.json
├─ compileCommands_MyGame/
│  └─ MyGame.0.rsp
└─ compileCommands_Default/
   └─ MyGame.0.rsp
```

이 파일들은 직접 작성하는 것보다 Unreal이 생성하게 두는 것이 기본 원칙이다.

---

## 3. VS Code에서 작업공간 열기와 구성 선택

1. VS Code에서 `.uproject`가 있는 `MyGame` 폴더를 연다.
2. `Developer: Reload Window`를 실행한다.
3. `C/C++: Select a Configuration`을 실행한다.
4. 보통 `MyGameEditor Editor Win64 Development (MyGame)`처럼 표시되는 Editor/Win64/Development 구성을 선택한다.

자동완성이 전혀 안 보일 때는 편집기에서 `Ctrl+Space`를 눌러 확인한다.

- `Ctrl+Space`에서는 보이지만 타이핑 중에는 안 보임: VS Code의 자동 제안 설정 문제다.
- `Ctrl+Space`에서도 안 보임: C/C++ 확장, 선택한 구성, compile database를 점검한다.

---

## 4. 정상 여부 점검

`MyGame/.vscode/c_cpp_properties.json`을 열고 다음을 확인한다.

- `compilerPath`가 현재 PC에 실제로 존재하는 `cl.exe`를 가리킨다.
- `compileCommands`가 현재 프로젝트 경로의 JSON 파일을 가리킨다.

`compileCommands_MyGame.json`에는 프로젝트의 `.cpp` 파일이 들어 있어야 한다. 새로 만든 `Private/MyPawn.cpp`가 없다면 VS Code는 그 파일을 기본 설정으로만 분석해 오탐을 낼 수 있다.

각 `.cpp` 항목은 대략 다음 형태다.

```json
{
  "file": "C:\\...\\MyGame\\Source\\MyGame\\Private\\MyPawn.cpp",
  "arguments": [
    "C:\\...\\cl.exe",
    "@C:\\...\\.vscode\\compileCommands_MyGame\\MyGame.0.rsp"
  ]
}
```

---

## 5. 대표 오류: `#include "MyPawn.h"`를 찾지 못함

### 증상

`MyPawn.h`는 실제로 `Source/MyGame/Public/MyPawn.h`에 있는데 VS Code가 파일을 열 수 없다고 표시한다.

### 원인

compile database의 응답 파일(`*.rsp`)에 다음 모듈 경로가 빠진 경우다.

```text
Source/MyGame/Public
Source/MyGame/Private
```

루트 `Source`만 include 경로에 넣어도 하위 폴더를 자동으로 모두 탐색하지는 않는다.

### 즉시 해결

1. 먼저 **Tools → Refresh Visual Studio Code Project**를 실행한다.
2. 여전히 오류면 `compileCommands_*/*.rsp` 파일에 누락된 `/I ".../Public"`, `/I ".../Private"` 경로가 있는지 확인한다.
3. 경로를 추가한 뒤 VS Code에서 `Developer: Reload Window`를 실행한다.
4. 캐시가 남으면 `C/C++: Reset IntelliSense Database`를 실행한다.

> `*.rsp`는 Unreal이 재생성할 때 덮어써질 수 있다. 수동 수정은 임시 해결이며, 반복된다면 아래 자동화를 사용한다.

---

## 6. 새 C++ 파일을 만들 때의 기본 절차

새 C++ 클래스를 Unreal Editor에서 만들거나 `.cpp` 파일을 추가한 뒤 다음 중 하나를 수행한다.

### 수동 방식

1. Unreal Editor에서 **Refresh Visual Studio Code Project** 실행
2. VS Code에서 창 새로고침

### 자동 방식: TriggerMate

TriggerMate는 VS Code의 파일 생성 이벤트를 감시해서 Task를 실행하는 확장이다. 다음 조건으로만 사용한다.

- 감시 대상: `Source/**/*.cpp`
- 생성 이벤트: 실행
- 저장/수정 이벤트: 실행하지 않음
- 실행 Task: Unreal Build Tool로 VS Code 프로젝트 파일 재생성
- 디바운스: 약 3초

이렇게 하면 새 `.cpp` 생성 시에만 compile database가 자동 갱신된다. 저장할 때마다 Unreal Build Tool을 실행하지 않아 불필요한 부하를 막는다.

### TriggerMate 설정 예시

`<ProjectName>`은 현재 Unreal 프로젝트 이름으로 바꾼다. 아래 설정은 VS Code의 프로젝트 작업공간 `.vscode/settings.json`에 넣는다.

```json
{
    "triggerMate.fileTriggers": [
        {
            "file": "Source/**/*.cpp",
            "command": "<ProjectName>: Refresh VS Code Compile Database",
            "commandType": "task",
            "autoExecute": true,
            "ignoreCreateEvents": false,
            "ignoreChangeEvents": true,
            "ignoreDeleteEvents": true,
            "commandDebounce": 3000
        }
    ]
}
```

같은 작업공간의 `.vscode/tasks.json`에는 TriggerMate가 실행할 Task를 등록한다.

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "<ProjectName>: Refresh VS Code Compile Database",
            "type": "shell",
            "command": "powershell",
            "args": [
                "-NoProfile",
                "-ExecutionPolicy",
                "Bypass",
                "-File",
                "${workspaceFolder}/../Tools/Refresh-VsCodeCompileDatabase.ps1"
            ],
            "options": {
                "cwd": "${workspaceFolder}"
            },
            "problemMatcher": []
        }
    ]
}
```

### 갱신 스크립트가 해야 하는 일

위 Task가 호출하는 `Tools/Refresh-VsCodeCompileDatabase.ps1`은 프로젝트마다 다음 값을 맞춘다.

1. `${workspaceFolder}` 기준으로 `.uproject` 파일을 찾는다.
2. 해당 PC의 UnrealBuildTool 경로를 찾는다.
3. 다음 명령으로 VS Code 프로젝트 파일을 재생성한다.

```text
UnrealBuildTool.exe -ProjectFiles -Project="<ProjectFile>.uproject" -Game -Engine -VSCode
```

4. 생성된 `compileCommands_*.json`에 새 `.cpp`가 포함됐는지 확인한다.
5. 각 `compileCommands_*/*.rsp`에 현재 모듈의 `Source/<ModuleName>/Public`과 `Private` 경로가 없으면 `/I` 인수로 추가한다.

엔진 경로는 PC마다 다를 수 있으므로 특정 PC의 절대 경로를 그대로 복사하지 않는다. `EngineAssociation`, 환경 변수, 또는 사용자가 지정한 엔진 경로를 우선 사용한다.

### 설정 후 확인

1. VS Code에서 `Developer: Reload Window`를 실행한다.
2. `Ctrl+Shift+P → Tasks: Run Task`에서 `<ProjectName>: Refresh VS Code Compile Database`를 한 번 실행한다.
3. `Source` 아래에 테스트용 `.cpp` 파일을 새로 만들고 TriggerMate가 Task를 실행하는지 확인한다.
4. compile database에 새 파일이 등록됐는지 확인한다.

---

## 7. AI에게 설정을 맡길 때 쓸 요청문

Replace the bracketed placeholders with the current project values before sending this request to an AI.

```text
Set up the local VS Code IntelliSense environment for my Unreal Engine [version] C++ project.

- Unreal project file: [project path]/[game folder]/[game name].uproject
- VS Code workspace folder: [game folder]
- Current symptom: [for example, MyPawn.h cannot be found / code completion does not appear]
- Unreal Editor build status: [successful / not built yet]

Follow these rules:
1. First inspect `.vscode`, `compileCommands`, `*.rsp`, and `Source` read-only, then explain the likely cause.
2. Do not add generated files such as `Binaries`, `Intermediate`, `Saved`, or compile databases to Git.
3. Keep `c_cpp_properties.json` and compile databases with PC-specific absolute paths local.
4. If automatic refresh is needed when a new `.cpp` file is created, use TriggerMate workspace settings and a VS Code Task.
5. Do not run the task on every save; watch only for new `.cpp` file creation.
6. After regenerating files with Unreal Build Tool, check whether the module `Public` and `Private` include paths are missing and add them when necessary.
7. Before editing, summarize the planned changes. After editing, verify the JSON files, paths, and generated results.
```
