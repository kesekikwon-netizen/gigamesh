# 제3자 라이브러리 라이선스 고지

이 저장소는 GPL-3.0-or-later([LICENSE](LICENSE))로 배포됩니다. 아래는 포함되거나 빌드 시 결합되는 제3자 구성요소입니다. 2026-10-17 소스 머리말을 직접 읽어 정리했습니다.

| 구성요소 | 위치 | 라이선스 | 상업 배포 | GPLv3 결합 | 비고 |
|---|---|---|---|---|---|
| GigaMesh Software Framework | `GigaMesh/` | GPL-3.0-or-later (`GigaMesh/LICENSE.txt`, `COPYING.txt`) | GPL 조건 하 가능 | — | Copyright 2020 Hubert Mara 외. 업스트림 gitlab.com/fcgl/GigaMesh (2024-02-21 23d8cf4) 대비 `core/` 5파일 +76/−2줄 변경 |
| **Triangle** (J. R. Shewchuk) | `GigaMesh/external/triangle/` | 자체 라이선스: 수정 배포 허용, **"상용 시스템의 일부로 배포하려면 저자와 직접 합의"** (`src/triangle.c:23-35`) | **합의 없이는 불가** | 비호환 (추가 제한) | `gigameshCore` 정적 라이브러리에 링크되어 GigaMesh 기반 실행파일 전부에 포함됨. **Studio/K-Arch는 사용하지 않음.** 상용 배포 전 교체(earcut.hpp ISC, CDT MPL-2.0 등) 또는 합의 필요 |
| ALGLIB 2.6 (AP library 1.3) | `GigaMesh/external/libAlg/` | GPL-2.0-or-later (`include/alg/ap.h:6-9`) | GPL 조건 하 가능 | 호환 | Copyright 2003-2009 Sergey Bochkanov |
| psalm | `GigaMesh/external/libpsalm/` | BSD-2-Clause (`GigaMesh/LICENSE.txt`) | 가능 | 호환 | Copyright 2010 Bastian Rieck |
| Catch v2.11.1 | `GigaMesh/tests/3rdParty/catch/` | Boost Software License 1.0 | 가능 | 호환 | 테스트 전용 |
| GLM 1.0.1 | CMake `FetchContent` (`GigaMesh/CMakeLists.txt`) | MIT / Happy Bunny | 가능 | 호환 | configure 시 GitHub에서 내려받으나 **실제 사용처 없음** → 제거 예정 |
| Qt 5.15 / Qt 6 | 시스템 설치(저장소 미포함) | LGPL-3.0 (오픈소스판) | 동적 링크·교체 가능 고지 조건 | 호환 | 배포 시 Qt 라이선스 고지와 동적 링크 유지 |
| Noto Sans KR | `GigaMesh/gui/resources/fonts/` | SIL OFL 1.1 | 가능 | 호환 | `NotoSansKR-Bold.ttf`가 Regular의 복사본(md5 동일) — 교체 예정. OFL 전문 동봉 필요 |
| GigaMesh 테스트 데이터 5종 | `GigaMesh/testdata/` | GigaMesh 저장소의 일부(GPLv3) | — | — | 업스트림 `testdata/`에서 가져옴 |

## 포함하지 않는 것 (의도적으로 제외)

- **Artec 3D Scanning SDK / Artec Studio**: 평가용 라이선스("sole property of Artec Group"), 재배포·역공학 금지. 헤더·DLL·번역 파일·분석 결과를 저장소에 넣지 않습니다. 파일 형식·단위 같은 공개 지식만 문서에 요약합니다.
- **Arch3D Liner 사용자 가이드 PDF**: 상용 제품 매뉴얼(저작권). 이전 이력에만 있고 새 이력에서는 제외했습니다.
- `.gitsecret/` 키링과 Google Analytics 키 파일: 업스트림 비밀 관리 잔재. 새 앱에서는 분석 전송 코드 자체를 제거할 예정입니다.
