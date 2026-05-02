#!/usr/bin/env python3

import subprocess
import tempfile
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]

COMMON_CFLAGS = [
    "-std=c11",
    "-Wall",
    "-Wextra",
    "-Werror",
    "-Wno-unused-parameter",
]

INCLUDE_DIRS = [
    REPO_ROOT / "tests/host/stubs",
    REPO_ROOT / "code/nrf-connect/prstlib/include",
    REPO_ROOT / "code/nrf-connect/samples/ble/src",
    REPO_ROOT / "code/nrf-connect/samples/zigbee/src",
]


def compile_and_run(build_dir: Path, name: str, sources: list[Path], cflags: list[str] | None = None) -> None:
    binary = build_dir / name
    cmd = [
        "cc",
        *COMMON_CFLAGS,
        *(cflags or []),
        *(f"-I{include_dir}" for include_dir in INCLUDE_DIRS),
        *(str(source) for source in sources),
        "-lm",
        "-o",
        str(binary),
    ]
    subprocess.run(cmd, check=True, cwd=REPO_ROOT)
    subprocess.run([str(binary)], check=True, cwd=REPO_ROOT)
    print(f"{name}: ok")


def main() -> None:
    with tempfile.TemporaryDirectory(prefix="b-parasite-host-tests-") as tmp:
        build_dir = Path(tmp)
        compile_and_run(
            build_dir,
            "test_ble_bthome_v2",
            [
                REPO_ROOT / "tests/host/test_ble_bthome_v2.c",
                REPO_ROOT / "code/nrf-connect/samples/ble/src/encoding.c",
            ],
            [
                "-DCONFIG_PRST_BLE_ENCODING_SERVICE_DATA_LEN=19",
                "-DCONFIG_PRST_BLE_ENCODING_BTHOME_V2=1",
            ],
        )
        compile_and_run(
            build_dir,
            "test_sensor_math",
            [
                REPO_ROOT / "tests/host/test_sensor_math.c",
                REPO_ROOT / "code/nrf-connect/prstlib/src/sensor_math.c",
            ],
        )
        compile_and_run(
            build_dir,
            "test_zigbee_sensor_attrs",
            [
                REPO_ROOT / "tests/host/test_zigbee_sensor_attrs.c",
                REPO_ROOT / "code/nrf-connect/samples/zigbee/src/sensor_attrs.c",
            ],
        )
        compile_and_run(
            build_dir,
            "test_double_reset_flag",
            [
                REPO_ROOT / "tests/host/test_double_reset_flag.c",
                REPO_ROOT / "code/nrf-connect/samples/zigbee/src/double_reset_flag.c",
            ],
        )


if __name__ == "__main__":
    main()
