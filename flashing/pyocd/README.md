# Flashing with pyocd

This is a different approach to flash the firmware on the b-parasites. It is completely optional.

## Basic instructions

(Based on the discussion in issue #67)

The instructions will probably differ slightly depending on your debugging probe and hardware setup.

The setup is fairly straightforward and documented in the `pyocd` [docs](https://pyocd.io/).

I used an inexpensive debugging probe from Aliexpress called `nanoDAP` from `Muse Labs` which is less than 10 € and readily available. The J-LINKs are hard to get and quite expensive. In my understanding basically all `CMSIS-DAP`-compatible probes which are recognized by `pyocd` should work.

After connecting the debugging probe via usb and connecting the b-parasite as described in the wiki, flashing was easy and fast.

For a complete flashing of a Zephyr firmware image, build the firmware first and then use the provided Python script. By default it expects the BLE sample output at:

```
../../code/nrf-connect/samples/ble/build_nrf52840_2.0.0/zephyr/zephyr.hex
```

Assuming that file exists, flashing boils down to:

```
python ./flash_firmware.py
```

For legacy firmware images that still need a SoftDevice, pass its hex file explicitly with `--softdevice`.

For help and documentation of the available command line flags use

```
python ./flash_firmware.py -h
```
