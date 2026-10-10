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

Nothing owed for 1.1.0. It changed WiFi and Improv timing (a RUNBOOK 5b
trigger), so cases 1 and 2 were re-run and passed. Re-run cases 1 and 2 when the 5b triggers apply again (platform,
partitions, WiFi/Improv or `loop()` timing, NVS keys, a new env, or first OTA).

- [x] Case 1 – fresh install, erased (1.1.0-dev, 10-10-2026, `B0:CB:D8:DA:AE:8C`)
- [x] Case 2 – Update on a provisioned board, settings kept (1.1.0 live page, 10-10-2026, same board)

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

---

## 1.1.0 – smoke test (10-10-2026)

Changes: a 15 s saved-network connect timeout (was ~60 s with Improv
unserviced), the Improv name from the MAC tail, and a local POSIX timezone rule.

**Why.** After the 1.0.0 release, the bench board failed to join the saved
WiFi twice (`AutoConnect: FAILED for 60611 ms`). During that minute Improv was
silent. The next boot also showed NTP in **UTC**: ezTime's `setLocation()`
lookup had timed out, so the clock would have shown the wrong time.

**Fresh install, 2.8″ `B0:CB:D8:DA:AE:8C`.** This used the CI preview from run
38006818104 (`ed62603`, before the timezone fix). The board was erased,
installed with erase, and WiFi set through **Configure WiFi**. Boot log:

- `1.1.0-dev`, Improv listening as `SplitFlap-AE8C`
- `connect timeout 15s`, `Running from app0`
- WiFi joined
- NTP set, but in UTC – fixed next in `34b11ed`
- no crash

Connect then showed "Connected to SplitFlap-AE8C · CYD_Split-Flap-Clock
1.1.0-dev (ESP32)". Pass.

---

## Release v1.1.0 (10-10-2026)

Tag `v1.1.0` on `main` (`9998763`). Release run 38008277192 built and
published. The live page, `index.json` and the manifest show 1.1.0, with four
parts. The release carries `CYD_Split-Flap-Clock-v1.1.0-cyd-firmware.bin`,
`…-cyd-merged.bin` and `SHA256SUMS.txt`.

**Live Update, 2.8″ `B0:CB:D8:DA:AE:8C` (case 2).** The board was running the
CI `1.1.0-dev` image. From the live page, Connect offered **Update** with no
erase question, and the clock came back showing the right local time. Boot
log:

- `1.1.0`, `SplitFlap-AE8C`, `Running from app0`
- `connect timeout 15s`
- WiFi rejoined from NVS
- timezone `AEST-10AEDT,M10.1.0,M4.1.0/3`

On the first boot, NTP timed out after 30 s and retried in the background. Two
further boots synced in AEDT within seconds, so that was the network, not the
firmware. Pass.
