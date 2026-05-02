import argparse

from pathlib import Path

from pyocd.core.helpers import ConnectHelper
from pyocd.core.target import Target
from pyocd.flash.file_programmer import FileProgrammer

from mac_address_reader import MacAddressReader

SCRIPT_DIR = Path(__file__).resolve().parent
DEFAULT_FIRMWARE = (
    SCRIPT_DIR
    / "../../code/nrf-connect/samples/ble/build_nrf52840_2.0.0/zephyr/zephyr.hex"
).resolve()

parser = argparse.ArgumentParser()
parser.add_argument(
    "-s",
    "--softdevice",
    action="store",
    help="Optional legacy SoftDevice hex file.",
    default=None,
    type=Path,
)
parser.add_argument(
    "-f",
    "--firmware",
    action="store",
    help="Optional file path to firmware hex file.",
    default=DEFAULT_FIRMWARE,
    type=Path,
)
parser.add_argument(
    "--skip-softdevice",
    action="store_true",
    help="Skip flashing of the legacy SoftDevice.",
)
parser.add_argument(
    "--skip-firmware",
    action="store_true",
    help="Skip flashing of firmware.",
)

args = parser.parse_args()

if args.softdevice is not None:
    args.softdevice = args.softdevice.expanduser().resolve()
if args.firmware is not None:
    args.firmware = args.firmware.expanduser().resolve()

if args.softdevice is not None and not args.skip_softdevice and not args.softdevice.exists():
    parser.error(f"SoftDevice hex file not found: {args.softdevice}")
if not args.skip_firmware and not args.firmware.exists():
    parser.error(f"Firmware hex file not found: {args.firmware}")

with ConnectHelper.session_with_chosen_probe() as session:

    board = session.board
    target: Target = board.target

    if args.softdevice is not None and not args.skip_softdevice:
        softdevice_path: Path = args.softdevice
        print(f"Flashing softdevice {softdevice_path.name}...")
        FileProgrammer(session).program(str(softdevice_path))

        target.reset_and_halt()
        target.resume()

    if not args.skip_firmware:
        firmware_path: Path = args.firmware
        print(f"Flashing firmware {firmware_path.name}...")
        FileProgrammer(session).program(str(firmware_path))

        target.reset_and_halt()
        target.resume()
    
    mac_address_reader = MacAddressReader(target)
    mac_address = mac_address_reader.get_static_mac_address()
    print(f'Static manufacturer MAC address (DEVICEADDR): {mac_address}')
