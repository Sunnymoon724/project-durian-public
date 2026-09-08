# Project Durian 작업 지침

## 프로젝트 구조

- Unreal 프로젝트 루트는 `Durian/`입니다.
- Unreal 프로젝트 파일은 `Durian/Durian.uproject`입니다.
- C++ 게임 모듈은 `Durian/Source/` 아래에 있습니다.
- 문서와 자동화 도구는 각각 `Documents/`, `Tools/`에서 관리합니다.

## Unreal Engine 규칙

- `Durian/Config/`, `Durian/Content/`, `Durian/Source/`의 기존 구조를 유지합니다.
- `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, `.cache/`는 생성 파일이므로 커밋하지 않습니다.
- C++ 코드 변경 후 필요한 경우 Unreal Editor에서 프로젝트 파일과 VS Code 설정을 다시 생성합니다.
- 모듈 이름과 `*.Build.cs`, Target 파일의 이름을 임의로 변경하지 않습니다.

## 변경 및 검증

- 기존 사용자 변경 사항을 덮어쓰지 않습니다.
- 변경 범위를 작게 유지하고, 가능한 경우 관련 빌드 또는 컴파일 검사를 실행합니다.
- 생성 파일이나 IDE 개인 설정을 기능 코드와 함께 커밋하지 않습니다.
