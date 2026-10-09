# Web installer – adoption and hardware tests

CYD_Split-Flap-Clock adopted the shared [cyd-web-installer](https://github.com/anthonyjclarke/cyd-web-installer)
tooling for 1.0.0, following the rollout RUNBOOK. The pilot is
CYD_AnimatedPixelClock (`docs/WEB_INSTALLER_PLAN.md`).

---

## Build facts (1.0.0-dev, espressif32@6.12.0)

| Env   | Installer label            | `firmware.bin` | Slot use |
|:------|:---------------------------|:---------------|:---------|
| `cyd` | CYD 2.8″ – ESP32-2432S028R | 1,445,689 B    | 79 %     |

The slot is `app0` / `app1` of `partitions_custom.csv`, 0x1C0000 (1,835,008 B).
Manifests list four parts at `0x1000 / 0x8000 / 0xe000 / 0x10000`.

No filesystem image ships. Every split-flap tile is a PROGMEM array in
`src/splitflap_bitmaps.cpp`, generated from `splitflap_cyd_assets/`, and the
firmware never mounts LittleFS. The old `data/splitflap/` PNG copies were never
read and were removed. They wouldn't have fitted anyway: 152 files need about
162 LittleFS blocks, and the 384 KB data partition has 96. The only stored state
is WiFiManager's credentials in NVS, which an **Update** keeps.

The project has no OTA, so the `app1` case doesn't apply.

---

## Tests owed

Nothing owed for 1.0.0. Case 2 passed as the live Update after the release
(below). Re-run cases 1 and 2 when RUNBOOK 5b's triggers apply (platform,
partitions, WiFi/Improv or `loop()` timing, NVS keys, a new env, or first OTA).

- [x] Case 1 – fresh install, erased (10-10-2026, `B0:CB:D8:DA:AE:8C`)
- [x] Case 2 – Update on a provisioned board, settings kept (10-10-2026, same board)

Case 1 (fresh install) passed on the only board env. Case 3 (`app1`) doesn't
apply, because the project has no OTA. Case 4 (wrong board) doesn't apply,
because there is one env.

---

## Smoke test (RUNBOOK 5a) – 10-10-2026

The image was the CI `site-preview` from run 37974282182 (`dev` at `9ba5af3`,
with the Improv newline fix), served on `http://localhost:8000` and installed
from macOS Chrome.

| Case                           | Board / MAC                          | Result |
|:-------------------------------|:-------------------------------------|:-------|
| Fresh install, erased – `cyd`  | 2.8″ CH340 `B0:CB:D8:DA:AE:8C`       | Pass   |
| Connect on a provisioned board | same                                 | Pass   |

**Fresh install.** Erased with `pio run -t erase`, installed with erase, and
WiFi set through **Configure WiFi** (Improv). The boot log showed:

- `CYD_Split-Flap-Clock 1.0.0-dev` and Improv listening as `SplitFlap-CBB0`
- `Running from app0`
- the self-test completing, then WiFi joined from NVS
- NTP set (AEDT)
- 162 KB of free heap and no crash

**Connect.** With the board running the clock, Connect showed "Connected to
SplitFlap-CBB0 · CYD_Split-Flap-Clock 1.0.0-dev (ESP32)", not a bare
**Install**. Improv therefore answers in time and `PROJECT_NAME` matches the
manifest.

The Improv device suffix `CBB0` comes from the Espressif OUI, not the MAC
tail. This is a known cosmetic issue in the shared copy-in.

---

## Release v1.0.0 (10-10-2026)

Tag `v1.0.0` on `main` (`5e33629`). Release run 37975639246 built and
published, and `https://anthonyjclarke.github.io/CYD_Split-Flap-Clock/`
loads. `index.json` and `manifest-cyd.json` show 1.0.0, and the manifest lists
four parts. The release carries `CYD_Split-Flap-Clock-v1.0.0-cyd-firmware.bin`,
`…-cyd-merged.bin` and `SHA256SUMS.txt`.

**Live Update, 2.8″ `B0:CB:D8:DA:AE:8C` (case 2).** The board was provisioned
and running the CI `1.0.0-dev` image from the smoke test. From the live page,
Connect offered **Update CYD_Split-Flap-Clock** with no erase question. The
boot log afterwards showed:

- `CYD_Split-Flap-Clock 1.0.0` and `Running from app0`
- WiFi rejoined from the saved NVS credentials, with no portal
- NTP set
- no crash

Pass.
