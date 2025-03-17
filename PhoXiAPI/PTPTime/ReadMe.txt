========================================================================
    CONSOLE APPLICATION : PTP Time Overview
========================================================================


This application demonstrates how to work with PTP-synchronized time.

Features:
- Retrieve acquisition time from received frames.
- Verify timestamp validity.
- Format and print timestamps.
- Calculate inter-device frame delays using std::chrono.

The program collects frames from multiple devices, prints their timestamps, and calculates frame delays in milliseconds.

Note: Devices must be synchronized to the same PTP master for accurate delay calculations.

Software & Hardware Requirements

1. Check for Hardware Timestamping Support
Run the following command, replacing <interface> with your actual network interface:

ethtool -T <interface>

If your adapter supports PTP, you should see:

Capabilities:
    hardware-transmit
    hardware-receive
    hardware-raw-clock

If these capabilities are missing, your adapter may not support PTP.

2. Ubuntu Setup (PTP Master Mode) - running manually

Install Linux PTP:

sudo apt update
sudo apt install linuxptp
ptp4l -v   # Verify installation

Configure PTP Master:

sudo cp /etc/linuxptp/ptp4l.conf /etc/linuxptp/ptp4l_custom.conf
sudo nano /etc/linuxptp/ptp4l_custom.conf

Modify the configuration:

[global]
twoStepFlag = 1
clientOnly = 0
priority1 = 5
priority2 = 128
domainNumber = 0

Run PTP Master:

sudo ptp4l -i <interface> -S -m -f /etc/linuxptp/ptp4l_custom.conf

Example:

sudo ptp4l -i enp6s0 -S -m -f /etc/linuxptp/ptp4l_custom.conf

Restart Photoneo Device:
Once the PTP master is running, the device should automatically detect and sync with it.

3. Ubuntu Setup (PTP Master Mode) - recommended service setup

To ensure PTP starts automatically, create a systemd service:

sudo nano /etc/systemd/system/ptp4l.service

Add the following content:

[Unit]
Description=PTP Time Synchronization Service
After=network.target

[Service]
ExecStart=/usr/sbin/ptp4l -i <interface> -S -m -f /etc/linuxptp/ptp4l_custom.conf
Restart=always

[Install]
WantedBy=multi-user.target

Save and exit, then enable and start the service:

sudo systemctl daemon-reload
sudo systemctl enable ptp4l
sudo systemctl start ptp4l

4. Windows Setup
Windows 11 does not support PTP master mode natively but can act as a PTP client. To enable master mode, use third-party software such as:
- Meinberg PTP
- ptp4l (via WSL2)
- Other time synchronization tools

Note: WSL2 can only run software-based PTP Grandmaster mode.

Useful Links
- https://static.ouster.dev/sensor-docs/image_route1/image_route2/appendix/ptp-quickstart.html
- https://networklessons.com/ip-services/introduction-to-precision-time-protocol-ptp
- https://github.com/microsoft/W32Time/tree/master/Precision%20Time%20Protocol

