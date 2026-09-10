# Chronicle 위키 동기화

`sync-wiki.yml`은 Forge의 재사용 `refine-documents` workflow를 호출해 `Documents/`의 Markdown 변경을 `Chronicle/docs/`의 위키 페이지로 변환하고, 갱신 초안을 Pull Request로 만듭니다.

## 문서 매핑

| 원본 | Chronicle 대상 | 양식 |
| --- | --- | --- |
| `Documents/Design/` | `Chronicle/docs/design/` | `templates/design.md` |
| `Documents/Planning/` | `Chronicle/docs/overview/` | `templates/planning.md` |
| `Documents/Production/` | `Chronicle/docs/guides/` | `templates/guide.md` |
| `Documents/Operations/` | `Chronicle/docs/guides/` | `templates/guide.md` |

## GitHub 설정

기존 개발일지 자동화와 동일한 Repository Variables 및 Secret을 사용합니다.

- Variables: `AI_PROVIDER`, `AI_API_URL`, `AI_MODEL`
- Secret: `AI_API_KEY`

처음 전체 문서를 위키 초안으로 만들 때는 Actions의 **Sync Wiki**에서 `full_sync`를 선택해 수동 실행합니다. 이후에는 `Documents/**/*.md` 변경만 처리합니다.
